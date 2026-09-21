/*!
\file filtering.cpp
\brief Implementation of BackgroundActivityFilter and RefractoryPeriodFilter.
\author Raul Tapia
*/
#include "openev/evproc/filtering.hpp"
#include <cstddef>
#include <limits>
#include <opencv2/core/types.hpp>

ev::BackgroundActivityFilter::BackgroundActivityFilter(const cv::Size &size, const ev::TimeType dt, const int radius)
    : map_(size, std::numeric_limits<ev::TimeType>::lowest()), dt_{dt}, radius_{radius} {}

void ev::BackgroundActivityFilter::reset() {
  map_.setTo(std::numeric_limits<ev::TimeType>::lowest());
}

bool ev::BackgroundActivityFilter::operator()(const ev::Event &e) {
  const int x = e.x;
  const int y = e.y;
  map_.insert(e);

  const ev::TimeType threshold = e.t - dt_;
  for(int dy = -radius_; dy <= radius_; dy++) {
    for(int dx = -radius_; dx <= radius_; dx++) {
      if(dx == 0 && dy == 0) {
        continue;
      }
      const int nx = x + dx;
      const int ny = y + dy;
      if(nx >= 0 && nx < map_.cols && ny >= 0 && ny < map_.rows) {
        if(map_(ny, nx) >= threshold) {
          return true;
        }
      }
    }
  }

  return false;
}

ev::RefractoryPeriodFilter::RefractoryPeriodFilter(const cv::Size &size, const ev::TimeType dt)
    : map_(size, std::numeric_limits<ev::TimeType>::lowest()), dt_{dt} {}

void ev::RefractoryPeriodFilter::reset() {
  map_.setTo(std::numeric_limits<ev::TimeType>::lowest());
}

bool ev::RefractoryPeriodFilter::operator()(const ev::Event &e) {
  if(e.t - map_(e.y, e.x) < dt_) {
    return false;
  }

  map_.insert(e);
  return true;
}

ev::HotPixelFilter::HotPixelFilter(const cv::Size &size, const ev::TimeType window, const double factor, const std::size_t limit)
    : counts_(size, 0), mask_(size, 0), window_{window}, factor_{factor}, limit_{limit} {}

void ev::HotPixelFilter::reset() {
  counts_.setTo(0);
  mask_.setTo(0);
  started_ = false;
}

bool ev::HotPixelFilter::operator()(const ev::Event &e) {
  if(!started_) {
    start_ = e.t;
    started_ = true;
  } else if(e.t - start_ >= window_) {
    evaluate_();
    start_ = e.t;
  }

  counts_(e.y, e.x)++;
  return mask_(e.y, e.x) == 0;
}

void ev::HotPixelFilter::evaluate_() {
  std::size_t events = 0;
  std::size_t active = 0;
  for(const int count : counts_) {
    if(count > 0) {
      events += static_cast<std::size_t>(count);
      active++;
    }
  }

  const double mean = active > 0 ? static_cast<double>(events) / static_cast<double>(active) : 0.0;
  const double relative = factor_ > 0 ? factor_ * mean : std::numeric_limits<double>::max();
  const double absolute = limit_ > 0 ? static_cast<double>(limit_) : std::numeric_limits<double>::max();
  const double threshold = relative < absolute ? relative : absolute;

  auto count = counts_.begin();
  for(uchar &hot : mask_) {
    hot = static_cast<double>(*count) > threshold ? 255 : 0;
    *count = 0;
    ++count;
  }
}
