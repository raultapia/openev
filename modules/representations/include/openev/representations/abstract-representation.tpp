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
  tLimits_ = {DBL_MAX, DBL_MIN};
  clear_();
}

template <typename T, const RepresentationOptions Options, typename E>
void AbstractRepresentation_<T, Options, E>::clear(const cv::Mat &background) {
  count_ = 0;
  tLimits_ = {DBL_MAX, DBL_MIN};

  if(background.channels() != TypeHelper<T>::NumChannels) {
    cv::Mat temp;
    if(background.channels() == 1 && TypeHelper<T>::NumChannels == 3) {
      cv::cvtColor(background, temp, cv::COLOR_GRAY2BGR);
    } else if(background.channels() == 3 && TypeHelper<T>::NumChannels == 1) {
      cv::cvtColor(background, temp, cv::COLOR_BGR2GRAY);
    }
    clear_(temp);
  } else {
    clear_(background);
  }
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
