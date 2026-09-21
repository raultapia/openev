#ifndef OPENEV_REPRESENTATIONS_SPACE_TIME_TPP
#define OPENEV_REPRESENTATIONS_SPACE_TIME_TPP

#ifndef OPENEV_REPRESENTATIONS_SPACE_TIME_HPP
#include "openev/representations/space-time.hpp"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <opencv2/imgproc.hpp>

namespace ev {

template <typename T, const RepresentationOptions Options, typename E>
SpaceTime_<T, Options, E>::SpaceTime_(const cv::Size canvas, const cv::Size sensor /*= cv::Size()*/) : EventImage_<T, Options, E>(canvas), sensor_(sensor.empty() ? canvas : sensor) {
  fit_();
  EventImage_<T, Options, E>::clear();
}

template <typename T, const RepresentationOptions Options, typename E>
void SpaceTime_<T, Options, E>::setView(const double yaw, const double pitch) {
  yaw_ = yaw;
  pitch_ = pitch;
  fit_();
}

template <typename T, const RepresentationOptions Options, typename E>
void SpaceTime_<T, Options, E>::setDepth(const double depth) {
  depth_ = std::max(depth, 0.0);
  fit_();
}

template <typename T, const RepresentationOptions Options, typename E>
void SpaceTime_<T, Options, E>::fit_() {
  const double yaw = yaw_ * CV_PI / 180.0;
  const double pitch = pitch_ * CV_PI / 180.0;
  const cv::Matx33d aroundVertical(std::cos(yaw), 0, std::sin(yaw), 0, 1, 0, -std::sin(yaw), 0, std::cos(yaw));
  const cv::Matx33d aroundHorizontal(1, 0, 0, 0, std::cos(pitch), -std::sin(pitch), 0, std::sin(pitch), std::cos(pitch));
  rotation_ = aroundHorizontal * aroundVertical;

  scale_ = 1;
  center_ = {0, 0};
  const double halfHeight = 0.5 * sensor_.height / sensor_.width;
  double left = std::numeric_limits<double>::max();
  double right = std::numeric_limits<double>::lowest();
  double top = std::numeric_limits<double>::max();
  double bottom = std::numeric_limits<double>::lowest();
  for(const double x : {-0.5, 0.5}) {
    for(const double y : {-halfHeight, halfHeight}) {
      for(const double z : {-0.5 * depth_, 0.5 * depth_}) {
        const cv::Point2d corner = project_({x, y, z});
        left = std::min(left, corner.x);
        right = std::max(right, corner.x);
        top = std::min(top, corner.y);
        bottom = std::max(bottom, corner.y);
      }
    }
  }

  const double width = std::max(right - left, std::numeric_limits<double>::epsilon());
  const double height = std::max(bottom - top, std::numeric_limits<double>::epsilon());
  scale_ = std::min((this->cols - 2 * MARGIN) / width, (this->rows - 2 * MARGIN) / height);
  center_ = {0.5 * this->cols - scale_ * 0.5 * (left + right), 0.5 * this->rows - scale_ * 0.5 * (top + bottom)};
  warp_();
}

template <typename T, const RepresentationOptions Options, typename E>
void SpaceTime_<T, Options, E>::warp_() {
  if(frame_.empty()) {
    return;
  }
  const auto width = static_cast<float>(sensor_.width);
  const auto height = static_cast<float>(sensor_.height);
  const std::array<cv::Point2f, 4> source{cv::Point2f(0, 0), cv::Point2f(width, 0), cv::Point2f(width, height), cv::Point2f(0, height)};
  std::array<cv::Point2f, 4> target;
  for(std::size_t i = 0; i < source.size(); i++) {
    target.at(i) = cv::Point2f(corner_(source.at(i).x, source.at(i).y, 1.0));
  }
  const cv::Mat homography = cv::getPerspectiveTransform(source.data(), target.data());
  const cv::Scalar border = TypeHelper<T>::convert(this->V_RESET);
  if(frame_.depth() == CV_32S) {
    cv::Mat real;
    frame_.convertTo(real, CV_64F);
    cv::warpPerspective(real, real, homography, cv::Mat_<T>::size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, border);
    real.convertTo(backdrop_, frame_.type());
  } else {
    cv::warpPerspective(frame_, backdrop_, homography, cv::Mat_<T>::size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, border);
  }
}

template <typename T, const RepresentationOptions Options, typename E>
cv::Point2d SpaceTime_<T, Options, E>::project_(const cv::Point3d &point) const {
  const cv::Vec3d rotated = rotation_ * cv::Vec3d(point.x, point.y, point.z);
  const double distance = rotated[2] + DISTANCE;
  return {center_.x + scale_ * rotated[0] / distance, center_.y + scale_ * rotated[1] / distance};
}

template <typename T, const RepresentationOptions Options, typename E>
cv::Point2d SpaceTime_<T, Options, E>::corner_(const double x, const double y, const double age) const {
  return project_({x / sensor_.width - 0.5, (y - 0.5 * sensor_.height) / sensor_.width, (age - 0.5) * depth_});
}

template <typename T, const RepresentationOptions Options, typename E>
void SpaceTime_<T, Options, E>::drawAxes_() {
  const cv::Scalar color = TypeHelper<T>::convert(axesValue_);
  const cv::Point origin = corner_(0, 0, 0);
  const std::array<std::pair<cv::Point, const char *>, 3> ends{std::make_pair(cv::Point(corner_(axesLength_ * sensor_.width, 0, 0)), "x"), std::make_pair(cv::Point(corner_(0, axesLength_ * sensor_.height, 0)), "y"), std::make_pair(cv::Point(corner_(0, 0, axesLength_)), "t")};
  for(const auto &[end, label] : ends) {
    cv::line(*this, origin, end, color, 1, cv::LINE_8);
    const cv::Point2d direction = cv::Point2d(end - origin) / std::max(cv::norm(end - origin), 1.0);
    cv::putText(*this, label, end + cv::Point(direction * 4.0) + cv::Point(-3, 4), cv::FONT_HERSHEY_SIMPLEX, LABEL_SCALE, color, 1, cv::LINE_8);
  }
}

template <typename T, const RepresentationOptions Options, typename E>
typename TypeHelper<T>::Type SpaceTime_<T, Options, E>::faded_(const typename TypeHelper<T>::Type &value, const cv::Point &pixel, const double age) const {
  const double alpha = fadeStrength_ * age;
  const cv::Scalar front = TypeHelper<T>::convert(value);
  const cv::Scalar back = TypeHelper<T>::convert(backdrop_.empty() ? this->V_RESET : backdrop_.template at<typename TypeHelper<T>::Type>(pixel));
  return TypeHelper<T>::convert(cv::viz::Color(front * (1.0 - alpha) + back * alpha));
}

template <typename T, const RepresentationOptions Options, typename E>
cv::Point2d SpaceTime_<T, Options, E>::project(const Event_<E> &e, const double age) const {
  const double x = (static_cast<double>(e.x) + 0.5) / sensor_.width - 0.5;
  const double y = (static_cast<double>(e.y) + 0.5 - 0.5 * sensor_.height) / sensor_.width;
  const double z = (std::clamp(age, 0.0, 1.0) - 0.5) * depth_;
  return project_({x, y, z});
}

template <typename T, const RepresentationOptions Options, typename E>
cv::Mat &SpaceTime_<T, Options, E>::render() {
  if(backdrop_.empty()) {
    cv::Mat_<T>::setTo(this->V_RESET);
  } else {
    backdrop_.copyTo(*this);
  }
  if(axes_) {
    drawAxes_();
  }

  const TimeType oldest = this->tLimits_[this->MIN];
  const TimeType span = this->tLimits_[this->MAX] - oldest;
  const cv::Rect canvas(0, 0, this->cols, this->rows);
  for(const Event_<E> &e : events_) {
    const double age = span > 0 ? 1.0 - (e.t - oldest) / span : 0.0;
    const cv::Point2d projected = project(e, age);
    const cv::Point pixel(static_cast<int>(std::floor(projected.x)), static_cast<int>(std::floor(projected.y)));
    if(pixel.inside(canvas)) {
      const typename TypeHelper<T>::Type &value = e.p ? this->V_ON : this->V_OFF;
      cv::Mat_<T>::operator()(pixel.y, pixel.x) = fade_ ? faded_(value, pixel, age) : value;
    }
  }
  return *this;
}

template <typename T, const RepresentationOptions Options, typename E>
void SpaceTime_<T, Options, E>::clear_() {
  events_.clear();
  frame_.release();
  backdrop_.release();
  cv::Mat_<T>::setTo(this->V_RESET);
}

template <typename T, const RepresentationOptions Options, typename E>
void SpaceTime_<T, Options, E>::clear_(const cv::Mat &background) {
  events_.clear();
  background.copyTo(frame_);
  warp_();
  backdrop_.copyTo(*this);
}

template <typename T, const RepresentationOptions Options, typename E>
bool SpaceTime_<T, Options, E>::insert_(const Event_<E> &e) {
  if(e.inside(cv::Rect(0, 0, sensor_.width, sensor_.height))) {
    events_.push_back(e);
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_SPACE_TIME_TPP
