/*!
\file evproc.hpp
\brief Include all the OpenEV evproc module.
\note This module provides functions for event processing.
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_HPP
#define OPENEV_EVPROC_HPP

#include "openev/evproc/filtering.hpp"
#include "openev/evproc/undistortion.hpp"
#include "openev/evproc/voting.hpp"

namespace {
inline void workaroundEvproc() {
  (void)ev::USING_FILTERING_HPP;
  (void)ev::USING_UNDISTORTION_HPP;
  (void)ev::USING_VOTING_HPP;
}
} // namespace

#endif // OPENEV_EVPROC_HPP
