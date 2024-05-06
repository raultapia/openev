/*!
\file abstract-camera.cpp
\brief Implementation of abstract-camera.
\author Raul Tapia
*/
#include "openev/devices/abstract-camera.hpp"
#include "libcaer/devices/davis.h"
#include "libcaer/devices/device.h"
#include <chrono>

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

ev::BiasValue ev::AbstractCamera_::getBias(const int8_t config_bias, const uint8_t name) const {
  uint32_t param{0};
  caerDeviceConfigGet(deviceHandler_, config_bias, name, &param);
  return {caerBiasCoarseFineParse(param).coarseValue, caerBiasCoarseFineParse(param).fineValue};
}

bool ev::AbstractCamera_::setBias(const int8_t config_bias, const uint8_t name, const BiasValue &value) {
  uint32_t param{0};
  caerDeviceConfigGet(deviceHandler_, config_bias, name, &param);
  struct caer_bias_coarsefine cf = caerBiasCoarseFineParse(param);
  cf.coarseValue = value.coarse;
  cf.fineValue = value.fine;
  return caerDeviceConfigSet(deviceHandler_, config_bias, name, caerBiasCoarseFineGenerate(cf));
}

void ev::AbstractCamera_::flush(double msec) const {
  const std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
  do {
    caerDeviceDataGet(deviceHandler_);
  } while(static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0).count()) < msec);
}
