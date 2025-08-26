/*!
\file deque.hpp
\brief Deque container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_DEQUE_HPP
#define OPENEV_CONTAINERS_DEQUE_HPP

#include "openev/core/types.hpp"
#include <deque>
#include <opencv2/core/types.hpp>

namespace ev {
/*!
\brief This class extends std::deque to implement event deques. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/deque">here</a>.

Event deques inherit all the properties from standard C++ deques. Events dequeu are double-ended queues that allows fast insertion and deletion at both its beginning and its end.
*/
template <typename T>
class Deque_ : public std::deque<Event_<T>> {
  using std::deque<Event_<T>>::deque;

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
using Dequei = Deque_<int>;    /*!< Alias for Deque_ using int */
using Dequel = Deque_<long>;   /*!< Alias for Deque_ using long */
using Dequef = Deque_<float>;  /*!< Alias for Deque_ using float */
using Dequed = Deque_<double>; /*!< Alias for Deque_ using double */
using Deque = Dequei;          /*!< Alias for Deque_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_DEQUE_HPP
