#ifndef OPENEV_REPRESENTATIONS_ABSTRACT_REPRESENTATION_TPP
#define OPENEV_REPRESENTATIONS_ABSTRACT_REPRESENTATION_TPP

#ifndef OPENEV_REPRESENTATIONS_ABSTRACT_REPRESENTATION_HPP
#include "openev/representation/abstract-representation.hpp"
#endif

#include "openev/core/types.hpp"

namespace ev {

template <typename T, const RepresentationOptions Options, typename E>
void AbstractRepresentation_<T, Options, E>::clear() {
  count_ = 0;
  tLimits_ = {std::numeric_limits<TimeType>::max(), std::numeric_limits<TimeType>::min()};
  background_.release();
  clear_();
}

template <typename T, const RepresentationOptions Options, typename E>
void AbstractRepresentation_<T, Options, E>::clear(const cv::Mat &background, const cv::Point &origin /*= {0, 0}*/) {
  count_ = 0;
  tLimits_ = {std::numeric_limits<TimeType>::max(), std::numeric_limits<TimeType>::min()};

  cv::Mat converted;
  if(background.channels() != TypeHelper<T>::NumChannels) {
    if(background.channels() == 1 && TypeHelper<T>::NumChannels == 3) {
      cv::cvtColor(background, converted, cv::COLOR_GRAY2BGR);
    } else if(background.channels() == 3 && TypeHelper<T>::NumChannels == 1) {
      cv::cvtColor(background, converted, cv::COLOR_BGR2GRAY);
    }
  } else {
    converted = background;
  }

  const cv::Size frame = frameSize_();
  if(frame.empty()) {
    converted.copyTo(background_);
  } else {
    const cv::Rect placed(origin, converted.size());
    if((placed & cv::Rect({0, 0}, frame)) != placed) {
      CV_Error(cv::Error::StsBadSize, "ev::AbstractRepresentation::clear: the background does not fit in the representation.");
    }
    if(placed.size() == frame) {
      converted.copyTo(background_);
    } else {
      background_.create(frame, converted.type());
      background_.setTo(V_RESET);
      converted.copyTo(background_(placed));
    }
  }
  clear_(background_);
}

template <typename T, const RepresentationOptions Options, typename E>
bool AbstractRepresentation_<T, Options, E>::insert(const Event_<E> &e) {
  if constexpr(REPRESENTATION_OPTION_CHECK(Options, RepresentationOptions::ONLY_IF_POSITIVE)) {
    if(e.p == ev::NEGATIVE) {
      return false;
    }
  }

  if constexpr(REPRESENTATION_OPTION_CHECK(Options, RepresentationOptions::ONLY_IF_NEGATIVE)) {
    if(e.p == ev::POSITIVE) {
      return false;
    }
  }

  if constexpr(REPRESENTATION_OPTION_CHECK(Options, RepresentationOptions::IGNORE_POLARITY)) {
    if constexpr(std::is_floating_point<E>::value) {
      if(insert_({std::round(e.x), std::round(e.y), e.t + timeOffset_, ev::POSITIVE})) {
        tLimits_[MIN] = std::min(tLimits_[MIN], e.t + timeOffset_);
        tLimits_[MAX] = std::max(tLimits_[MAX], e.t + timeOffset_);
        count_++;
        return true;
      }
      return false;
    } else {
      if(insert_({e.x, e.y, e.t + timeOffset_, ev::POSITIVE})) {
        tLimits_[MIN] = std::min(tLimits_[MIN], e.t + timeOffset_);
        tLimits_[MAX] = std::max(tLimits_[MAX], e.t + timeOffset_);
        count_++;
        return true;
      }
      return false;
    }
  } else {
    if constexpr(std::is_floating_point<E>::value) {
      if(insert_({std::round(e.x), std::round(e.y), e.t + timeOffset_, e.p})) {
        tLimits_[MIN] = std::min(tLimits_[MIN], e.t + timeOffset_);
        tLimits_[MAX] = std::max(tLimits_[MAX], e.t + timeOffset_);
        count_++;
        return true;
      }
      return false;
    } else {
      if(insert_({e.x, e.y, e.t + timeOffset_, e.p})) {
        tLimits_[MIN] = std::min(tLimits_[MIN], e.t + timeOffset_);
        tLimits_[MAX] = std::max(tLimits_[MAX], e.t + timeOffset_);
        count_++;
        return true;
      }
      return false;
    }
  }
}

template <typename T, const RepresentationOptions Options, typename E>
template <std::size_t N>
bool AbstractRepresentation_<T, Options, E>::insert(const Array_<E, N> &array) {
  return std::all_of(array.begin(), array.end(), [this](const Event_<E> &e) { return this->insert(e); });
}

template <typename T, const RepresentationOptions Options, typename E>
bool AbstractRepresentation_<T, Options, E>::insert(const Vector_<E> &vector) {
  return std::all_of(vector.begin(), vector.end(), [this](const Event_<E> &e) { return this->insert(e); });
}

template <typename T, const RepresentationOptions Options, typename E>
bool AbstractRepresentation_<T, Options, E>::insert(Queue_<E> &queue, const bool keep_events_in_queue /*= false*/) {
  bool ret = true;
  if(keep_events_in_queue) {
    const std::size_t size = queue.size();
    for(std::size_t i = 0; i < size; i++) {
      if(!insert(queue.front())) {
        ret = false;
      }
      queue.emplace(queue.front());
      queue.pop();
    }
  } else {
    while(!queue.empty()) {
      if(!insert(queue.front())) {
        ret = false;
      }
      queue.pop();
    }
  }
  return ret;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_ABSTRACT_REPRESENTATION_TPP
