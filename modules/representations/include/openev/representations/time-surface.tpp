#ifndef OPENEV_REPRESENTATIONS_TIME_SURFACE_TPP
#define OPENEV_REPRESENTATIONS_TIME_SURFACE_TPP

#ifndef OPENEV_REPRESENTATIONS_TIME_SURFACE_HPP
#include "openev/representations/time-surface.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options, typename E>
cv::Mat &TimeSurface_<T, Options, E>::render(const Kernel kernel /*= Kernel::NONE*/, const double tau /*= 0*/) {
  logger::error("TimeSurface::applyKernel: tau value must be greater that zero", kernel == Kernel::NONE || tau > 0);
  if(TimeSurface_<T, Options, E>::tLimits_[TimeSurface_<T, Options, E>::MAX] < 0) {
    return *this;
  }

  cv::Mat_<double> ts;
  switch(kernel) {
  case Kernel::NONE:
    cv::normalize(time, ts, 0, 1, cv::NORM_MINMAX, -1, time > 0);
    break;
  case Kernel::LINEAR:
    ts = cv::Mat_<double>(1.0 + (time - TimeSurface_<T, Options, E>::tLimits_[TimeSurface_<T, Options, E>::MAX]) / tau);
    ts.setTo(0, ts < 0);
    break;
  case Kernel::EXPONENTIAL:
    cv::exp((time - TimeSurface_<T, Options, E>::tLimits_[TimeSurface_<T, Options, E>::MAX]) / tau, ts);
    break;
  }

  if constexpr(TypeHelper<T>::NumChannels == 1) {
    cv::Mat_<T>(ts * (TimeSurface_<T, Options, E>::V_ON - TimeSurface_<T, Options, E>::V_RESET) + TimeSurface_<T, Options, E>::V_RESET).copyTo(*this, polarity == 1);
    cv::Mat_<T>(ts * (TimeSurface_<T, Options, E>::V_OFF - TimeSurface_<T, Options, E>::V_RESET) + TimeSurface_<T, Options, E>::V_RESET).copyTo(*this, polarity == 0);
  } else {
    std::vector<typename TypeHelper<T>::ChannelType> v(TypeHelper<T>::NumChannels);
    cv::parallel_for_(cv::Range(0, TypeHelper<T>::NumChannels), [&](const cv::Range &range) {
      const int start = range.start;
      const int end = range.end;
      for(int i = start; i < end; i++) {
        typename TypeHelper<T>::ChannelType(ts * (TimeSurface_<T, Options, E>::V_ON[i] - TimeSurface_<T, Options, E>::V_RESET[i]) + TimeSurface_<T, Options, E>::V_RESET[i]).copyTo(v[i], polarity == 1);
        typename TypeHelper<T>::ChannelType(ts * (TimeSurface_<T, Options, E>::V_OFF[i] - TimeSurface_<T, Options, E>::V_RESET[i]) + TimeSurface_<T, Options, E>::V_RESET[i]).copyTo(v[i], polarity == 0);
      }
    });
    cv::merge(v, *this);
  }

  return *this;
}

template <typename T, const RepresentationOptions Options, typename E>
void TimeSurface_<T, Options, E>::clear_() {
  this->setTo(TimeSurface_<T, Options, E>::V_RESET);
  time.clear();
  polarity.clear();
}

template <typename T, const RepresentationOptions Options, typename E>
void TimeSurface_<T, Options, E>::clear_(const cv::Mat &background) {
  background.copyTo(*this);
  time.clear();
  polarity.clear();
}

template <typename T, const RepresentationOptions Options, typename E>
bool TimeSurface_<T, Options, E>::insert_(const Event_<E> &e) {
  if(e.inside(cv::Rect(0, 0, this->cols, this->rows))) {
    time.insert(e);
    polarity.insert(e);
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_TIME_SURFACE_TPP
