#ifndef OPENEV_EVENT_IMAGE_TPP
#define OPENEV_EVENT_IMAGE_TPP

#ifndef OPENEV_REPRESENTATION_HPP
#include "openev/representation.hpp"
#endif

namespace ev {

template <typename T>
void EventImage_<T>::clear_() {
  this->setTo(this->values_[EventImage_<T>::RESET]);
}

template <typename T>
bool EventImage_<T>::insert_(const Event &e) {
  if(e.inside(cv::Rect(0, 0, this->cols, this->rows))) {
    this->operator()(e.y, e.x) = e.p ? this->values_[EventImage_<T>::ON] : this->values_[EventImage_<T>::OFF];
    return true;
  }
  return false;
}

} // namespace ev

#endif // OPENEV_EVENT_IMAGE_TPP
