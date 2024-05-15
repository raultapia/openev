#ifndef OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP
#define OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP

#ifndef OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_HPP
#include "openev/representations/event-histogram.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options>
cv::Mat &EventHistogram_<T, Options>::render() {
  if(!peak_) {
    return *this;
  }

  cv::Mat_<double> normalized(counter);
  normalized = normalized / peak_;

  if constexpr(TypeHelper<T>::NumChannels == 1) {
    cv::Mat_<T>(
        (EventHistogram_<T, Options>::ON - EventHistogram_<T, Options>::RESET) * normalized.mul(cv::Mat_<double>(normalized > 0) / 255) +
        (EventHistogram_<T, Options>::RESET - EventHistogram_<T, Options>::OFF) * normalized.mul(cv::Mat_<double>(normalized < 0) / 255) +
        EventHistogram_<T, Options>::RESET)
        .copyTo(*this);
  } else {
    const cv::Mat_<double> a(normalized.mul(cv::Mat_<double>(normalized > 0) / 255));
    const cv::Mat_<double> b(normalized.mul(cv::Mat_<double>(normalized < 0) / 255));
    std::vector<typename TypeHelper<T>::ChannelType> v(TypeHelper<T>::NumChannels);
    cv::parallel_for_(cv::Range(0, TypeHelper<T>::NumChannels), [&](const cv::Range &range) {
      const int start = range.start;
      const int end = range.end;
      for(int i = start; i < end; i++) {
        typename TypeHelper<T>::ChannelType(
            (EventHistogram_<T, Options>::ON[i] - EventHistogram_<T, Options>::RESET[i]) * a +
            (EventHistogram_<T, Options>::RESET[i] - EventHistogram_<T, Options>::OFF[i]) * b +
            EventHistogram_<T, Options>::RESET[i])
            .copyTo(v[i]);
      }
    });
    cv::merge(v, *this);
  }

  return *this;
}

template <typename T, const RepresentationOptions Options>
void EventHistogram_<T, Options>::clear_() {
  EventImage_<T, Options>::setTo(EventHistogram_<T, Options>::RESET);
  counter.setTo(0);
  peak_ = 0;
}

template <typename T, const RepresentationOptions Options>
bool EventHistogram_<T, Options>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, EventImage_<T, Options>::cols, EventImage_<T, Options>::rows))) {
    if(abs(counter(e.y, e.x) += (e.p ? +1 : -1)) > peak_) {
      peak_ = abs(counter(e.y, e.x));
    }
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP
