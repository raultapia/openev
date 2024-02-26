#ifndef OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP
#define OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP

#ifndef OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_HPP
#include "openev/representations/event-histogram.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options>
void EventHistogram_<T, Options>::render() {
  cv::Mat_<double> on;
  cv::Mat_<double> off;
  if(cv::countNonZero(counter[ev::POSITIVE])) {
    cv::normalize(counter[ev::POSITIVE], on, 0, 1, cv::NORM_MINMAX, -1, cv::noArray());
  }
  if(cv::countNonZero(counter[ev::NEGATIVE])) {
    cv::normalize(counter[ev::NEGATIVE], off, 0, 1, cv::NORM_MINMAX, -1, cv::noArray());
  }

  if constexpr(TypeHelper<T>::NumChannels == 1) {
    cv::Mat_<T>(on * (EventHistogram_<T, Options>::ON - EventHistogram_<T, Options>::RESET) + off * (EventHistogram_<T, Options>::OFF - EventHistogram_<T, Options>::RESET) + EventHistogram_<T, Options>::RESET).copyTo(*this);
  } else {
    std::vector<typename TypeHelper<T>::ChannelType> v(TypeHelper<T>::NumChannels);
    cv::parallel_for_(cv::Range(0, TypeHelper<T>::NumChannels), [&](const cv::Range &range) {
      const int start = range.start;
      const int end = range.end;
      for(int i = start; i < end; i++) {
        v[i] = on * (EventHistogram_<T, Options>::ON[i] - EventHistogram_<T, Options>::RESET[i]) + off * (EventHistogram_<T, Options>::OFF[i] - EventHistogram_<T, Options>::RESET[i]) + EventHistogram_<T, Options>::RESET[i];
      }
    });
    cv::merge(v, *this);
  }
}

template <typename T, const RepresentationOptions Options>
void EventHistogram_<T, Options>::clear_() {
  EventImage_<T, Options>::setTo(EventHistogram_<T, Options>::RESET);
  counter[ev::POSITIVE].setTo(0);
  counter[ev::NEGATIVE].setTo(0);
}

template <typename T, const RepresentationOptions Options>
bool EventHistogram_<T, Options>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, EventImage_<T, Options>::cols, EventImage_<T, Options>::rows))) {
    counter[e.p](e.y, e.x)++;
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP
