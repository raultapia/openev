/*!
\file davis.cpp
\brief Implementation of davis.
\author Raul Tapia
*/
#include "libcaer/devices/davis.h"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/devices/abstract-camera.hpp"
#include "openev/devices/davis.hpp"
#include "openev/utils/logger.hpp"
#include <cstring>
#include <iosfwd>
#include <libcaer/devices/device.h>
#include <libcaer/devices/usb.h>
#include <libcaer/events/common.h>
#include <libcaer/events/packetContainer.h>
#include <libcaercpp/events/common.hpp>
#include <libcaercpp/events/frame.hpp>
#include <libcaercpp/events/imu6.hpp>
#include <libcaercpp/events/packetContainer.hpp>
#include <libcaercpp/events/polarity.hpp>
#include <memory>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/types.hpp>
#include <stdint.h>
#include <stdlib.h>

ev::Davis_::Davis_() {
  deviceHandler_ = caerDeviceOpen(0, CAER_DEVICE_DAVIS, 0, 0, "");
  if(deviceHandler_ == nullptr) {
    ev::logger::error("ev::Davis: Could not find camera.");
  } else {
    caerDeviceSendDefaultConfig(deviceHandler_);

    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, 0U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, 0U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, 0U);

    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_DATAEXCHANGE, CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, 1U);                               // Blocking mode
    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, ev::Davis_::DEFAULT_INTERVAL); // 50Hz == 20000us
    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_PACKET_SIZE, 0U);                        // Set to zero to disable
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_INTERVAL, ev::Davis_::DEFAULT_INTERVAL);                         // 50Hz == 20000us
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, ev::Davis_::DEFAULT_EXPOSURE);                               // 6500 us
  }
}

ev::Davis_::~Davis_() {
  caerDeviceDataStop(deviceHandler_);
  caerDeviceClose(&deviceHandler_);
}

ev::BiasValue ev::Davis_::getBias(const uint8_t name) const {
  return AbstractCamera_::getBias(DAVIS_CONFIG_BIAS, name);
}

bool ev::Davis_::setBias(const uint8_t name, const ev::BiasValue &value) {
  return AbstractCamera_::setBias(DAVIS_CONFIG_BIAS, name, value);
}

void ev::Davis_::enableDvs(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, static_cast<uint32_t>(state));
}

void ev::Davis_::setDvsTimeInterval(const uint32_t usec) {
  // NOTE (libcaer): Must be at least 1 microsecond
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, (usec < 1 || usec > 600000000) ? 600000000 : usec);
}

void ev::Davis_::setDvsEventsPerPacket(const uint32_t n) {
  // NOTE (libcaer): Set to zero to disable
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_PACKET_SIZE, n);
}

void ev::Davis_::enableAps(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, static_cast<uint32_t>(state));
}

void ev::Davis_::setApsTimeInterval(const uint32_t usec) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_INTERVAL, usec);
}

void ev::Davis_::setExposure(const uint32_t exposure) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, exposure);
}

void ev::Davis_::enableImu(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, static_cast<uint32_t>(state));
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, static_cast<uint32_t>(state));
}

bool ev::Davis_::getData(ev::Vector &events) {
  getData_<ev::Vector, std::nullptr_t, std::nullptr_t>(&events, nullptr, nullptr);
  return !events.empty();
}

bool ev::Davis_::getData(ev::Queue &events) {
  getData_<ev::Queue, std::nullptr_t, std::nullptr_t>(&events, nullptr, nullptr);
  return !events.empty();
}

bool ev::Davis_::getData(StampedMat &frame) {
  frame.release();
  getData_<std::nullptr_t, ev::StampedMat, std::nullptr_t>(nullptr, &frame, nullptr);
  return !frame.empty();
}

bool ev::Davis_::getData(StampedMatVector &frames) {
  getData_<std::nullptr_t, ev::StampedMatVector, std::nullptr_t>(nullptr, &frames, nullptr);
  return !frames.empty();
}

bool ev::Davis_::getData(StampedMatQueue &frames) {
  getData_<std::nullptr_t, ev::StampedMatQueue, std::nullptr_t>(nullptr, &frames, nullptr);
  return !frames.empty();
}

bool ev::Davis_::getData(Imu &imu) {
  imu.release();
  getData_<std::nullptr_t, ev::Imu, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis_::getData(ImuVector &imu) {
  getData_<std::nullptr_t, ev::ImuVector, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis_::getData(ImuQueue &imu) {
  getData_<std::nullptr_t, ev::ImuQueue, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis_::getData(ev::Vector &events, ev::StampedMat &frame) {
  frame.release();
  getData_<ev::Vector, ev::StampedMat, std::nullptr_t>(&events, &frame, nullptr);
  return (!events.empty() || !frame.empty());
}

bool ev::Davis_::getData(ev::Vector &events, ev::StampedMatVector &frames) {
  getData_<ev::Vector, ev::StampedMatVector, std::nullptr_t>(&events, &frames, nullptr);
  return (!events.empty() || !frames.empty());
}

bool ev::Davis_::getData(ev::Queue &events, ev::StampedMatQueue &frames) {
  getData_<ev::Queue, ev::StampedMatQueue, std::nullptr_t>(&events, &frames, nullptr);
  return (!events.empty() || !frames.empty());
}

bool ev::Davis_::getData(ev::Vector &events, ev::Imu &imu) {
  imu.release();
  getData_<ev::Vector, std::nullptr_t, ev::Imu>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis_::getData(ev::Vector &events, ev::ImuVector &imu) {
  getData_<ev::Vector, std::nullptr_t, ev::ImuVector>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis_::getData(ev::Queue &events, ev::ImuQueue &imu) {
  getData_<ev::Queue, std::nullptr_t, ev::ImuQueue>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis_::getData(ev::Vector &events, ev::StampedMat &frame, ev::Imu &imu) {
  frame.release();
  imu.release();
  getData_<ev::Vector, ev::StampedMat, ev::Imu>(&events, &frame, &imu);
  return (!events.empty() || !frame.empty() || !imu.empty());
}

bool ev::Davis_::getData(ev::Vector &events, ev::StampedMatVector &frames, ev::ImuVector &imu) {
  getData_<ev::Vector, ev::StampedMatVector, ev::ImuVector>(&events, &frames, &imu);
  return (!events.empty() || !frames.empty() || !imu.empty());
}

bool ev::Davis_::getData(ev::Queue &events, ev::StampedMatQueue &frames, ev::ImuQueue &imu) {
  getData_<ev::Queue, ev::StampedMatQueue, ev::ImuQueue>(&events, &frames, &imu);
  return (!events.empty() || !frames.empty() || !imu.empty());
}

inline std::unique_ptr<libcaer::events::EventPacketContainer> get_data_from_handler(caerDeviceHandle &h) {
  caerEventPacketContainer cContainer = caerDeviceDataGet(h);
  while(true) {
    if(cContainer == nullptr) {
      ev::logger::warning("Connection with camera lost, retrying.");
      caerDeviceDataStop(h);
      caerDeviceDataStart(h, nullptr, nullptr, nullptr, nullptr, nullptr);
      cContainer = caerDeviceDataGet(h);
    } else {
      std::unique_ptr<libcaer::events::EventPacketContainer> cppContainer = std::make_unique<libcaer::events::EventPacketContainer>(cContainer);
      free(cContainer);
      return cppContainer;
    }
  }
}

template <typename T1, typename T2, typename T3>
void ev::Davis_::getData_(T1 *dvs, T2 *aps, T3 *imu) {
  std::unique_ptr<libcaer::events::EventPacketContainer> container = get_data_from_handler(deviceHandler_);

  for(const auto &packet : *container) {
    if(packet == nullptr) {
      continue;
    }
    switch(packet->getEventType()) {
    case POLARITY_EVENT:
      if constexpr(std::is_same_v<T1, std::nullptr_t>) {
        break;
      } else {
        std::shared_ptr<const libcaer::events::PolarityEventPacket> ptr = std::static_pointer_cast<libcaer::events::PolarityEventPacket>(packet);
        if constexpr(std::is_same_v<T1, ev::Vector>) {
          dvs->reserve(dvs->size() + ptr->size());
          for(const auto &p : *ptr) {
            if(roi_.empty() || roi_.contains(cv::Point(p.getX(), p.getY()))) {
              dvs->emplace_back(p.getX(), p.getY(), p.getTimestamp() + timeOffset_, p.getPolarity());
            }
          }
        }
        if constexpr(std::is_same_v<T1, ev::Queue>) {
          for(const auto &p : *ptr) {
            if(roi_.empty() || roi_.contains(cv::Point(p.getX(), p.getY()))) {
              dvs->emplace(p.getX(), p.getY(), p.getTimestamp() + timeOffset_, p.getPolarity());
            }
          }
        }
        break;
      }

    case FRAME_EVENT:
      if constexpr(std::is_same_v<T2, std::nullptr_t>) {
        break;
      } else {
        std::shared_ptr<const libcaer::events::FrameEventPacket> ptr = std::static_pointer_cast<libcaer::events::FrameEventPacket>(packet);
        for(const auto &p : *ptr) {
          ev::StampedMat mat;
          mat.t = p.getTimestamp() + timeOffset_;

          cv::Mat m16(p.getLengthY(), p.getLengthX(), CV_16UC1);
          std::memcpy(m16.data, p.getPixelArrayUnsafe(), sizeof(uint16_t) * p.getLengthY() * p.getLengthX());
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
        std::shared_ptr<const libcaer::events::IMU6EventPacket> ptr = std::static_pointer_cast<libcaer::events::IMU6EventPacket>(packet);
        for(const auto &p : *ptr) {
          ev::Imu data{};
          data.t = p.getTimestamp() + timeOffset_;
          data.linear_acceleration.x = -p.getAccelX() * ev::EARTH_GRAVITY;
          data.linear_acceleration.y = p.getAccelY() * ev::EARTH_GRAVITY;
          data.linear_acceleration.z = -p.getAccelZ() * ev::EARTH_GRAVITY;
          data.angular_velocity.x = -p.getGyroX() * ev::DEG2RAD;
          data.angular_velocity.y = p.getGyroY() * ev::DEG2RAD;
          data.angular_velocity.z = -p.getGyroZ() * ev::DEG2RAD;

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
