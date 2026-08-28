/*!
\file davis.cpp
\brief Implementation of davis.
\author Raul Tapia
*/
#include "openev/devices/davis.hpp"
#include "openev/devices/abstract-camera.hpp"
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <libcaer/devices/davis.h>
#include <libcaer/devices/device.h>
#include <libcaer/devices/usb.h>
#include <libcaer/events/common.h>
#include <libcaer/events/frame.h>
#include <libcaer/events/imu6.h>
#include <libcaer/events/packetContainer.h>
#include <libcaer/events/polarity.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/saturate.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <string>

ev::Davis::Davis() {
  deviceHandler_ = caerDeviceOpen(0, CAER_DEVICE_DAVIS, 0, 0, "");
  if(deviceHandler_ == nullptr) {
    CV_LOG_ERROR(nullptr, "ev::Davis: Could not find camera.");
  } else {
    caerDeviceSendDefaultConfig(deviceHandler_);

    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, 1U);

    AbstractCamera::setContainerInterval(ev::Davis::DEFAULT_INTERVAL);                                                   // 50Hz == 20000us
    AbstractCamera::setContainerSize(0U);                                                                                // Set to zero to disable
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_MODE, 0U);                              // Default frame mode
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_INTERVAL, ev::Davis::DEFAULT_INTERVAL); // 50Hz == 20000us
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, ev::Davis::DEFAULT_EXPOSURE);       // 6500 us

    const cv::Size size = getSensorSize();
    CV_LOG_DEBUG(nullptr, "ev::Davis: opened " << getSerialNumber() << ", " << size.width << "x" << size.height << " sensor.");
  }
}

void ev::Davis::start() {
  if(running_) {
    return;
  }

  std::array<uint32_t, 5> enable{};
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, enable.data());
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, &enable[1]);
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, &enable[2]);
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, &enable[3]);
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, &enable[4]);

  running_ = caerDeviceDataStart(deviceHandler_, nullptr, nullptr, nullptr, nullptr, nullptr);
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_DATAEXCHANGE, CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, 1U);

  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, enable[0]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, enable[1]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, enable[2]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, enable[3]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, enable[4]);

  AbstractCamera::flush(1000);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_MUX, DAVIS_CONFIG_MUX_TIMESTAMP_RESET, 1);
  resetTime_ = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
}

cv::Size ev::Davis::getSensorSize() const {
  struct caer_davis_info const info = caerDavisInfoGet(deviceHandler_);
  return {info.dvsSizeX, info.dvsSizeY};
}

namespace {
const char *chipName(const int16_t id) {
  switch(id) {
  case DAVIS_CHIP_DAVIS240A:
    return "DAVIS240A";
  case DAVIS_CHIP_DAVIS240B:
    return "DAVIS240B";
  case DAVIS_CHIP_DAVIS240C:
    return "DAVIS240C";
  case DAVIS_CHIP_DAVIS128:
    return "DAVIS128";
  case DAVIS_CHIP_DAVIS346A:
    return "DAVIS346A";
  case DAVIS_CHIP_DAVIS346B:
    return "DAVIS346B";
  case DAVIS_CHIP_DAVIS346C:
    return "DAVIS346C";
  case DAVIS_CHIP_DAVIS640:
    return "DAVIS640";
  case DAVIS_CHIP_DAVIS640H:
    return "DAVIS640H";
  case DAVIS_CHIP_DAVIS208:
    return "DAVIS208";
  default:
    return "DAVIS (Unknown)";
  }
}
} // namespace

std::string ev::Davis::getSerialNumber() const {
  struct caer_davis_info const info = caerDavisInfoGet(deviceHandler_);
  const std::string serial(static_cast<const char *>(info.deviceSerialNumber));
  if(serial.empty()) {
    return {};
  }
  return std::string(chipName(info.chipID)) + "-" + serial;
}

bool ev::Davis::setRoi(const cv::Rect_<uint16_t> &roi) {
  struct caer_davis_info const info = caerDavisInfoGet(deviceHandler_);
  const cv::Rect_<uint16_t> full(0, 0, static_cast<uint16_t>(info.dvsSizeX), static_cast<uint16_t>(info.dvsSizeY));

  if(roi.width > 0 && roi.height > 0 && full.contains(roi.tl()) && roi.br().x <= full.width && roi.br().y <= full.height) {
    const auto end_x = static_cast<uint16_t>(roi.br().x - 1);
    const auto end_y = static_cast<uint16_t>(roi.br().y - 1);

    // NOTE (libcaer): this configures the APS ROI
    if(caerDavisROIConfigure(deviceHandler_, roi.tl().x, roi.tl().y, end_x, end_y)) {
      // NOTE (libcaer): this manages the DVS ROI
      filterRoiInSoftware_ = !info.dvsHasROIFilter;
      if(!filterRoiInSoftware_) {
        filterRoiInSoftware_ = !(caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_FILTER_ROI_START_COLUMN, roi.tl().x) && caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_FILTER_ROI_START_ROW, roi.tl().y) && caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_FILTER_ROI_END_COLUMN, end_x) && caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_FILTER_ROI_END_ROW, end_y));
        if(filterRoiInSoftware_) {
          CV_LOG_WARNING(nullptr, "ev::Davis: Could not configure the DVS ROI filter, falling back to software filtering.");
        }
      }
      roi_ = roi;
      CV_LOG_DEBUG(nullptr, "ev::Davis::setRoi: " << roi.width << "x" << roi.height << " at (" << roi.x << ", " << roi.y << "), dvs filtered by " << (filterRoiInSoftware_ ? "software" : "hardware") << ".");
      return true;
    }
  }
  return false;
}

ev::BiasValue ev::Davis::getBias(const uint8_t name) const {
  uint32_t param{0};
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_BIAS, name, &param);
  const struct caer_bias_coarsefine cf = caerBiasCoarseFineParse(static_cast<uint16_t>(param));
  return {cf.coarseValue, cf.fineValue};
}

bool ev::Davis::setBias(const uint8_t name, const ev::BiasValue &value) {
  uint32_t param{0};
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_BIAS, name, &param);
  struct caer_bias_coarsefine cf = caerBiasCoarseFineParse(static_cast<uint16_t>(param));
  cf.coarseValue = value.coarse;
  cf.fineValue = value.fine;
  return caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_BIAS, name, caerBiasCoarseFineGenerate(cf));
}

uint32_t ev::Davis::biasToCurrent(const ev::BiasValue &value) {
  struct caer_bias_coarsefine cf{};
  cf.coarseValue = value.coarse;
  cf.fineValue = value.fine;
  return caerBiasCoarseFineToCurrent(cf);
}

void ev::Davis::enableDvs(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, static_cast<uint32_t>(state));
}

void ev::Davis::enableAps(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, static_cast<uint32_t>(state));
}

void ev::Davis::setApsTimeInterval(const uint32_t usec) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_INTERVAL, usec);
}

uint32_t ev::Davis::getApsExposure() const {
  uint32_t usec{0};
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, &usec);
  return usec;
}

void ev::Davis::setApsExposure(const uint32_t usec) {
  uint32_t automatic{0};
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_AUTOEXPOSURE, &automatic);
  if(automatic != 0) {
    CV_Error(cv::Error::StsError, "ev::Davis::setApsExposure: the automatic exposure is enabled and overwrites the exposure on every frame.");
  }
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, usec);
}

void ev::Davis::enableApsAutoExposure(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_AUTOEXPOSURE, static_cast<uint32_t>(state));
}

void ev::Davis::enableImu(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, static_cast<uint32_t>(state));
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, static_cast<uint32_t>(state));
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, static_cast<uint32_t>(state));
}
