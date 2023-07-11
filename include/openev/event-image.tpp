#ifndef OPENEV_EVENT_IMAGE_TPP
#define OPENEV_EVENT_IMAGE_TPP

#ifndef OPENEV_REPRESENTATION_HPP
#include "openev/representation.hpp"
#endif

namespace ev {

template <typename T>
void EventImage_<T>::clear_() {
  this->setTo(imageValues_[value::RESET]);
}

template <typename T>
void EventImage_<T>::insert_(const Event &e) {
  this->operator()(e.y, e.x) = e.p ? imageValues_[value::ON] : imageValues_[value::OFF];
}

} // namespace ev

#endif // OPENEV_EVENT_IMAGE_TPP
