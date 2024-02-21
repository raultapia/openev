#ifndef OPENEV_EVENT_IMAGE_TPP
#define OPENEV_EVENT_IMAGE_TPP

#ifndef OPENEV_REPRESENTATION_HPP
#include "openev/representation.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options>
void EventImage_<T, Options>::clear_() {
  cv::Mat_<T>::setTo(EventImage_<T, Options>::RESET);
}

template <typename T, const RepresentationOptions Options>
bool EventImage_<T, Options>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, cv::Mat_<T>::cols, cv::Mat_<T>::rows))) {
    cv::Mat_<T>::operator()(e.y, e.x) = e.p ? EventImage_<T, Options>::ON : EventImage_<T, Options>::OFF;
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_EVENT_IMAGE_TPP
