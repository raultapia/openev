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
class Vector_ : public std::vector<Event_<T>> {
  using std::vector<Event_<T>>::vector;

public:
  /*! \cond INTERNAL */
  inline void push_back(const Event_<T> &e) {
    std::vector<Event_<T>>::push_back(e);
  }
  /*! \endcond */

  /*!
  \brief Push back elements from an array of events.
  \param array Event array to push back
  */
  template <std::size_t N>
  inline void push_back(const Array_<T, N> &array) {
    std::vector<Event_<T>>::reserve(std::vector<Event_<T>>::size() + array.size());
    std::vector<Event_<T>>::insert(std::vector<Event_<T>>::end(), array.begin(), array.end());
  }

  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return std::vector<Event_<T>>::back().t - std::vector<Event_<T>>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::vector<Event_<T>>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const {
    const double x = std::accumulate(std::vector<Event_<T>>::begin(), std::vector<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::vector<Event_<T>>::size();
    const double y = std::accumulate(std::vector<Event_<T>>::begin(), std::vector<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::vector<Event_<T>>::size();
    const double t = std::accumulate(std::vector<Event_<T>>::begin(), std::vector<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::vector<Event_<T>>::size();
    const double p = std::accumulate(std::vector<Event_<T>>::begin(), std::vector<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.p; }) / std::vector<Event_<T>>::size();
    return {x, y, t, p > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() const {
    const double x = std::accumulate(std::vector<Event_<T>>::begin(), std::vector<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::vector<Event_<T>>::size();
    const double y = std::accumulate(std::vector<Event_<T>>::begin(), std::vector<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::vector<Event_<T>>::size();
    return {x, y};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const {
    return std::accumulate(std::vector<Event_<T>>::begin(), std::vector<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::vector<Event_<T>>::size();
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (std::vector<Event_<T>>::front().t + std::vector<Event_<T>>::back().t);
  }
};
using Vectori = Vector_<int>;    /*!< Alias for Vector_ using int */
using Vectorl = Vector_<long>;   /*!< Alias for Vector_ using long */
using Vectorf = Vector_<float>;  /*!< Alias for Vector_ using float */
using Vectord = Vector_<double>; /*!< Alias for Vector_ using double */
using Vector = Vectori;          /*!< Alias for Vector_ using Event */
} // namespace ev

#endif // OPENEV_CONTAINERS_VECTOR_HPP
