#ifndef OPENEV_ABSTRACT_REPRESENTATION_TPP
#define OPENEV_ABSTRACT_REPRESENTATION_TPP

#include "openev/representation.hpp"

namespace ev {

template <typename T>
void AbstractRepresentation_<T>::clear() {
  count_ = 0;
  tLimits_ = {DBL_MAX, DBL_MIN};
  clear_();
}

template <typename T>
bool AbstractRepresentation_<T>::insert(const Event &e) {
  if(!e.inside(cv::Rect(0, 0, this->cols, this->rows))) {
    return false;
  }
  tLimits_[MIN] = std::min(tLimits_[MIN], e.t);
  tLimits_[MAX] = std::max(tLimits_[MAX], e.t);
  count_++;
  insert_(e);
  return true;
}

template <typename T>
bool AbstractRepresentation_<T>::insert(const EventPacket &packet) {
  return std::all_of(packet.begin(), packet.end(), [this](const Event &e) { return this->insert(e); });
}

template <typename T>
bool AbstractRepresentation_<T>::insert(EventBuffer &buffer) {
  bool ret = true;
  while(!buffer.empty()) {
    ret = ret && insert(buffer.front());
    buffer.pop();
  }
  return ret;
}

} // namespace ev

#endif // OPENEV_ABSTRACT_REPRESENTATION_TPP
