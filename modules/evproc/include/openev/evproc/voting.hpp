/*!
\file voting.hpp
\brief Bilinear voting utilities for event-based vision.
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_VOTING_HPP
#define OPENEV_EVPROC_VOTING_HPP

#include "openev/core/types.hpp"
#include <array>

namespace ev {

template <typename T>
std::array<T, 4> bilinearVoting(Event_<T> event);

template <typename T>
std::array<AugmentedEvent_<T>, 4> bilinearVoting(AugmentedEvent_<T> event);

} // namespace ev

#endif // OPENEV_EVPROC_VOTING_HPP
