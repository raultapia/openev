/*!
\file representations.hpp
\brief Include all the OpenEV representations module.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_HPP
#define OPENEV_REPRESENTATIONS_HPP

#include "openev/representations/event-histogram.hpp"
#include "openev/representations/event-image.hpp"
#include "openev/representations/point-cloud.hpp"
#include "openev/representations/time-surface.hpp"

namespace {
inline void workaroundRepresentations() {
  (void)ev::USING_EVENT_HISTOGRAM_HPP;
  (void)ev::USING_EVENT_IMAGE_HPP;
  (void)ev::USING_POINT_CLOUD_HPP;
  (void)ev::USING_TIME_SURFACE_HPP;
}
} // namespace

#endif // OPENEV_REPRESENTATIONS_HPP
