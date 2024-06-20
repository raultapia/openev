/*!
\file davis.cpp
\brief Implementation of davis.
\author Raul Tapia
*/
#include "openev/devices/davis.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/devices/abstract-camera.hpp"
#include "openev/utils/logger.hpp"
#include <array>
#include <cstring>
#include <iosfwd>
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
#include <opencv2/core/types.hpp>
#include <stdint.h>

ev::Davis::Davis() {
  deviceHandler_ = caerDeviceOpen(0, CAER_DEVICE_DAVIS, 0, 0, "");
  if(deviceHandler_ == nullptr) {
    ev::logger::error("ev::Davis: Could not find camera.");
  } else {
    caerDeviceSendDefaultConfig(deviceHandler_);

    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, 0U);

    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, ev::Davis::DEFAULT_INTERVAL); // 50Hz == 20000us
    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_PACKET_SIZE, 0U);                       // Set to zero to disable
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_INTERVAL, ev::Davis::DEFAULT_INTERVAL);                         // 50Hz == 20000us
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, ev::Davis::DEFAULT_EXPOSURE);                               // 6500 us
  }
}

ev::Davis::~Davis() {
  caerDeviceDataStop(deviceHandler_);
  caerDeviceClose(&deviceHandler_);
}

void ev::Davis::start() {
  std::array<uint32_t, 5> enable;
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, &enable[0]);
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, &enable[1]);
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, &enable[2]);
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, &enable[3]);
  caerDeviceConfigGet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, &enable[4]);

  caerDeviceDataStart(deviceHandler_, nullptr, nullptr, nullptr, nullptr, nullptr);
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_DATAEXCHANGE, CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, 1U);

  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, enable[0]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, enable[1]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, enable[2]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, enable[3]);
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, enable[4]);

  flush(1);
}

ev::BiasValue ev::Davis::getBias(const uint8_t name) const {
  return AbstractCamera_::getBias(DAVIS_CONFIG_BIAS, name);
}

bool ev::Davis::setBias(const uint8_t name, const ev::BiasValue &value) {
  return AbstractCamera_::setBias(DAVIS_CONFIG_BIAS, name, value);
}

void ev::Davis::enableDvs(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, static_cast<uint32_t>(state));
}

void ev::Davis::setDvsTimeInterval(const uint32_t usec) {
  // NOTE (libcaer): Must be at least 1 microsecond
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, (usec < 1 || usec > 600000000) ? 600000000 : usec);
}

void ev::Davis::setDvsEventsPerPacket(const uint32_t n) {
  // NOTE (libcaer): Set to zero to disable
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_PACKET_SIZE, n);
}

void ev::Davis::enableAps(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, static_cast<uint32_t>(state));
}

void ev::Davis::setApsTimeInterval(const uint32_t usec) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_INTERVAL, usec);
}

void ev::Davis::setExposure(const uint32_t exposure) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, exposure);
}

void ev::Davis::enableImu(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, static_cast<uint32_t>(state));
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, static_cast<uint32_t>(state));
}

bool ev::Davis::getData(ev::Vector &events) {
  getData_<ev::Vector, std::nullptr_t, std::nullptr_t>(&events, nullptr, nullptr);
  return !events.empty();
}

bool ev::Davis::getData(ev::Queue &events) {
  getData_<ev::Queue, std::nullptr_t, std::nullptr_t>(&events, nullptr, nullptr);
  return !events.empty();
}

bool ev::Davis::getData(StampedMat &frame) {
  frame.release();
  getData_<std::nullptr_t, ev::StampedMat, std::nullptr_t>(nullptr, &frame, nullptr);
  return !frame.empty();
}

bool ev::Davis::getData(StampedMatVector &frames) {
  getData_<std::nullptr_t, ev::StampedMatVector, std::nullptr_t>(nullptr, &frames, nullptr);
  return !frames.empty();
}

bool ev::Davis::getData(StampedMatQueue &frames) {
  getData_<std::nullptr_t, ev::StampedMatQueue, std::nullptr_t>(nullptr, &frames, nullptr);
  return !frames.empty();
}

bool ev::Davis::getData(Imu &imu) {
  imu.release();
  getData_<std::nullptr_t, ev::Imu, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis::getData(ImuVector &imu) {
  getData_<std::nullptr_t, ev::ImuVector, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis::getData(ImuQueue &imu) {
  getData_<std::nullptr_t, ev::ImuQueue, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis::getData(ev::Vector &events, ev::StampedMat &frame) {
  frame.release();
  getData_<ev::Vector, ev::StampedMat, std::nullptr_t>(&events, &frame, nullptr);
  return (!events.empty() || !frame.empty());
}

bool ev::Davis::getData(ev::Vector &events, ev::StampedMatVector &frames) {
  getData_<ev::Vector, ev::StampedMatVector, std::nullptr_t>(&events, &frames, nullptr);
  return (!events.empty() || !frames.empty());
}

bool ev::Davis::getData(ev::Queue &events, ev::StampedMatQueue &frames) {
  getData_<ev::Queue, ev::StampedMatQueue, std::nullptr_t>(&events, &frames, nullptr);
  return (!events.empty() || !frames.empty());
}

bool ev::Davis::getData(ev::Vector &events, ev::Imu &imu) {
  imu.release();
  getData_<ev::Vector, std::nullptr_t, ev::Imu>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::Vector &events, ev::ImuVector &imu) {
  getData_<ev::Vector, std::nullptr_t, ev::ImuVector>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::Queue &events, ev::ImuQueue &imu) {
  getData_<ev::Queue, std::nullptr_t, ev::ImuQueue>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::Vector &events, ev::StampedMat &frame, ev::Imu &imu) {
  frame.release();
  imu.release();
  getData_<ev::Vector, ev::StampedMat, ev::Imu>(&events, &frame, &imu);
  return (!events.empty() || !frame.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::Vector &events, ev::StampedMatVector &frames, ev::ImuVector &imu) {
  getData_<ev::Vector, ev::StampedMatVector, ev::ImuVector>(&events, &frames, &imu);
  return (!events.empty() || !frames.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::Queue &events, ev::StampedMatQueue &frames, ev::ImuQueue &imu) {
  getData_<ev::Queue, ev::StampedMatQueue, ev::ImuQueue>(&events, &frames, &imu);
  return (!events.empty() || !frames.empty() || !imu.empty());
}

template <typename T1, typename T2, typename T3>
void ev::Davis::getData_(T1 *dvs, T2 *aps, T3 *imu) {
  caerEventPacketContainerConst container = caerDeviceDataGet(deviceHandler_);
  while(container == nullptr) {
    ev::logger::warning("Connection with camera lost, retrying.");
    caerDeviceDataStop(deviceHandler_);
    caerDeviceDataStart(deviceHandler_, nullptr, nullptr, nullptr, nullptr, nullptr);
    container = caerDeviceDataGet(deviceHandler_);
  }

  const int32_t containter_size = caerEventPacketContainerGetEventPacketsNumber(container);
  for(int32_t i = 0; i < containter_size; i++) {
    const caerEventPacketHeaderConst packet = caerEventPacketContainerGetEventPacketConst(container, i);
    if(packet == nullptr) {
      continue;
    }

    const int32_t packet_size = caerEventPacketHeaderGetEventNumber(packet);
    switch(caerEventPacketHeaderGetEventType(packet)) {
    case POLARITY_EVENT:
      if constexpr(std::is_same_v<T1, std::nullptr_t>) {
        break;
      } else {
        if constexpr(std::is_same_v<T1, ev::Vector>) {
          dvs->reserve(dvs->size() + packet_size);
        }
        for(int32_t k = 0; k < packet_size; k++) {
          const caerPolarityEventConst p = caerPolarityEventPacketGetEventConst(reinterpret_cast<caerPolarityEventPacketConst>(packet), k);
          const uint16_t x = caerPolarityEventGetX(p);
          const uint16_t y = caerPolarityEventGetY(p);
          if(roi_.empty() || roi_.contains(cv::Point(x, y))) {
            if constexpr(std::is_same_v<T1, ev::Vector>) {
              dvs->emplace_back(x, y, caerPolarityEventGetTimestamp(p) + timeOffset_, caerPolarityEventGetPolarity(p));
            } else if constexpr(std::is_same_v<T1, ev::Queue>) {
              dvs->emplace(x, y, caerPolarityEventGetTimestamp(p) + timeOffset_, caerPolarityEventGetPolarity(p));
            }
          }
        }
        break;
      }

    case FRAME_EVENT:
      if constexpr(std::is_same_v<T2, std::nullptr_t>) {
        break;
      } else {
        for(int32_t k = 0; k < packet_size; k++) {
          const caerFrameEventConst p = caerFrameEventPacketGetEventConst(reinterpret_cast<caerFrameEventPacketConst>(packet), k);
          ev::StampedMat mat;
          mat.t = caerFrameEventGetTimestamp(p) + timeOffset_;

          const int32_t x = caerFrameEventGetLengthX(p);
          const int32_t y = caerFrameEventGetLengthY(p);
          cv::Mat m16(y, x, CV_16UC1);
          std::memcpy(m16.data, p->pixels, sizeof(uint16_t) * y * x);
          m16.convertTo(mat, CV_8UC1, ev::SCALE_16B_8B);

          if constexpr(std::is_same_v<T2, ev::StampedMat>) {
            mat.copyTo(*aps);
          }
          if constexpr(std::is_same_v<T2, ev::StampedMatVector>) {
            aps->push_back(mat);
          }
          if constexpr(std::is_same_v<T2, ev::StampedMatQueue>) {
            aps->push(mat);
          }
        }
        break;
      }

    case IMU6_EVENT:
      if constexpr(std::is_same_v<T3, std::nullptr_t>) {
        break;
      } else {
        for(int32_t k = 0; k < packet_size; k++) {
          const caerIMU6EventConst p = caerIMU6EventPacketGetEventConst(reinterpret_cast<caerIMU6EventPacketConst>(packet), k);
          ev::Imu data;
          data.t = caerIMU6EventGetTimestamp(p) + timeOffset_;
          data.linear_acceleration.x = -caerIMU6EventGetAccelX(p) * ev::EARTH_GRAVITY;
          data.linear_acceleration.y = caerIMU6EventGetAccelY(p) * ev::EARTH_GRAVITY;
          data.linear_acceleration.z = -caerIMU6EventGetAccelZ(p) * ev::EARTH_GRAVITY;
          data.angular_velocity.x = -caerIMU6EventGetGyroX(p) * ev::DEG2RAD;
          data.angular_velocity.y = caerIMU6EventGetGyroY(p) * ev::DEG2RAD;
          data.angular_velocity.z = -caerIMU6EventGetGyroZ(p) * ev::DEG2RAD;

          if constexpr(std::is_same_v<T3, ev::Imu>) {
            *imu = data;
          }
          if constexpr(std::is_same_v<T3, ev::ImuVector>) {
            imu->push_back(data);
          }
          if constexpr(std::is_same_v<T3, ev::ImuQueue>) {
            imu->push(data);
          }
        }
        break;
      }
    }
  }
}

void ev::Davis::getEventRaw(std::vector<uint64_t> &data) {
  caerEventPacketContainerConst container = caerDeviceDataGet(deviceHandler_);
  while(container == nullptr) {
    ev::logger::warning("Connection with camera lost, retrying.");
    caerDeviceDataStop(deviceHandler_);
    caerDeviceDataStart(deviceHandler_, nullptr, nullptr, nullptr, nullptr, nullptr);
    container = caerDeviceDataGet(deviceHandler_);
  }

  const int32_t containter_size = caerEventPacketContainerGetEventPacketsNumber(container);
  for(int32_t i = 0; i < containter_size; i++) {
    const caerEventPacketHeaderConst packet = caerEventPacketContainerGetEventPacketConst(container, i);
    if(packet == nullptr || caerEventPacketHeaderGetEventType(packet) != POLARITY_EVENT) {
      continue;
    }
    const int32_t packet_size = caerEventPacketHeaderGetEventNumber(packet);
    data.reserve(data.size() + packet_size);
    for(int32_t k = 0; k < packet_size; k++) {
      const caerPolarityEventConst p = caerPolarityEventPacketGetEventConst(reinterpret_cast<caerPolarityEventPacketConst>(packet), k);
      data.emplace_back((static_cast<uint64_t>(p->data) << 32) | static_cast<uint64_t>(p->timestamp));
    }
  }
}
