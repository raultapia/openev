/*!
\file voting.cpp
\brief Implementation of voting.
\author Raul Tapia
*/
#include "openev/core/types.hpp"
#include "openev/evproc/voting.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/saturate.hpp>
#include <opencv2/core/utility.hpp>

inline std::array<double, 4> ev::bilinearVoting(ev::Event event) {
  const double dx = event.x - static_cast<int>(event.x);
  const double dy = event.y - static_cast<int>(event.y);
  const double one_minus_dx = 1 - dx;
  const double one_minus_dy = 1 - dy;
  return {one_minus_dx * one_minus_dy, dx * one_minus_dy, one_minus_dx * dy, dx * dy};
}

inline std::array<ev::AugmentedEvent, 4> ev::bilinearVoting(ev::AugmentedEvent event) {
  const int int_x = static_cast<int>(event.x);
  const int int_y = static_cast<int>(event.y);
  const double dx = event.x - int_x;
  const double dy = event.y - int_y;
  const double one_minus_dx = 1 - dx;
  const double one_minus_dy = 1 - dy;
  ev::AugmentedEvent e1 = event;
  ev::AugmentedEvent e2 = event;
  ev::AugmentedEvent e3 = event;
  ev::AugmentedEvent e4 = event;
  e1.x = e3.x = int_x;
  e2.x = e4.x = int_x + 1;
  e1.y = e2.y = int_y;
  e3.y = e4.y = int_y + 1;
  e1.weight = one_minus_dx * one_minus_dy;
  e2.weight = dx * one_minus_dy;
  e3.weight = one_minus_dx * dy;
  e4.weight = dx * dy;
  return {e1, e2, e3, e4};
}
