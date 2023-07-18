#ifndef OPENEV_EVENT_HISTOGRAM_TPP
#define OPENEV_EVENT_HISTOGRAM_TPP

#include "openev/representation.hpp"

namespace ev {

template <typename T>
void EventHistogram_<T>::render() {
  const T ON = this->values_[EventHistogram_<T>::value::ON];
  const T OFF = this->values_[EventHistogram_<T>::value::OFF];
  const T RESET = this->values_[EventHistogram_<T>::value::RESET];

  EventImage1d p_aux;
  EventImage1d n_aux;
  cv::normalize(counter_.mul(EventImage1d(counter_ > 0)), p_aux, 0, 1, cv::NORM_MINMAX, -1, cv::noArray());
  cv::normalize((-counter_).mul(EventImage1d(counter_ < 0)), n_aux, 0, 1, cv::NORM_MINMAX, -1, cv::noArray());

  if constexpr(cv::DataType<T>::channels == 1) {
    EventImage_<T>(p_aux * (ON - RESET) + n_aux * (OFF - RESET) + RESET).copyTo(*this);
  } else {
    std::vector<cv::Mat_<typename T::value_type>> v(cv::DataType<T>::channels);
    cv::parallel_for_(cv::Range(0, cv::DataType<T>::channels), [&](const cv::Range &range) {
      for(int i = range.start; i < range.end; i++) {
        v[i] = p_aux * (ON[i] - RESET[i]) + n_aux * (OFF[i] - RESET[i]) + RESET[i];
      }
    });
    cv::merge(v, *this);
  }
}

template <typename T>
void EventHistogram_<T>::clear_() {
  this->setTo(this->values_[EventHistogram_<T>::value::RESET]);
  counter_.setTo(0);
}

template <typename T>
bool EventHistogram_<T>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, EventImage_<T, Options>::cols, EventImage_<T, Options>::rows))) {
    counter[e.p](e.y, e.x)++;
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_EVENT_HISTOGRAM_TPP
