/*!
\file containers.hpp
\brief Array container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_ARRAY_HPP
#define OPENEV_CONTAINERS_ARRAY_HPP

#include "openev/core/types.hpp"
#include <array>
#include <cstddef>
#include <numeric>

namespace ev {
/*!
\brief This class extends std::array to implement event arrays. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/array">here</a>.

Event arrays inherit all the properties from standard C++ arrays. Events in the array are stored contiguously.
*/
template <typename T, std::size_t N>
class Array_ : public std::array<Event_<T>, N> {
  using std::array<Event_<T>, N>::array;

public:
  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return std::array<Event_<T>, N>::back().t - std::array<Event_<T>, N>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::array<Event_<T>, N>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const {
    const double x = std::accumulate(std::array<Event_<T>, N>::begin(), std::array<Event_<T>, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / N;
    const double y = std::accumulate(std::array<Event_<T>, N>::begin(), std::array<Event_<T>, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / N;
    const double t = std::accumulate(std::array<Event_<T>, N>::begin(), std::array<Event_<T>, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / N;
    const double p = std::accumulate(std::array<Event_<T>, N>::begin(), std::array<Event_<T>, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.p; }) / N;
    return {x, y, t, p > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() const {
    const double x = std::accumulate(std::array<Event_<T>, N>::begin(), std::array<Event_<T>, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / N;
    const double y = std::accumulate(std::array<Event_<T>, N>::begin(), std::array<Event_<T>, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / N;
    return {x, y};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const {
    return std::accumulate(std::array<Event_<T>, N>::begin(), std::array<Event_<T>, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / N;
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (std::array<Event_<T>, N>::front().t + std::array<Event_<T>, N>::back().t);
  }
};
template <std::size_t N>
using Arrayi = Array_<int, N>; /*!< Alias for Array_ using int */
template <std::size_t N>
using Arrayl = Array_<long, N>; /*!< Alias for Array_ using long */
template <std::size_t N>
using Arrayf = Array_<float, N>; /*!< Alias for Array_ using float */
template <std::size_t N>
using Arrayd = Array_<double, N>; /*!< Alias for Array_ using double */
template <std::size_t N>
using Array = Arrayi<N>; /*!< Alias for Array_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_ARRAY_HPP
