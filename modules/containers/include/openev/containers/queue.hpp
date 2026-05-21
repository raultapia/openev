/*!
\file queue.hpp
\brief Queue container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_QUEUE_HPP
#define OPENEV_CONTAINERS_QUEUE_HPP

#include "openev/core/types.hpp"
#include <cstddef>
#include <opencv2/core/types.hpp>
#include <queue>

namespace ev {
constexpr bool USING_QUEUE_HPP = true;

/*!
\brief This class extends std::queue to implement event queues. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/queue">here</a>.

Event queues inherit all the properties from standard C++ queues. Events queues are FIFO data structures not intended to be directly iterated.
*/
template <typename T>
class Queue_ : public std::queue<Event_<T>> {
  using std::queue<Event_<T>>::queue;
  using ResultType = TimeType;

public:
  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline ResultType duration() const {
    return std::queue<ev::Event_<T>>::back().t - std::queue<ev::Event_<T>>::front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline ResultType rate() const {
    return std::queue<ev::Event_<T>>::size() / duration();
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] inline Event_<ResultType> mean() {
    const std::size_t n = std::queue<ev::Event_<T>>::size();
    ResultType x{0};
    ResultType y{0};
    ResultType t{0};
    ResultType p{0};

    while(!std::queue<ev::Event_<T>>::empty()) {
      const Event_<T> &e = std::queue<ev::Event_<T>>::front();
      x += e.x;
      y += e.y;
      t += e.t;
      p += e.p;
      std::queue<ev::Event_<T>>::pop();
    }

    return {x / n, y / n, t / n, p / n > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point_<ResultType> meanPoint() {
    const std::size_t n = std::queue<ev::Event_<T>>::size();
    ResultType x{0};
    ResultType y{0};

    while(!std::queue<ev::Event_<T>>::empty()) {
      const Event_<T> &e = std::queue<ev::Event_<T>>::front();
      x += e.x;
      y += e.y;
      std::queue<ev::Event_<T>>::pop();
    }

    return {x / n, y / n};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline ResultType meanTime() {
    const std::size_t n = std::queue<ev::Event_<T>>::size();
    ResultType t{0};

    while(!std::queue<ev::Event_<T>>::empty()) {
      t += std::queue<ev::Event_<T>>::front().t;
      std::queue<ev::Event_<T>>::pop();
    }

    return t / n;
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time.
  */
  [[nodiscard]] inline ResultType midTime() const {
    return 0.5 * (std::queue<ev::Event_<T>>::front().t + std::queue<ev::Event_<T>>::back().t);
  }
};
using Queuei = Queue_<int>;    /*!< Alias for Queue_ using int */
using Queuel = Queue_<long>;   /*!< Alias for Queue_ using long */
using Queuef = Queue_<float>;  /*!< Alias for Queue_ using float */
using Queued = Queue_<double>; /*!< Alias for Queue_ using double */
using Queue = Queuei;          /*!< Alias for Queue_ using Event */
} // namespace ev

#endif // OPENEV_CONTAINERS_QUEUE_HPP
