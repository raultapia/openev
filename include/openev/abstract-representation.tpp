#ifndef OPENEV_ABSTRACT_REPRESENTATION_TPP
#define OPENEV_ABSTRACT_REPRESENTATION_TPP

#ifndef OPENEV_REPRESENTATION_HPP
#include "openev/representation.hpp"
#endif

namespace ev {

template <typename T, const RepresentationOptions Options>
void AbstractRepresentation_<T, Options>::clear() {
  count_ = 0;
  tLimits_ = {DBL_MAX, DBL_MIN};
  clear_();
}

template <typename T, const RepresentationOptions Options>
bool AbstractRepresentation_<T, Options>::insert(const Event &e) {
  if constexpr(REPRESENTATION_OPTION_CHECK(Options, RepresentationOptions::ONLY_IF_POSITIVE)) {
    if(e.p == ev::NEGATIVE) {
      return false;
    }
  }

  if constexpr(REPRESENTATION_OPTION_CHECK(Options, RepresentationOptions::ONLY_IF_NEGATIVE)) {
    if(e.p == ev::POSITIVE) {
      return false;
    }
  }

  if constexpr(REPRESENTATION_OPTION_CHECK(Options, RepresentationOptions::IGNORE_POLARITY)) {
    if(insert_({e.x, e.y, e.t + timeOffset_, ev::POSITIVE})) {
      tLimits_[MIN] = std::min(tLimits_[MIN], e.t + timeOffset_);
      tLimits_[MAX] = std::max(tLimits_[MAX], e.t + timeOffset_);
      count_++;
      return true;
    }
    return false;
  } else {
    if(insert_({e.x, e.y, e.t + timeOffset_, e.p})) {
      tLimits_[MIN] = std::min(tLimits_[MIN], e.t + timeOffset_);
      tLimits_[MAX] = std::max(tLimits_[MAX], e.t + timeOffset_);
      count_++;
      return true;
    }
    return false;
  }
}

template <typename T, const RepresentationOptions Options>
template <std::size_t N>
bool AbstractRepresentation_<T, Options>::insert(const Array<N> &array) {
  return std::all_of(array.begin(), array.end(), [this](const Event &e) { return this->insert(e); });
}

template <typename T, const RepresentationOptions Options>
bool AbstractRepresentation_<T, Options>::insert(const Vector &vector) {
  return std::all_of(vector.begin(), vector.end(), [this](const Event &e) { return this->insert(e); });
}

template <typename T, const RepresentationOptions Options>
bool AbstractRepresentation_<T, Options>::insert(Queue &queue) {
  bool ret = true;
  while(!queue.empty()) {
    ret = insert(queue.front()) && ret;
    queue.pop();
  }
  return ret;
}

} // namespace ev

#endif // OPENEV_ABSTRACT_REPRESENTATION_TPP
