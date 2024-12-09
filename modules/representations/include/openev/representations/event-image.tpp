#ifndef OPENEV_REPRESENTATIONS_EVENT_IMAGES_TPP
#define OPENEV_REPRESENTATIONS_EVENT_IMAGES_TPP

#ifndef OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP
#include "openev/representations/event-image.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options, typename E>
void EventImage_<T, Options, E>::clear_() {
  cv::Mat_<T>::setTo(EventImage_<T, Options, E>::V_RESET);
}

template <typename T, const RepresentationOptions Options, typename E>
void EventImage_<T, Options, E>::clear_(const cv::Mat &background) {
  background.copyTo(*this);
}

template <typename T, const RepresentationOptions Options, typename E>
bool EventImage_<T, Options, E>::insert_(const Event_<E> &e) {
  if(e.inside(cv::Rect(0, 0, cv::Mat_<T>::cols, cv::Mat_<T>::rows))) {
    cv::Mat_<T>::operator()(e.y, e.x) = e.p ? EventImage_<T, Options, E>::V_ON : EventImage_<T, Options, E>::V_OFF;
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_REPRESENTATIONS_EVENT_IMAGES_TPP
