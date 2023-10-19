/*!
\file abstract-camera.cpp
\brief Implementation of AbstractCamera_ class.
\author Raul Tapia
*/
#include "openev/camera.hpp"

void ev::AbstractCamera_::start() {
  caerDeviceDataStart(deviceHandler_, nullptr, nullptr, nullptr, nullptr, nullptr);
  flush(1);
}

void ev::AbstractCamera_::stop() {
  caerDeviceDataStop(deviceHandler_);
}

cv::Size ev::AbstractCamera_::getSensorSize() const {
  struct caer_davis_info info = caerDavisInfoGet(deviceHandler_);
  return {info.dvsSizeX, info.dvsSizeY};
}

cv::Rect ev::AbstractCamera_::getRoi() const {
  if(roi_.empty()) {
    struct caer_davis_info info = caerDavisInfoGet(deviceHandler_);
    return {0, 0, info.dvsSizeX, info.dvsSizeY};
  }
  return roi_;
}

bool ev::AbstractCamera_::setRoi(const cv::Rect &roi) {
  struct caer_davis_info info = caerDavisInfoGet(deviceHandler_);
  cv::Rect full(0, 0, info.dvsSizeX, info.dvsSizeY);

  if(!roi.empty() && full.contains(roi.tl()) && full.contains(roi.br())) {
    if(caerDavisROIConfigure(deviceHandler_, roi.tl().x, roi.tl().y, roi.br().x - 1, roi.br().y - 1)) {
      roi_ = roi;
      return true;
    }
  }
  return false;
}

void ev::AbstractCamera_::flush(double msec) const {
  auto t0 = std::chrono::system_clock::now();
  auto tn = std::chrono::system_clock::now();
  do {
    caerDeviceDataGet(deviceHandler_);
    tn = std::chrono::system_clock::now();
  } while(static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(tn - t0).count()) < msec);
}
