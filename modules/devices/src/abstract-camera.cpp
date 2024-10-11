/*!
\file abstract-camera.cpp
\brief Implementation of abstract-camera.
\author Raul Tapia
*/
#include "openev/devices/abstract-camera.hpp"
#include "libcaer/devices/davis.h"
#include "libcaer/devices/device.h"
#include <chrono>

ev::AbstractCamera::~AbstractCamera() {
  caerDeviceDataStop(deviceHandler_);
  caerDeviceClose(&deviceHandler_);
}

void ev::AbstractCamera::start() {
  init();
  flush(1);
}

void ev::AbstractCamera::stop() {
  caerDeviceDataStop(deviceHandler_);
}

cv::Size ev::AbstractCamera::getSensorSize() const {
  struct caer_davis_info info = caerDavisInfoGet(deviceHandler_);
  return {info.dvsSizeX, info.dvsSizeY};
}

cv::Rect_<uint16_t> ev::AbstractCamera::getRoi() const {
  if(roi_.empty()) {
    struct caer_davis_info info = caerDavisInfoGet(deviceHandler_);
    return {0, 0, static_cast<uint16_t>(info.dvsSizeX), static_cast<uint16_t>(info.dvsSizeY)};
  }
  return roi_;
}

bool ev::AbstractCamera::setRoi(const cv::Rect_<uint16_t> &roi) {
  struct caer_davis_info info = caerDavisInfoGet(deviceHandler_);
  cv::Rect_<uint16_t> full(0, 0, static_cast<uint16_t>(info.dvsSizeX), static_cast<uint16_t>(info.dvsSizeY));

  if(!roi.empty() && full.contains(roi.tl()) && full.contains(roi.br())) {
    if(caerDavisROIConfigure(deviceHandler_, roi.tl().x, roi.tl().y, static_cast<uint16_t>(roi.br().x - 1), static_cast<uint16_t>(roi.br().y - 1))) {
      roi_ = roi;
      return true;
    }
  }
  return false;
}

ev::BiasValue ev::AbstractCamera::getBias(const int8_t config_bias, const uint8_t name) const {
  uint32_t param{0};
  caerDeviceConfigGet(deviceHandler_, config_bias, name, &param);
  return {caerBiasCoarseFineParse(static_cast<uint16_t>(param)).coarseValue, caerBiasCoarseFineParse(static_cast<uint16_t>(param)).fineValue};
}

bool ev::AbstractCamera::setBias(const int8_t config_bias, const uint8_t name, const BiasValue &value) {
  uint32_t param{0};
  caerDeviceConfigGet(deviceHandler_, config_bias, name, &param);
  struct caer_bias_coarsefine cf = caerBiasCoarseFineParse(static_cast<uint16_t>(param));
  cf.coarseValue = value.coarse;
  cf.fineValue = value.fine;
  return caerDeviceConfigSet(deviceHandler_, config_bias, name, caerBiasCoarseFineGenerate(cf));
}

void ev::AbstractCamera::flush(const double msec) const {
  if(msec <= 0) {
    return;
  }
  const std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
  do {
    caerDeviceDataGet(deviceHandler_);
  } while(static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0).count()) < msec);
}
