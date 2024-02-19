/*!
\file containers.hpp
\brief Containers for basic event-based vision structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_HPP
#define OPENEV_CONTAINERS_HPP

#include "openev/types.hpp"
#include <array>
#include <cstddef>
#include <queue>
#include <set>
#include <utility>
#include <vector>

namespace ev {
/*! \cond INTERNAL */
template <typename T, std::size_t N>
class Array_;

template <typename T>
class Vector_;

template <typename T>
class Queue_;
/*! \endcond */

/*!
\brief This class extends std::array to implement event arrays. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/array">here</a>.

Event arrays inherit all the properties from standard C++ arrays. Events in the array are stored contiguously.
*/
template <typename T, std::size_t N>
class Array_ : public std::array<T, N> {
  using std::array<T, N>::array;

public:
  /*!
  \brief Time difference between the last and the first event in the array.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (std::array<T, N>::back()).t - (std::array<T, N>::front()).t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event in the array.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::array<T, N>::size() / duration();
  }
};
template <std::size_t N>
using Arrayi = Array_<Eventi, N>; /*!< Alias for Array_ using Eventi */
template <std::size_t N>
using Arrayl = Array_<Eventl, N>; /*!< Alias for Array_ using Eventl */
template <std::size_t N>
using Arrayf = Array_<Eventf, N>; /*!< Alias for Array_ using Eventf */
template <std::size_t N>
using Arrayd = Array_<Eventd, N>; /*!< Alias for Array_ using Eventd */
template <std::size_t N>
using Array = Array_<Event, N>; /*!< Alias for Array_ using Event */
template <std::size_t N>
using AugmentedArrayi = Array_<AugmentedEventi, N>; /*!< Alias for augmented Array_ using AugmentedEventi */
template <std::size_t N>
using AugmentedArrayl = Array_<AugmentedEventl, N>; /*!< Alias for augmented Array_ using AugmentedEventl */
template <std::size_t N>
using AugmentedArrayf = Array_<AugmentedEventf, N>; /*!< Alias for augmented Array_ using AugmentedEventf */
template <std::size_t N>
using AugmentedArrayd = Array_<AugmentedEventd, N>; /*!< Alias for augmented Array_ using AugmentedEventd */
template <std::size_t N>
using AugmentedArray = Array_<AugmentedEvent, N>; /*!< Alias for augmented Array_ using AugmentedEvent */

/*!
\brief This class extends std::vector to implement event vectors. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/vector">here</a>.

Event vectors inherit all the properties from standard C++ vectors. Events in the vector are stored contiguously.
*/
template <typename T>
class Vector_ : public std::vector<T> {
  using std::vector<T>::vector;

public:
  /*!
  \brief Push back an event.
  \param e Event to push back
  */
  inline void push_back(const T &e) {
    std::vector<T>::push_back(e);
  }

  /*!
  \brief Emplace back an event.
  \param args Constructor arguments
  */
  template <typename... Args>
  inline void emplace_back(Args &&...args) {
    std::vector<T>::emplace_back(std::forward<Args>(args)...);
  }

  /*!
  \brief Push back elements from an array of events.
  \param array Event array to push back
  */
  template <std::size_t N>
  inline void push_back(const Array_<T, N> &array) {
    std::vector<T>::reserve(std::vector<T>::size() + array.size());
    std::vector<T>::insert(std::vector<T>::end(), array.begin(), array.end());
  }

  /*!
  \brief Push back elements from a queue of events.
  \param queue Event queue to push back
  */
  inline void push_back(Queue_<T> &queue);

  /*!
  \brief Time difference between the last and the first event in the vector.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (std::vector<T>::back()).t - (std::vector<T>::front()).t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event in the vector.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::vector<T>::size() / duration();
  }
};
using Vectori = Vector_<Eventi>;                   /*!< Alias for Vector_ using Eventi */
using Vectorl = Vector_<Eventl>;                   /*!< Alias for Vector_ using Eventl */
using Vectorf = Vector_<Eventf>;                   /*!< Alias for Vector_ using Eventf */
using Vectord = Vector_<Eventd>;                   /*!< Alias for Vector_ using Eventd */
using Vector = Vector_<Event>;                     /*!< Alias for Vector_ using Event */
using AugmentedVectori = Vector_<AugmentedEventi>; /*!< Alias for augmented Vector_ using AugmentedEventi */
using AugmentedVectorl = Vector_<AugmentedEventl>; /*!< Alias for augmented Vector_ using AugmentedEventl */
using AugmentedVectorf = Vector_<AugmentedEventf>; /*!< Alias for augmented Vector_ using AugmentedEventf */
using AugmentedVectord = Vector_<AugmentedEventd>; /*!< Alias for augmented Vector_ using AugmentedEventd */
using AugmentedVector = Vector_<AugmentedEvent>;   /*!< Alias for augmented Vector_ using AugmentedEvent */

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
  inline void push(const Vector_<T> &vector);

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

template <typename T>
inline void Vector_<T>::push_back(Queue_<T> &queue) {
  std::vector<T>::reserve(std::vector<T>::size() + queue.size());
  while(!queue.empty()) {
    std::vector<T>::emplace_back(std::move(queue.front()));
    queue.pop();
  }
}

template <typename T>
inline void Queue_<T>::push(const Vector_<T> &vector) {
  for(const T &e : vector) {
    std::queue<T>::emplace(std::move(e));
  }
}

} // namespace ev

#endif // OPENEV_CONTAINERS_HPP
