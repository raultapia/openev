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
class Array_ : public std::array<T, N> {
  using std::array<T, N>::array;

public:
  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return std::array<T, N>::back().t - std::array<T, N>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::array<T, N>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const {
    const double x = std::accumulate(std::array<T, N>::begin(), std::array<T, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / N;
    const double y = std::accumulate(std::array<T, N>::begin(), std::array<T, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / N;
    const double t = std::accumulate(std::array<T, N>::begin(), std::array<T, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / N;
    const double p = std::accumulate(std::array<T, N>::begin(), std::array<T, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.p; }) / N;
    return {x, y, t, p > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() const {
    const double x = std::accumulate(std::array<T, N>::begin(), std::array<T, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / N;
    const double y = std::accumulate(std::array<T, N>::begin(), std::array<T, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / N;
    return {x, y};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const {
    return std::accumulate(std::array<T, N>::begin(), std::array<T, N>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / N;
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (std::array<T, N>::front().t + std::array<T, N>::back().t);
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
} // namespace ev

#endif // OPENEV_CONTAINERS_ARRAY_HPP
