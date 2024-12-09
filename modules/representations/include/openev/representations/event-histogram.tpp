#ifndef OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP
#define OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP

#ifndef OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_HPP
#include "openev/representations/event-histogram.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options, typename E>
cv::Mat &EventHistogram_<T, Options, E>::render() {
  if(!peak_) {
    return *this;
  }

  cv::Mat_<double> normalized(counter);
  normalized = normalized / peak_;

  if constexpr(TypeHelper<T>::NumChannels == 1) {
    cv::Mat_<T>(
        (EventHistogram_<T, Options, E>::V_ON - EventHistogram_<T, Options, E>::V_RESET) * normalized.mul(cv::Mat_<double>(normalized > 0) / 255) +
        (EventHistogram_<T, Options, E>::V_RESET - EventHistogram_<T, Options, E>::V_OFF) * normalized.mul(cv::Mat_<double>(normalized < 0) / 255) +
        EventHistogram_<T, Options, E>::V_RESET)
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
            (EventHistogram_<T, Options, E>::V_ON[i] - EventHistogram_<T, Options, E>::V_RESET[i]) * a +
            (EventHistogram_<T, Options, E>::V_RESET[i] - EventHistogram_<T, Options, E>::V_OFF[i]) * b +
            EventHistogram_<T, Options, E>::V_RESET[i])
            .copyTo(v[i]);
      }
    });
    cv::merge(v, *this);
  }

  return *this;
}

template <typename T, const RepresentationOptions Options, typename E>
void EventHistogram_<T, Options, E>::clear_() {
  EventImage_<T, Options, E>::setTo(EventHistogram_<T, Options, E>::V_RESET);
  counter.clear();
  peak_ = 0;
}

template <typename T, const RepresentationOptions Options, typename E>
void EventHistogram_<T, Options, E>::clear_(const cv::Mat &background) {
  background.copyTo(*this);
  counter.clear();
  peak_ = 0;
}

template <typename T, const RepresentationOptions Options, typename E>
bool EventHistogram_<T, Options, E>::insert_(const Event_<E> &e) {
  if(e.inside(cv::Rect(0, 0, EventImage_<T, Options, E>::cols, EventImage_<T, Options, E>::rows))) {
    if(abs(counter.insert(e)) > peak_) {
      peak_ = abs(counter(e));
    }
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_TPP
