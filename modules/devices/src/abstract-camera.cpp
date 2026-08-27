/*!
\file abstract-camera.cpp
\brief Implementation of abstract-camera.
\author Raul Tapia
*/
#include "openev/devices/abstract-camera.hpp"
#include "libcaer/devices/device.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <libcaer/events/common.h>
#include <libcaer/events/frame.h>
#include <libcaer/events/imu6.h>
#include <libcaer/events/packetContainer.h>
#include <libcaer/events/polarity.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/persistence.hpp>
#include <opencv2/core/saturate.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <type_traits>

ev::AbstractCamera::~AbstractCamera() {
  stop();
  if(isOpen()) {
    caerDeviceClose(&deviceHandler_);
  }
}

void ev::AbstractCamera::stop() {
  if(isOpen() && running_.exchange(false)) {
    caerDeviceDataStop(deviceHandler_);
  }
}

cv::Rect_<uint16_t> ev::AbstractCamera::getRoi() const {
  if(roi_.width <= 0 || roi_.height <= 0) {
    const cv::Size size = getSensorSize();
    return {0, 0, static_cast<uint16_t>(size.width), static_cast<uint16_t>(size.height)};
  }
  return roi_;
}

void ev::AbstractCamera::setDefectivePixels(const std::string &defective_pixels_file) {
  if(!isOpen()) {
    CV_Error(cv::Error::StsError, "ev::AbstractCamera: the camera is not open.");
  }

  cv::FileStorage fs(defective_pixels_file, cv::FileStorage::READ);
  if(!fs.isOpened()) {
    CV_Error(cv::Error::StsError, "ev::AbstractCamera: could not open the defective pixel file.");
  }

  const cv::FileNode camera = fs[getSerialNumber()];
  if(camera.isNone() || camera.empty()) {
    CV_Error(cv::Error::StsBadArg, "ev::AbstractCamera: the defective pixel file does not contain this camera.");
  }

  std::vector<cv::Point> hot;
  std::vector<cv::Point> saturated;
  camera["hot_pixels"] >> hot;
  camera["saturated_pixels"] >> saturated;
  fs.release();

  const cv::Size size = getSensorSize();
  hotPixels_ = cv::Mat::zeros(size, CV_8UC1);
  for(const cv::Point &p : hot) {
    if(p.inside(cv::Rect({0, 0}, size))) {
      hotPixels_.at<uchar>(p) = 1;
    }
  }
  saturatedPixels_ = std::move(saturated);
  hasDefectivePixels_ = true;
}

void ev::AbstractCamera::setContainerInterval(const uint32_t usec) {
  // NOTE (libcaer): Must be at least 1 microsecond
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, (usec < 1 || usec > 600000000) ? 600000000 : usec);
}

void ev::AbstractCamera::setContainerSize(const uint32_t n) {
  // NOTE (libcaer): Set to zero to disable
  caerDeviceConfigSet(deviceHandler_, CAER_HOST_CONFIG_PACKETS, CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_PACKET_SIZE, n);
}

namespace {
// NOTE (libcaer): the container returned by caerDeviceDataGet must be freed
class Transmission {
public:
  explicit Transmission(caerDeviceHandle handle) : container_(caerDeviceDataGet(handle)) {}
  ~Transmission() {
    caerEventPacketContainerFree(container_);
  }
  Transmission(const Transmission &) = delete;
  Transmission(Transmission &&) noexcept = delete;
  Transmission &operator=(const Transmission &) = delete;
  Transmission &operator=(Transmission &&) noexcept = delete;

  [[nodiscard]] caerEventPacketContainerConst get() const {
    return container_;
  }

private:
  caerEventPacketContainer container_;
};

std::nullptr_t *const none = nullptr;

template <typename T>
bool notEmpty(T *dst) {
  if constexpr(std::is_same_v<T, std::nullptr_t>) {
    return false;
  } else {
    return !dst->empty();
  }
}
} // namespace

bool ev::AbstractCamera::getData(ev::Vector &events) { return getData_(&events, none, none); }

bool ev::AbstractCamera::getData(ev::Queue &events) { return getData_(&events, none, none); }

bool ev::AbstractCamera::getData(StampedMat &frame) { return getData_(none, &frame, none); }

bool ev::AbstractCamera::getData(StampedMatVector &frames) { return getData_(none, &frames, none); }

bool ev::AbstractCamera::getData(StampedMatQueue &frames) { return getData_(none, &frames, none); }

bool ev::AbstractCamera::getData(Imu &imu) { return getData_(none, none, &imu); }

bool ev::AbstractCamera::getData(ImuVector &imu) { return getData_(none, none, &imu); }

bool ev::AbstractCamera::getData(ImuQueue &imu) { return getData_(none, none, &imu); }

bool ev::AbstractCamera::getData(ev::Vector &events, ev::StampedMat &frame) { return getData_(&events, &frame, none); }

bool ev::AbstractCamera::getData(ev::Vector &events, ev::StampedMatVector &frames) { return getData_(&events, &frames, none); }

bool ev::AbstractCamera::getData(ev::Queue &events, ev::StampedMatQueue &frames) { return getData_(&events, &frames, none); }

bool ev::AbstractCamera::getData(ev::Vector &events, ev::Imu &imu) { return getData_(&events, none, &imu); }

bool ev::AbstractCamera::getData(ev::Vector &events, ev::ImuVector &imu) { return getData_(&events, none, &imu); }

bool ev::AbstractCamera::getData(ev::Queue &events, ev::ImuQueue &imu) { return getData_(&events, none, &imu); }

bool ev::AbstractCamera::getData(ev::Vector &events, ev::StampedMat &frame, ev::Imu &imu) { return getData_(&events, &frame, &imu); }

bool ev::AbstractCamera::getData(ev::Vector &events, ev::StampedMatVector &frames, ev::ImuVector &imu) { return getData_(&events, &frames, &imu); }

bool ev::AbstractCamera::getData(ev::Queue &events, ev::StampedMatQueue &frames, ev::ImuQueue &imu) { return getData_(&events, &frames, &imu); }

std::size_t ev::AbstractCamera::getEventRaw(std::vector<uint64_t> &data) { return getEventRaw_(data, false); }

std::size_t ev::AbstractCamera::getEventRaw(uint64_t *&data, const bool allow_realloc /*= true*/) { return getEventRaw_(data, allow_realloc); }

void ev::AbstractCamera::flush(const double msec) const {
  if(msec <= 0) {
    return;
  }
  const std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
  do {
    caerEventPacketContainerFree(caerDeviceDataGet(deviceHandler_));
  } while(static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0).count()) < msec);
}

template <typename T1, typename T2, typename T3>
bool ev::AbstractCamera::getData_(T1 *dvs, T2 *aps, T3 *imu) {
  static_assert(std::is_same_v<T1, std::nullptr_t> || std::is_same_v<T1, ev::Vector> || std::is_same_v<T1, ev::Queue>, "ev::AbstractCamera::getData_: unsupported dvs destination.");
  static_assert(std::is_same_v<T2, std::nullptr_t> || std::is_same_v<T2, ev::StampedMat> || std::is_same_v<T2, ev::StampedMatVector> || std::is_same_v<T2, ev::StampedMatQueue>, "ev::AbstractCamera::getData_: unsupported aps destination.");
  static_assert(std::is_same_v<T3, std::nullptr_t> || std::is_same_v<T3, ev::Imu> || std::is_same_v<T3, ev::ImuVector> || std::is_same_v<T3, ev::ImuQueue>, "ev::AbstractCamera::getData_: unsupported imu destination.");

  if constexpr(std::is_same_v<T2, ev::StampedMat>) {
    aps->release();
  }
  if constexpr(std::is_same_v<T3, ev::Imu>) {
    imu->release();
  }

  const Transmission transmission(deviceHandler_);
  const caerEventPacketContainerConst container = transmission.get();
  if(container == nullptr) {
    CV_LOG_WARNING(nullptr, "Connection with camera lost.");
    return false;
  }

  const int32_t container_size = caerEventPacketContainerGetEventPacketsNumber(container);
  for(int32_t i = 0; i < container_size; i++) {
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
          dvs->reserve(dvs->size() + static_cast<std::size_t>(packet_size));
        }
        for(int32_t k = 0; k < packet_size; k++) {
          const caerPolarityEventConst p = caerPolarityEventPacketGetEventConst(reinterpret_cast<caerPolarityEventPacketConst>(packet), k);
          const uint16_t x = caerPolarityEventGetX(p);
          const uint16_t y = caerPolarityEventGetY(p);
          if(hasDefectivePixels_ && hotPixels_.at<uchar>(y, x)) {
            continue;
          }
          if(!filterRoiInSoftware_ || roi_.contains(cv::Point(x, y))) {
            if constexpr(std::is_same_v<T1, ev::Vector>) {
              dvs->emplace_back(x, y, caerPolarityEventGetTimestamp(p), caerPolarityEventGetPolarity(p));
            } else if constexpr(std::is_same_v<T1, ev::Queue>) {
              dvs->emplace(x, y, caerPolarityEventGetTimestamp(p), caerPolarityEventGetPolarity(p));
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
          mat.t = caerFrameEventGetTimestamp(p);

          const int32_t x = caerFrameEventGetLengthX(p);
          const int32_t y = caerFrameEventGetLengthY(p);
          const cv::Mat m16(y, x, CV_16UC1);
          std::memcpy(m16.data, p->pixels, sizeof(uint16_t) * y * x);
          m16.convertTo(mat, CV_8UC1, ev::SCALE_16B_8B);
          if(hasDefectivePixels_) {
            interpolate_(mat);
          }

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
          data.t = caerIMU6EventGetTimestamp(p);
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

    default:
      break;
    }
  }

  return notEmpty(dvs) || notEmpty(aps) || notEmpty(imu);
}

template <typename T>
std::size_t ev::AbstractCamera::getEventRaw_(T &data, [[maybe_unused]] const bool allow_realloc) {
  static_assert(std::is_same_v<T, std::vector<uint64_t>> || std::is_same_v<T, uint64_t *>, "ev::AbstractCamera::getEventRaw_: unsupported buffer type.");

  std::size_t idx = 0;
  [[maybe_unused]] std::size_t size = 0;

  const Transmission transmission(deviceHandler_);
  const caerEventPacketContainerConst container = transmission.get();
  if(container == nullptr) {
    CV_LOG_WARNING(nullptr, "Connection with camera lost.");
    return 0;
  }

  const int32_t container_size = caerEventPacketContainerGetEventPacketsNumber(container);
  for(int32_t i = 0; i < container_size; i++) {
    const caerEventPacketHeaderConst packet = caerEventPacketContainerGetEventPacketConst(container, i);
    if(packet == nullptr || caerEventPacketHeaderGetEventType(packet) != POLARITY_EVENT) {
      continue;
    }

    const int32_t packet_size = caerEventPacketHeaderGetEventNumber(packet);
    if constexpr(std::is_same_v<T, std::vector<uint64_t>>) {
      data.reserve(data.size() + static_cast<std::size_t>(packet_size));
    } else {
      size += static_cast<std::size_t>(packet_size);
      if(allow_realloc) {
        auto *const resized = static_cast<uint64_t *>(realloc(data, size * sizeof(uint64_t)));
        if(resized == nullptr) {
          CV_Error(cv::Error::StsNoMem, "ev::AbstractCamera: Could not resize the raw event buffer.");
        }
        data = resized;
      }
    }

    for(int32_t k = 0; k < packet_size; k++) {
      const caerPolarityEventConst p = caerPolarityEventPacketGetEventConst(reinterpret_cast<caerPolarityEventPacketConst>(packet), k);
      if(p != nullptr) {
        const uint64_t event = (static_cast<uint64_t>(p->data) << 32) | static_cast<uint64_t>(p->timestamp);
        if constexpr(std::is_same_v<T, std::vector<uint64_t>>) {
          data.emplace_back(event);
        } else {
          data[idx] = event;
        }
        idx++;
      }
    }
  }

  return idx;
}

void ev::AbstractCamera::interpolate_(cv::Mat &frame) const {
  const cv::Point offset(roi_.width > 0 ? roi_.x : 0, roi_.height > 0 ? roi_.y : 0);
  std::array<uchar, 8> neighbours{};

  const cv::Rect bounds({0, 0}, frame.size());
  for(const cv::Point &p : saturatedPixels_) {
    const cv::Point q = p - offset;
    if(!q.inside(bounds)) {
      continue;
    }

    std::size_t n = 0;
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        const cv::Point neighbour(p.x + dx, p.y + dy);
        if((dx == 0 && dy == 0) || !cv::Point(q.x + dx, q.y + dy).inside(bounds) || std::find(saturatedPixels_.begin(), saturatedPixels_.end(), neighbour) != saturatedPixels_.end()) {
          continue;
        }
        neighbours.at(n++) = frame.at<uchar>(q.y + dy, q.x + dx);
      }
    }
    if(n == 0) {
      continue;
    }
    std::sort(neighbours.begin(), neighbours.begin() + static_cast<long>(n));
    frame.at<uchar>(q) = neighbours.at(n / 2);
  }
}
