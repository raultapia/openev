/*!
\file deque.hpp
\brief Deque container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_DEQUE_HPP
#define OPENEV_CONTAINERS_DEQUE_HPP

#include "openev/containers/array.hpp"
#include "openev/containers/vector.hpp"
#include "openev/core/types.hpp"
#include <cstddef>
#include <deque>
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
\brief This class extends std::deque to implement event deques. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/deque">here</a>.

Event deques inherit all the properties from standard C++ deques. Events dequeu are double-ended queues that allows fast insertion and deletion at both its beginning and its end.
*/
template <typename T>
class Deque_ : public std::deque<Event_<T>> {
  using std::deque<Event_<T>>::deque;

public:
  /*! \cond INTERNAL */
  inline void push_back(const Event_<T> &e) {
    std::deque<Event_<T>>::push_back(e);
  }
  /*! \endcond */

  /*!
  \brief Push elements from an array of events.
  \param array Event array to push
  */
  template <std::size_t N>
  inline void push_back(const Array_<T, N> &array) {
    std::deque<Event_<T>>::insert(std::deque<Event_<T>>::end(), array.begin(), array.end());
  }

  /*!
  \brief Push elements from a vector of events.
  \param vector Event vector to push
  */
  inline void push_back(const Vector_<T> &vector) {
    std::deque<Event_<T>>::insert(std::deque<Event_<T>>::end(), vector.begin(), vector.end());
  }

  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return std::deque<Event_<T>>::back().t - std::deque<Event_<T>>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::deque<Event_<T>>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() {
    const double x = std::accumulate(std::deque<Event_<T>>::begin(), std::deque<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::deque<Event_<T>>::size();
    const double y = std::accumulate(std::deque<Event_<T>>::begin(), std::deque<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::deque<Event_<T>>::size();
    const double t = std::accumulate(std::deque<Event_<T>>::begin(), std::deque<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::deque<Event_<T>>::size();
    const double p = std::accumulate(std::deque<Event_<T>>::begin(), std::deque<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.p; }) / std::deque<Event_<T>>::size();
    return {x, y, t, p > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() {
    const double x = std::accumulate(std::deque<Event_<T>>::begin(), std::deque<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::deque<Event_<T>>::size();
    const double y = std::accumulate(std::deque<Event_<T>>::begin(), std::deque<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::deque<Event_<T>>::size();
    return {x, y};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() {
    return std::accumulate(std::deque<Event_<T>>::begin(), std::deque<Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::deque<Event_<T>>::size();
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (std::deque<Event_<T>>::front().t + std::deque<Event_<T>>::back().t);
  }
};
using Dequei = Deque_<int>;    /*!< Alias for Deque_ using int */
using Dequel = Deque_<long>;   /*!< Alias for Deque_ using long */
using Dequef = Deque_<float>;  /*!< Alias for Deque_ using float */
using Dequed = Deque_<double>; /*!< Alias for Deque_ using double */
using Deque = Dequei;          /*!< Alias for Deque_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_DEQUE_HPP
