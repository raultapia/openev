/*!
\file concurrent_queue.hpp
\brief Queue container for basic event structures, safe between a thread that pushes and a thread that pops.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_CONCURRENT_QUEUE_HPP
#define OPENEV_CONTAINERS_CONCURRENT_QUEUE_HPP

#include "openev/core/types.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <cstddef>

namespace ev {
[[maybe_unused]] constexpr bool USING_CONCURRENT_QUEUE_HPP = true;

/*!
\brief This class extends boost::lockfree::spsc_queue to implement event queues that one thread fills while another one empties, with no locks. For more information, please refer <a href="https://www.boost.org/doc/libs/release/doc/html/boost/lockfree/spsc_queue.html">here</a>.

Concurrent event queues inherit all the properties from boost single-producer single-consumer queues, so they are FIFO
structures of fixed capacity that cannot be iterated. push() and full() belong to the thread that fills the queue, and front(),
pop() and clear() to the thread that empties it. empty(), size() and capacity() can be used by both.

\note Unlike event circular buffers, a full queue does not overwrite its oldest event: push() returns false and the event is not
added, so no event is ever lost between the two threads.

\note At most one thread may push and at most one thread may pop.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using ConcurrentQueuei = ConcurrentQueue_<int>;
using ConcurrentQueuel = ConcurrentQueue_<long>;
using ConcurrentQueuef = ConcurrentQueue_<float>;
using ConcurrentQueued = ConcurrentQueue_<double>;
using ConcurrentQueue  = ConcurrentQueuei;
\endcode
*/
template <typename T>
class ConcurrentQueue_ : public boost::lockfree::spsc_queue<Event_<T>> {
public:
  /*!
  \brief Construct the queue.
  \param capacity Number of events the queue can hold
  */
  explicit ConcurrentQueue_(const std::size_t capacity) : boost::lockfree::spsc_queue<Event_<T>>{capacity}, capacity_{capacity} {}

  /*!
  \brief Remove every event.
  */
  inline void clear() {
    while(boost::lockfree::spsc_queue<Event_<T>>::pop());
  }

  /*!
  \brief Check if the queue holds no events.
  \return True if empty
  */
  [[nodiscard]] inline bool empty() const {
    return boost::lockfree::spsc_queue<Event_<T>>::read_available() == 0;
  }

  /*!
  \brief Check if the queue holds as many events as it can.
  \return True if full
  */
  [[nodiscard]] inline bool full() const {
    return boost::lockfree::spsc_queue<Event_<T>>::write_available() == 0;
  }

  /*!
  \brief Number of events in the queue.
  \return Number of events
  */
  [[nodiscard]] inline std::size_t size() const {
    return boost::lockfree::spsc_queue<Event_<T>>::read_available();
  }

  /*!
  \brief Number of events the queue can hold.
  \return Capacity
  */
  [[nodiscard]] inline std::size_t capacity() const {
    return capacity_;
  }

private:
  std::size_t capacity_;
};

using ConcurrentQueuei = ConcurrentQueue_<int>;    /*!< Alias for ConcurrentQueue_ using int */
using ConcurrentQueuel = ConcurrentQueue_<long>;   /*!< Alias for ConcurrentQueue_ using long */
using ConcurrentQueuef = ConcurrentQueue_<float>;  /*!< Alias for ConcurrentQueue_ using float */
using ConcurrentQueued = ConcurrentQueue_<double>; /*!< Alias for ConcurrentQueue_ using double */
using ConcurrentQueue = ConcurrentQueuei;          /*!< Alias for ConcurrentQueue_ using Event */
} // namespace ev

#endif // OPENEV_CONTAINERS_CONCURRENT_QUEUE_HPP
