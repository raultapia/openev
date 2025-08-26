/*!
\file containers.hpp
\brief Vector container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_VECTOR_HPP
#define OPENEV_CONTAINERS_VECTOR_HPP

#include "openev/core/types.hpp"
#include <opencv2/core/types.hpp>
#include <vector>

namespace ev {
/*!
\brief This class extends std::vector to implement event vectors. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/vector">here</a>.

Event vectors inherit all the properties from standard C++ vectors. Events in the vector are stored contiguously.
*/
template <typename T>
class Vector_ : public std::vector<Event_<T>> {
  using std::vector<Event_<T>>::vector;

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
using Vectori = Vector_<int>;    /*!< Alias for Vector_ using int */
using Vectorl = Vector_<long>;   /*!< Alias for Vector_ using long */
using Vectorf = Vector_<float>;  /*!< Alias for Vector_ using float */
using Vectord = Vector_<double>; /*!< Alias for Vector_ using double */
using Vector = Vectori;          /*!< Alias for Vector_ using Event */
} // namespace ev

#endif // OPENEV_CONTAINERS_VECTOR_HPP
