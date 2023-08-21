/*!
\file davis.cpp
\brief Implementation of Davis class.
\author Raul Tapia
*/
#include "openev/camera.hpp"
#include "openev/logger.hpp"

ev::Davis::Davis(const ev::device model) {
  deviceHandler_ = caerDeviceOpen(0, CAER_DEVICE_DAVIS, 0, 0, "");
  if(deviceHandler_ == nullptr) {
    ev::logger::error("Could not find camera.");
  } else {
    caerDeviceSendDefaultConfig(deviceHandler_);

    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_RUN, 1U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_ACCELEROMETER, 0U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_GYROSCOPE, 0U);
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_IMU, DAVIS_CONFIG_IMU_RUN_TEMPERATURE, 0U);

    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_DATAEXCHANGE, CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, 1U);                       // Blocking mode
    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, ev::DEFAULT_INTERVAL); // 50Hz == 20000us
    caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_PACKET_SIZE, 0U);                // Set to zero to disable
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_FRAME_INTERVAL, ev::DEFAULT_INTERVAL);                         // 50Hz == 20000us
    caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_APS, DAVIS_CONFIG_APS_EXPOSURE, ev::DEFAULT_EXPOSURE);                               // 6500 us

    switch(model) {
    case device::NONE:
      break;
    case device::DAVIS346:
      caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_BIAS, DAVIS346_CONFIG_BIAS_PRBP, caerBiasCoarseFineGenerate(GET_DAVIS_346_BIAS_1()));
      caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_BIAS, DAVIS346_CONFIG_BIAS_PRSFBP, caerBiasCoarseFineGenerate(GET_DAVIS_346_BIAS_2()));
      ev::logger::info("DAVIS346 device configured.");
      break;
    }
  }
}

ev::Davis::~Davis() {
  caerDeviceDataStop(deviceHandler_);
  caerDeviceClose(&deviceHandler_);
}

void ev::Davis::enableDvs(const bool state) {
  caerDeviceConfigSet(deviceHandler_, DAVIS_CONFIG_DVS, DAVIS_CONFIG_DVS_RUN, static_cast<uint32_t>(state));
}

void ev::Davis::setDvsTimeInterval(const uint32_t usec) {
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, usec);
}

void ev::Davis::setDvsEventsPerPacket(const uint32_t n) {
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

bool ev::Davis::getData(ev::EventPacket &events) {
  getData_<ev::EventPacket, std::nullptr_t, std::nullptr_t>(&events, nullptr, nullptr);
  return !events.empty();
}

bool ev::Davis::getData(ev::EventBuffer &events) {
  getData_<ev::EventBuffer, std::nullptr_t, std::nullptr_t>(&events, nullptr, nullptr);
  return !events.empty();
}

bool ev::Davis::getData(StampedMat &frame) {
  frame.release();
  getData_<std::nullptr_t, ev::StampedMat, std::nullptr_t>(nullptr, &frame, nullptr);
  return !frame.empty();
}

bool ev::Davis::getData(StampedMatPacket &frames) {
  getData_<std::nullptr_t, ev::StampedMatPacket, std::nullptr_t>(nullptr, &frames, nullptr);
  return !frames.empty();
}

bool ev::Davis::getData(StampedMatBuffer &frames) {
  getData_<std::nullptr_t, ev::StampedMatBuffer, std::nullptr_t>(nullptr, &frames, nullptr);
  return !frames.empty();
}

bool ev::Davis::getData(Imu &imu) {
  imu.release();
  getData_<std::nullptr_t, ev::Imu, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis::getData(ImuPacket &imu) {
  getData_<std::nullptr_t, ev::ImuPacket, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis::getData(ImuBuffer &imu) {
  getData_<std::nullptr_t, ev::ImuBuffer, std::nullptr_t>(nullptr, &imu, nullptr);
  return !imu.empty();
}

bool ev::Davis::getData(ev::EventPacket &events, ev::StampedMat &frame) {
  frame.release();
  getData_<ev::EventPacket, ev::StampedMat, std::nullptr_t>(&events, &frame, nullptr);
  return (!events.empty() || !frame.empty());
}

bool ev::Davis::getData(ev::EventPacket &events, ev::StampedMatPacket &frames) {
  getData_<ev::EventPacket, ev::StampedMatPacket, std::nullptr_t>(&events, &frames, nullptr);
  return (!events.empty() || !frames.empty());
}

bool ev::Davis::getData(ev::EventBuffer &events, ev::StampedMatBuffer &frames) {
  getData_<ev::EventBuffer, ev::StampedMatBuffer, std::nullptr_t>(&events, &frames, nullptr);
  return (!events.empty() || !frames.empty());
}

bool ev::Davis::getData(ev::EventPacket &events, ev::Imu &imu) {
  imu.release();
  getData_<ev::EventPacket, std::nullptr_t, ev::Imu>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::EventPacket &events, ev::ImuPacket &imu) {
  getData_<ev::EventPacket, std::nullptr_t, ev::ImuPacket>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::EventBuffer &events, ev::ImuBuffer &imu) {
  getData_<ev::EventBuffer, std::nullptr_t, ev::ImuBuffer>(&events, nullptr, &imu);
  return (!events.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::EventPacket &events, ev::StampedMat &frame, ev::Imu &imu) {
  frame.release();
  imu.release();
  getData_<ev::EventPacket, ev::StampedMat, ev::Imu>(&events, &frame, &imu);
  return (!events.empty() || !frame.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::EventPacket &events, ev::StampedMatPacket &frames, ev::ImuPacket &imu) {
  getData_<ev::EventPacket, ev::StampedMatPacket, ev::ImuPacket>(&events, &frames, &imu);
  return (!events.empty() || !frames.empty() || !imu.empty());
}

bool ev::Davis::getData(ev::EventBuffer &events, ev::StampedMatBuffer &frames, ev::ImuBuffer &imu) {
  getData_<ev::EventBuffer, ev::StampedMatBuffer, ev::ImuBuffer>(&events, &frames, &imu);
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
void ev::Davis::getData_(T1 *dvs, T2 *aps, T3 *imu) {
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
        if constexpr(std::is_same_v<T1, ev::EventPacket>) {
          dvs->reserve(dvs->size() + ptr->size());
          for(const auto &p : *ptr) {
            if(roi_.empty() || roi_.contains(cv::Point(p.getX(), p.getY()))) {
              dvs->emplace_back(p.getX(), p.getY(), p.getTimestamp() + timeOffset_, p.getPolarity());
            }
          }
        }
        if constexpr(std::is_same_v<T1, ev::EventBuffer>) {
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
          if constexpr(std::is_same_v<T2, ev::StampedMatPacket>) {
            aps->push_back(mat);
          }
          if constexpr(std::is_same_v<T2, ev::StampedMatBuffer>) {
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
          if constexpr(std::is_same_v<T3, ev::ImuPacket>) {
            imu->push_back(data);
          }
          if constexpr(std::is_same_v<T3, ev::ImuBuffer>) {
            imu->push(data);
          }
        }
        break;
      }
    }
  }
}
