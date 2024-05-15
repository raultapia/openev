/*!
\file containers.hpp
\brief Vector container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_VECTOR_HPP
#define OPENEV_CONTAINERS_VECTOR_HPP

#include "openev/core/types.hpp"
#include <cstddef>
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
  inline void push_back(Queue_<T> &queue) {
    std::vector<T>::reserve(std::vector<T>::size() + queue.size());
    while(!queue.empty()) {
      std::vector<T>::emplace_back(std::move(queue.front()));
      queue.pop();
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
