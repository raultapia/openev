#ifndef OPENEV_TIME_SURFACE_TPP
#define OPENEV_TIME_SURFACE_TPP

#ifndef OPENEV_REPRESENTATION_HPP
#include "openev/representation.hpp"
#endif

namespace ev {

template <typename T>
void TimeSurface_<T>::render() {
  const T ON = this->values_[TimeSurface_<T>::value::ON];
  const T OFF = this->values_[TimeSurface_<T>::value::OFF];
  const T RESET = this->values_[TimeSurface_<T>::value::RESET];

  EventImage1d aux;
  cv::normalize(timestamp, aux, 0, 1, cv::NORM_MINMAX, -1, cv::noArray());

  if constexpr(cv::DataType<T>::channels == 1) {
    EventImage_<T>(polarity.mul(aux * (ON - RESET)) + (1 - polarity).mul(aux * (OFF - RESET)) + RESET).copyTo(*this);
  } else {
    std::vector<cv::Mat_<typename T::value_type>> v(cv::DataType<T>::channels);
    cv::parallel_for_(cv::Range(0, cv::DataType<T>::channels), [&](const cv::Range &range) {
      for(int i = range.start; i < range.end; i++) {
        v[i] = polarity.mul(aux.mul(ON[i] - RESET[i])) + (1 - polarity).mul(aux.mul(OFF[i] - RESET[i])) + RESET[i];
      }
    });
    cv::merge(v, *this);
  }
}

template <typename T>
void TimeSurface_<T>::clear_() {
  this->setTo(this->values_[TimeSurface_<T>::value::RESET]);
  timestamp.setTo(0, cv::noArray());
  polarity.setTo(0, cv::noArray());
}

template <typename T>
bool TimeSurface_<T>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, this->cols, this->rows))) {
    timestamp(e.y, e.x) = e.t;
    polarity(e.y, e.x) = static_cast<double>(e.p);
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_TIME_SURFACE_TPP
