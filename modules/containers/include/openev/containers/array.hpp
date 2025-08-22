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
#include <opencv2/core/types.hpp>

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
  [[nodiscard]] inline double duration() const;

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const;

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const;

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() const;

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const;

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const;
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
