/*!
\file voting.cpp
\brief Implementation of voting.
\author Raul Tapia
*/
#include "openev/evproc/voting.hpp"
#include "openev/core/types.hpp"

template <typename T>
static inline std::array<T, 4> ev::bilinearVoting(ev::Event_<T> event) {
  const T dx = event.x - static_cast<int>(event.x);
  const T dy = event.y - static_cast<int>(event.y);
  const T one_minus_dx = 1 - dx;
  const T one_minus_dy = 1 - dy;
  return {one_minus_dx * one_minus_dy, dx * one_minus_dy, one_minus_dx * dy, dx * dy};
}

template <typename T>
static inline std::array<ev::AugmentedEvent_<T>, 4> bilinearVoting(ev::AugmentedEvent_<T> event) {
  const int int_x = static_cast<int>(event.x);
  const int int_y = static_cast<int>(event.y);
  const T dx = event.x - int_x;
  const T dy = event.y - int_y;
  const T one_minus_dx = 1 - dx;
  const T one_minus_dy = 1 - dy;
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
