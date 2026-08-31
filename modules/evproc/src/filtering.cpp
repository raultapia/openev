/*!
\file filtering.cpp
\brief Implementation of BackgroundActivityFilter and RefractoryPeriodFilter.
\author Raul Tapia
*/
#include "openev/evproc/filtering.hpp"
#include <limits>
#include <opencv2/core/types.hpp>

ev::BackgroundActivityFilter::BackgroundActivityFilter(const cv::Size &size, const ev::TimeType dt, const int radius)
    : map_(size, std::numeric_limits<ev::TimeType>::lowest()), dt_{dt}, radius_{radius} {}

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

bool ev::RefractoryPeriodFilter::operator()(const ev::Event &e) {
  if(e.t - map_(e.y, e.x) < dt_) {
    return false;
  }

  map_.insert(e);
  return true;
}
