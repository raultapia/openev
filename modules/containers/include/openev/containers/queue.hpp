/*!
\file containers.hpp
\brief Queue container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_QUEUE_HPP
#define OPENEV_CONTAINERS_QUEUE_HPP

#include "openev/core/types.hpp"
#include <cstddef>
#include <queue>
#include <utility>

namespace ev {
/*! \cond INTERNAL */
#ifndef OPENEV_ARRAY_HPP
template <typename T, std::size_t N>
class Array_;
#endif

#ifndef OPENEV_VECTOR_HPP
template <typename T>
class Vector_;
#endif
/*! \endcond */

/*!
\brief This class extends std::queue to implement event queues. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/queue">here</a>.

Event queues inherit all the properties from standard C++ queues. Events queues are FIFO data structures not intended to be directly iterated.
*/
template <typename T>
class Queue_ : public std::queue<T> {
  using std::queue<T>::queue;

public:
  /*!
  \brief Push an event.
  \param e Event to push
  */
  inline void push(const T &e) {
    std::queue<T>::push(e);
  }

  /*!
  \brief Emplace back an event.
  \param args Constructor arguments
  */
  template <typename... Args>
  inline void emplace(Args &&...args) {
    std::queue<T>::emplace(std::forward<Args>(args)...);
  }

  /*!
  \brief Push elements from an array of events.
  \param array Event array to push
  */
  template <std::size_t N>
  inline void push(const Array_<T, N> &array) {
    for(const T &e : array) {
      std::queue<T>::emplace(std::move(e));
    }
  }

  /*!
  \brief Push elements from a vector of events.
  \param vector Event vector to push
  */
  inline void push(const Vector_<T> &vector) {
    for(const T &e : vector) {
      std::queue<T>::emplace(std::move(e));
    }
  }

  /*!
  \brief Time difference between the last and the first event in the queue.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (std::queue<T>::back()).t - (std::queue<T>::front()).t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event in the queue.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::queue<T>::size() / duration();
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (std::queue<T>::front().t + std::queue<T>::back().t);
  }
};
using Queuei = Queue_<Eventi>;                   /*!< Alias for Queue_ using Eventi */
using Queuel = Queue_<Eventl>;                   /*!< Alias for Queue_ using Eventl */
using Queuef = Queue_<Eventf>;                   /*!< Alias for Queue_ using Eventf */
using Queued = Queue_<Eventd>;                   /*!< Alias for Queue_ using Eventd */
using Queue = Queue_<Event>;                     /*!< Alias for Queue_ using Event */
using AugmentedQueuei = Queue_<AugmentedEventi>; /*!< Alias for augmented Queue_ using AugmentedEventi */
using AugmentedQueuel = Queue_<AugmentedEventl>; /*!< Alias for augmented Queue_ using AugmentedEventl */
using AugmentedQueuef = Queue_<AugmentedEventf>; /*!< Alias for augmented Queue_ using AugmentedEventf */
using AugmentedQueued = Queue_<AugmentedEventd>; /*!< Alias for augmented Queue_ using AugmentedEventd */
using AugmentedQueue = Queue_<AugmentedEvent>;   /*!< Alias for augmented Queue_ using AugmentedEvent */
} // namespace ev

#endif // OPENEV_CONTAINERS_QUEUE_HPP
