#ifndef OPENEV_ABSTRACT_REPRESENTATION_TPP
#define OPENEV_ABSTRACT_REPRESENTATION_TPP

#ifndef OPENEV_REPRESENTATION_HPP
#include "openev/representation.hpp"
#endif

namespace ev {

template <typename T>
void AbstractRepresentation_<T>::clear() {
  count_ = 0;
  tLimits_ = {DBL_MAX, DBL_MIN};
  clear_();
}

template <typename T>
bool AbstractRepresentation_<T>::insert(const Event &e) {
  if(insert_({e.x, e.y, e.t + timeOffset_, e.p})) {
    tLimits_[MIN] = std::min(tLimits_[MIN], e.t + timeOffset_);
    tLimits_[MAX] = std::max(tLimits_[MAX], e.t + timeOffset_);
    count_++;
    return true;
  }
  return false;
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
