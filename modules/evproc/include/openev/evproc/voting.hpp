/*!
\file cmax.hpp
\brief
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_VOTING_HPP
#define OPENEV_EVPROC_VOTING_HPP

#include "openev/core/types.hpp"

namespace ev {
inline std::array<double, 4> bilinearVoting(Event event);
inline std::array<AugmentedEvent, 4> bilinearVoting(AugmentedEvent event);
} // namespace ev

#endif // OPENEV_EVPROC_VOTING_HPP
