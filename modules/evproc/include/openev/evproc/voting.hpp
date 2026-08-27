/*!
\file voting.hpp
\brief Bilinear voting utilities for event-based vision.
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_VOTING_HPP
#define OPENEV_EVPROC_VOTING_HPP

#include "openev/core/types.hpp"
#include <array>
#include <type_traits>

namespace ev {

/*! \cond INTERNAL */
namespace detail {
template <typename T>
std::array<T, 4> bilinearVoting(Event_<T> event);

template <typename T>
std::array<AugmentedEvent_<T>, 4> bilinearVoting(AugmentedEvent_<T> event);
} // namespace detail
/*! \endcond */

template <typename T>
inline std::array<T, 4> bilinearVoting(Event_<T> event) {
  static_assert(std::is_floating_point_v<T>, "ev::bilinearVoting: the coordinate type must be floating point, otherwise every weight collapses to zero or one.");
  return detail::bilinearVoting(event);
}

template <typename T>
inline std::array<AugmentedEvent_<T>, 4> bilinearVoting(AugmentedEvent_<T> event) {
  static_assert(std::is_floating_point_v<T>, "ev::bilinearVoting: the coordinate type must be floating point, otherwise every weight collapses to zero or one.");
  return detail::bilinearVoting(event);
}

} // namespace ev

#endif // OPENEV_EVPROC_VOTING_HPP
