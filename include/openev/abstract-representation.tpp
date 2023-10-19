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
template <std::size_t N>
bool AbstractRepresentation_<T>::insert(const EventArray<N> &array) {
  return std::all_of(array.begin(), array.end(), [this](const Event &e) { return this->insert(e); });
}

template <typename T>
bool AbstractRepresentation_<T>::insert(const EventVector &vector) {
  return std::all_of(vector.begin(), vector.end(), [this](const Event &e) { return this->insert(e); });
}

template <typename T>
bool AbstractRepresentation_<T>::insert(EventQueue &queue) {
  bool ret = true;
  while(!queue.empty()) {
    ret = ret && insert(queue.front());
    queue.pop();
  }
  return ret;
}

} // namespace ev

#endif // OPENEV_ABSTRACT_REPRESENTATION_TPP
