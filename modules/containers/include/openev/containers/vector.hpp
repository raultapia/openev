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
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return std::vector<T>::back().t - std::vector<T>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::vector<T>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const {
    const double x = std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / std::vector<T>::size();
    const double y = std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / std::vector<T>::size();
    const double t = std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / std::vector<T>::size();
    const double p = std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.p; }) / std::vector<T>::size();
    return {x, y, t, p > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() const {
    const double x = std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / std::vector<T>::size();
    const double y = std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / std::vector<T>::size();
    return {x, y};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const {
    return std::accumulate(std::vector<T>::begin(), std::vector<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / std::vector<T>::size();
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (std::vector<T>::front().t + std::vector<T>::back().t);
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
