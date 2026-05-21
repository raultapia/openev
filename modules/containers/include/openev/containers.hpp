/*!
\file containers.hpp
\brief Include all the OpenEV containers module.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_HPP
#define OPENEV_CONTAINERS_HPP

#include "openev/containers/array.hpp"
#include "openev/containers/circular.hpp"
#include "openev/containers/deque.hpp"
#include "openev/containers/persistent_queue.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/sliding_window.hpp"
#include "openev/containers/vector.hpp"

namespace {
inline void workaround() {
  (void)ev::USING_ARRAY_HPP;
  (void)ev::USING_CIRCULAR_HPP;
  (void)ev::USING_DEQUE_HPP;
  (void)ev::USING_PERSISTENT_QUEUE_HPP;
  (void)ev::USING_QUEUE_HPP;
  (void)ev::USING_SLIDING_WINDOW_HPP;
  (void)ev::USING_VECTOR_HPP;
}
} // namespace

#endif // OPENEV_CONTAINERS_HPP
