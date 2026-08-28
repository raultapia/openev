/*!
\file circular.hpp
\brief Circular buffer container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_CIRCULAR_HPP
#define OPENEV_CONTAINERS_CIRCULAR_HPP

#include "openev/containers/abstract-container.hpp"
#include "openev/core/types.hpp"
#include <boost/circular_buffer.hpp>
#include <opencv2/core/types.hpp>
#include <utility>

namespace ev {
constexpr bool USING_CIRCULAR_HPP = true;

/*!
\brief This class extends boost::circular_buffer to implement event circular buffers. For more information, please refer <a href="http://boost.org/libs/circular_buffer">here</a>.

Event circular buffers inherit all the properties from boost circular buffers. Circular buffers are fixed-size data structures in a circular fashion (i.e, the end of the buffer is reached, it wraps around to the beginning).
*/
template <typename T>
class CircularBuffer_ : public boost::circular_buffer<Event_<T>>, public AbstractContainer_<CircularBuffer_<T>, T> {
  using boost::circular_buffer<Event_<T>>::circular_buffer;
  using ResultType = TimeType;

public:
  /*!
  \brief Construct an Event_<T> in-place at the back of the buffer.

  Arguments are forwarded to the Event_<T> constructor. If the buffer is full, the
  oldest element at the front is overwritten.
  */
  template <typename... Args>
  inline void emplace_back(Args &&...args) {
    boost::circular_buffer<Event_<T>>::push_back(Event_<T>(std::forward<Args>(args)...));
  }

  /*!
  \brief Construct an Event_<T> in-place at the front of the buffer.

  Arguments are forwarded to the Event_<T> constructor. If the buffer is full, the
  oldest element at the back is overwritten.
  */
  template <typename... Args>
  inline void emplace_front(Args &&...args) {
    boost::circular_buffer<Event_<T>>::push_front(Event_<T>(std::forward<Args>(args)...));
  }
};
using CircularBufferi = CircularBuffer_<int>;    /*!< Alias for CircularBuffer_ using int */
using CircularBufferl = CircularBuffer_<long>;   /*!< Alias for CircularBuffer_ using long */
using CircularBufferf = CircularBuffer_<float>;  /*!< Alias for CircularBuffer_ using float */
using CircularBufferd = CircularBuffer_<double>; /*!< Alias for CircularBuffer_ using double */
using CircularBuffer = CircularBufferi;          /*!< Alias for CircularBuffer_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_CIRCULAR_HPP
