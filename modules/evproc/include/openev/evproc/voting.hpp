/*!
\file cmax.hpp
\brief
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_VOTING_HPP
#define OPENEV_EVPROC_VOTING_HPP

#include "openev/core/types.hpp"
#include <array>

namespace ev {

template <typename T>
static inline std::array<T, 4> bilinearVoting(Event_<T> event);

template <typename T>
static inline std::array<AugmentedEvent_<T>, 4> bilinearVoting(AugmentedEvent_<T> event);
} // namespace ev

#endif // OPENEV_EVPROC_VOTING_HPP
