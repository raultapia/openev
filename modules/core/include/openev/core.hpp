/*!
\file core.hpp
\brief Include all the OpenEV core module.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_HPP
#define OPENEV_CORE_HPP

#include "openev/core/frames.hpp"
#include "openev/core/imu.hpp"
#include "openev/core/matrices.hpp"
#include "openev/core/stats.hpp"
#include "openev/core/types.hpp"

namespace {
inline void workaroundCore() {
  (void)ev::USING_FRAMES_HPP;
  (void)ev::USING_IMU_HPP;
  (void)ev::USING_MATRICES_HPP;
  (void)ev::USING_STATS_HPP;
  (void)ev::USING_TYPES_HPP;
}
} // namespace

#endif // OPENEV_CORE_HPP
