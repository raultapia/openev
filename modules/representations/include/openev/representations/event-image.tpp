#ifndef OPENEV_REPRESENTATIONS_EVENT_IMAGES_TPP
#define OPENEV_REPRESENTATIONS_EVENT_IMAGES_TPP

#ifndef OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP
#include "openev/representations/event-image.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options>
void EventImage_<T, Options>::clear_() {
  cv::Mat_<T>::setTo(EventImage_<T, Options>::V_RESET);
}

template <typename T, const RepresentationOptions Options>
void EventImage_<T, Options>::clear_(const cv::Mat &background) {
  background.copyTo(*this);
}

template <typename T, const RepresentationOptions Options>
bool EventImage_<T, Options>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, cv::Mat_<T>::cols, cv::Mat_<T>::rows))) {
    cv::Mat_<T>::operator()(e.y, e.x) = e.p ? EventImage_<T, Options>::V_ON : EventImage_<T, Options>::V_OFF;
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_EVENT_IMAGES_TPP
