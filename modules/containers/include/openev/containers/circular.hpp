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
class CircularBuffer_ : public boost::circular_buffer<T> {
  using boost::circular_buffer<T>::circular_buffer;

public:
  /*! \cond INTERNAL */
  template <typename... Args>
  inline void emplace_back(Args &&...args) {
    boost::circular_buffer<T>::push_back(T(std::forward<Args>(args)...));
  }

  template <typename... Args>
  inline void emplace_front(Args &&...args) {
    boost::circular_buffer<T>::push_front(T(std::forward<Args>(args)...));
  }
  /*! \endcond */

  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return boost::circular_buffer<T>::back().t - boost::circular_buffer<T>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return boost::circular_buffer<T>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] Eventd mean() const {
    const double x = std::accumulate(boost::circular_buffer<T>::begin(), boost::circular_buffer<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / boost::circular_buffer<T>::size();
    const double y = std::accumulate(boost::circular_buffer<T>::begin(), boost::circular_buffer<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / boost::circular_buffer<T>::size();
    const double t = std::accumulate(boost::circular_buffer<T>::begin(), boost::circular_buffer<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / boost::circular_buffer<T>::size();
    const double p = std::accumulate(boost::circular_buffer<T>::begin(), boost::circular_buffer<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.p; }) / boost::circular_buffer<T>::size();
    return {x, y, t, p > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point2d meanPoint() const {
    const double x = std::accumulate(boost::circular_buffer<T>::begin(), boost::circular_buffer<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.x; }) / boost::circular_buffer<T>::size();
    const double y = std::accumulate(boost::circular_buffer<T>::begin(), boost::circular_buffer<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.y; }) / boost::circular_buffer<T>::size();
    return {x, y};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline double meanTime() const {
    return std::accumulate(boost::circular_buffer<T>::begin(), boost::circular_buffer<T>::end(), 0.0, [](double sum, const T &e) { return sum + e.t; }) / boost::circular_buffer<T>::size();
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline double midTime() const {
    return 0.5 * (boost::circular_buffer<T>::front().t + boost::circular_buffer<T>::back().t);
  }
};
using CircularBufferi = CircularBuffer_<Eventi>;                   /*!< Alias for CircularBuffer_ using Eventi */
using CircularBufferl = CircularBuffer_<Eventl>;                   /*!< Alias for CircularBuffer_ using Eventl */
using CircularBufferf = CircularBuffer_<Eventf>;                   /*!< Alias for CircularBuffer_ using Eventf */
using CircularBufferd = CircularBuffer_<Eventd>;                   /*!< Alias for CircularBuffer_ using Eventd */
using CircularBuffer = CircularBuffer_<Event>;                     /*!< Alias for CircularBuffer_ using Event */
using AugmentedCircularBufferi = CircularBuffer_<AugmentedEventi>; /*!< Alias for augmented CircularBuffer_ using AugmentedEventi */
using AugmentedCircularBufferl = CircularBuffer_<AugmentedEventl>; /*!< Alias for augmented CircularBuffer_ using AugmentedEventl */
using AugmentedCircularBufferf = CircularBuffer_<AugmentedEventf>; /*!< Alias for augmented CircularBuffer_ using AugmentedEventf */
using AugmentedCircularBufferd = CircularBuffer_<AugmentedEventd>; /*!< Alias for augmented CircularBuffer_ using AugmentedEventd */
using AugmentedCircularBuffer = CircularBuffer_<AugmentedEvent>;   /*!< Alias for augmented CircularBuffer_ using AugmentedEvent */
} // namespace ev

#endif // OPENEV_CONTAINERS_CIRCULAR_HPP
