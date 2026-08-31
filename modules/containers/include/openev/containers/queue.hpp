/*!
\file queue.hpp
\brief Queue container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_QUEUE_HPP
#define OPENEV_CONTAINERS_QUEUE_HPP

#include "openev/containers/abstract-container.hpp"
#include "openev/core/types.hpp"
#include <queue>

namespace ev {
constexpr bool USING_QUEUE_HPP = true;

/*!
\brief This class extends std::queue to implement event queues. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/queue">here</a>.

Event queues inherit all the properties from standard C++ queues, so they remain FIFO structures that cannot be iterated.
Internally they do expose iterators to AbstractContainer_, which is how the statistics are computed without consuming the events.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Queuei = Queue_<int>;
using Queuel = Queue_<long>;
using Queuef = Queue_<float>;
using Queued = Queue_<double>;
using Queue  = Queuei;
\endcode
*/
template <typename T>
class Queue_ : public std::queue<Event_<T>>, public AbstractContainer_<Queue_<T>, T> {
  using std::queue<Event_<T>>::queue;

  friend class AbstractContainer_<Queue_<T>, T>;

protected:
  /*! \cond INTERNAL */
  [[nodiscard]] inline auto begin() {
    return std::queue<Event_<T>>::c.begin();
  }

  [[nodiscard]] inline auto end() {
    return std::queue<Event_<T>>::c.end();
  }

  [[nodiscard]] inline auto begin() const {
    return std::queue<Event_<T>>::c.begin();
  }

  [[nodiscard]] inline auto end() const {
    return std::queue<Event_<T>>::c.end();
  }
  /*! \endcond */
};

using Queuei = Queue_<int>;    /*!< Alias for Queue_ using int */
using Queuel = Queue_<long>;   /*!< Alias for Queue_ using long */
using Queuef = Queue_<float>;  /*!< Alias for Queue_ using float */
using Queued = Queue_<double>; /*!< Alias for Queue_ using double */
using Queue = Queuei;          /*!< Alias for Queue_ using Event */
} // namespace ev

#endif // OPENEV_CONTAINERS_QUEUE_HPP
