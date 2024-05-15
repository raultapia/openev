#ifndef OPENEV_REPRESENTATIONS_TIME_SURFACE_TPP
#define OPENEV_REPRESENTATIONS_TIME_SURFACE_TPP

#ifndef OPENEV_REPRESENTATIONS_TIME_SURFACE_HPP
#include "openev/representations/time-surface.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options>
cv::Mat &TimeSurface_<T, Options>::render(const Kernel kernel /*= Kernel::NONE*/, const double tau /*= 0*/) {
  logger::error("TimeSurface::applyKernel: tau value must be greater that zero", kernel == Kernel::NONE || tau > 0);
  if(TimeSurface_<T, Options>::tLimits_[TimeSurface_<T, Options>::MAX] < 0) {
    return *this;
  }

  cv::Mat_<double> ts;
  switch(kernel) {
  case Kernel::NONE:
    cv::normalize(time, ts, 0, 1, cv::NORM_MINMAX, -1, time > 0);
    break;
  case Kernel::LINEAR:
    ts = cv::Mat_<double>(1.0 + (time - TimeSurface_<T, Options>::tLimits_[TimeSurface_<T, Options>::MAX]) / tau);
    ts.setTo(0, ts < 0);
    break;
  case Kernel::EXPONENTIAL:
    cv::exp((time - TimeSurface_<T, Options>::tLimits_[TimeSurface_<T, Options>::MAX]) / tau, ts);
    break;
  }

  if constexpr(TypeHelper<T>::NumChannels == 1) {
    cv::Mat_<T>(ts * (TimeSurface_<T, Options>::ON - TimeSurface_<T, Options>::RESET) + TimeSurface_<T, Options>::RESET).copyTo(*this, polarity == 1);
    cv::Mat_<T>(ts * (TimeSurface_<T, Options>::OFF - TimeSurface_<T, Options>::RESET) + TimeSurface_<T, Options>::RESET).copyTo(*this, polarity == 0);
  } else {
    std::vector<typename TypeHelper<T>::ChannelType> v(TypeHelper<T>::NumChannels);
    cv::parallel_for_(cv::Range(0, TypeHelper<T>::NumChannels), [&](const cv::Range &range) {
      const int start = range.start;
      const int end = range.end;
      for(int i = start; i < end; i++) {
        typename TypeHelper<T>::ChannelType(ts * (TimeSurface_<T, Options>::ON[i] - TimeSurface_<T, Options>::RESET[i]) + TimeSurface_<T, Options>::RESET[i]).copyTo(v[i], polarity == 1);
        typename TypeHelper<T>::ChannelType(ts * (TimeSurface_<T, Options>::OFF[i] - TimeSurface_<T, Options>::RESET[i]) + TimeSurface_<T, Options>::RESET[i]).copyTo(v[i], polarity == 0);
      }
    });
    cv::merge(v, *this);
  }

  return *this;
}

template <typename T, const RepresentationOptions Options>
void TimeSurface_<T, Options>::clear_() {
  this->setTo(TimeSurface_<T, Options>::RESET);
  time.setTo(0);
  polarity.setTo(0);
}

template <typename T, const RepresentationOptions Options>
bool TimeSurface_<T, Options>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, this->cols, this->rows))) {
    time(e.y, e.x) = e.t;
    polarity(e.y, e.x) = static_cast<unsigned char>(e.p);
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_TIME_SURFACE_TPP
