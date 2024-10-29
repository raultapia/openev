/*!
\file circular.hpp
\brief Circular buffer container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_CIRCULAR_HPP
#define OPENEV_CONTAINERS_CIRCULAR_HPP

#include "openev/core/types.hpp"
#include <boost/circular_buffer.hpp>
#include <numeric>

namespace ev {
/*!
\brief This class extends boost::circular_buffer to implement event circular buffers. For more information, please refer <a href="http://boost.org/libs/circular_buffer">here</a>.

Event circular buffers inherit all the properties from boost circular buffers. Circular buffers are fixed-size data structures in a circular fashion (i.e, the end of the buffer is reached, it wraps around to the beginning).
*/
template <typename T>
class CircularBuffer_ : public boost::circular_buffer<Event_<T>> {
  using boost::circular_buffer<Event_<T>>::circular_buffer;

public:
  /*! \cond INTERNAL */
  template <typename... Args>
  inline void emplace_back(Args &&...args) {
    boost::circular_buffer<Event_<T>>::push_back(Event_<T>(std::forward<Args>(args)...));
  }

  template <typename... Args>
  inline void emplace_front(Args &&...args) {
    boost::circular_buffer<Event_<T>>::push_front(Event_<T>(std::forward<Args>(args)...));
  }
  /*! \endcond */

  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return boost::circular_buffer<Event_<T>>::back().t - boost::circular_buffer<Event_<T>>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return boost::circular_buffer<Event_<T>>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const {
    const double x = std::accumulate(boost::circular_buffer<Event_<T>>::begin(), boost::circular_buffer<Event_<T>>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / boost::circular_buffer<Event_<T>>::size();
    const double y = std::accumulate(boost::circular_buffer<Event_<T>>::begin(), boost::circular_buffer<Event_<T>>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / boost::circular_buffer<Event_<T>>::size();
    const double t = std::accumulate(boost::circular_buffer<Event_<T>>::begin(), boost::circular_buffer<Event_<T>>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / boost::circular_buffer<Event_<T>>::size();
    const double p = std::accumulate(boost::circular_buffer<Event_<T>>::begin(), boost::circular_buffer<Event_<T>>::end(), 0.0, [](double sum, const T &e) { return sum + e.p; }) / boost::circular_buffer<Event_<T>>::size();
    return {x, y, t, p > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() const {
    const double x = std::accumulate(boost::circular_buffer<Event_<T>>::begin(), boost::circular_buffer<Event_<T>>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / boost::circular_buffer<Event_<T>>::size();
    const double y = std::accumulate(boost::circular_buffer<Event_<T>>::begin(), boost::circular_buffer<Event_<T>>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / boost::circular_buffer<Event_<T>>::size();
    return {x, y};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const {
    return std::accumulate(boost::circular_buffer<Event_<T>>::begin(), boost::circular_buffer<Event_<T>>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / boost::circular_buffer<Event_<T>>::size();
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (boost::circular_buffer<Event_<T>>::front().t + boost::circular_buffer<Event_<T>>::back().t);
  }
};
using CircularBufferi = CircularBuffer_<int>;    /*!< Alias for CircularBuffer_ using int */
using CircularBufferl = CircularBuffer_<long>;   /*!< Alias for CircularBuffer_ using long */
using CircularBufferf = CircularBuffer_<float>;  /*!< Alias for CircularBuffer_ using float */
using CircularBufferd = CircularBuffer_<double>; /*!< Alias for CircularBuffer_ using double */
using CircularBuffer = CircularBufferi;          /*!< Alias for CircularBuffer_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_CIRCULAR_HPP
