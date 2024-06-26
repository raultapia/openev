/*!
\file containers.hpp
\brief Vector container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_VECTOR_HPP
#define OPENEV_CONTAINERS_VECTOR_HPP

#include "openev/containers/array.hpp"
#include "openev/containers/queue.hpp"
#include "openev/core/types.hpp"
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

namespace ev {
/*! \cond INTERNAL */
#ifndef OPENEV_ARRAY_HPP
template <typename T, std::size_t N>
class Array_;
#endif

#ifndef OPENEV_QUEUE_HPP
template <typename T>
class Queue_;
#endif
/*! \endcond */

/*!
\brief This class extends std::vector to implement event vectors. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/vector">here</a>.

Event vectors inherit all the properties from standard C++ vectors. Events in the vector are stored contiguously.
*/
template <typename T>
class Vector_ : public std::vector<T> {
  using std::vector<T>::vector;

public:
  /*! \cond INTERNAL */
  inline void push_back(const T &e) {
    std::vector<T>::push_back(e);
  }
  /*! \endcond */

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
  \param keep_events_in_queue If true, events are reinserted in the queue
  */
  inline void push_back(Queue_<T> &queue, const bool keep_events_in_queue = false) {
    std::vector<T>::reserve(std::vector<T>::size() + queue.size());
    if(keep_events_in_queue) {
      const std::size_t size = queue.size();
      for(std::size_t i = 0; i < size; i++) {
        std::vector<T>::emplace_back(std::move(queue.front()));
        queue.emplace(queue.front());
        queue.pop();
      }
    } else {
      while(!queue.empty()) {
        std::vector<T>::emplace_back(std::move(queue.front()));
        queue.pop();
      }
    }
  }

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

  /*!
  \brief Compute the mean of the events in the vector.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const {
    double x = 0.0;
    double y = 0.0;
    double t = 0.0;
    double p = 0.0;

    for(const T &e : *this) {
      x += static_cast<double>(e.x);
      y += static_cast<double>(e.y);
      t += static_cast<double>(e.t);
      p += static_cast<double>(e.p);
    }

    const std::size_t size = std::vector<T>::size();
    return {x / size, y / size, t / size, p / size >= 0.5};
  }

  /*!
  \brief Compute the mean time of the events in the vector.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const {
    return std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / std::vector<T>::size();
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
} // namespace ev

#endif // OPENEV_CONTAINERS_VECTOR_HPP
