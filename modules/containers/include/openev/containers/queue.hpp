/*!
\file queue.hpp
\brief Queue container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_QUEUE_HPP
#define OPENEV_CONTAINERS_QUEUE_HPP

#include "openev/containers/abstract-container.hpp"
#include "openev/core/types.hpp"
#include <cstddef>
#include <opencv2/core/types.hpp>
#include <queue>
#include <vector>

namespace ev {
constexpr bool USING_QUEUE_HPP = true;

/*!
\brief This class extends std::queue to implement event queues. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/queue">here</a>.

Event queues inherit all the properties from standard C++ queues. Events queues are FIFO data structures not intended to be directly iterated.
*/
template <typename T>
class Queue_ : public std::queue<Event_<T>>, public AbstractContainer_<Queue_<T>, T> {
  using std::queue<Event_<T>>::queue;
  using ResultType = TimeType;

public:
  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  \warning This method drains the queue; all events are removed upon completion. Use PersistentQueue_ to preserve contents.
  */
  [[nodiscard]] inline Event_<ResultType> mean() {
    if(std::queue<ev::Event_<T>>::empty()) {
      CV_Error(cv::Error::StsError, "ev::Queue_::mean: the container is empty.");
    }
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
  \warning This method drains the queue; all events are removed upon completion. Use PersistentQueue_ to preserve contents.
  */
  [[nodiscard]] inline cv::Point_<ResultType> meanPoint() {
    if(std::queue<ev::Event_<T>>::empty()) {
      CV_Error(cv::Error::StsError, "ev::Queue_::meanPoint: the container is empty.");
    }
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
  \warning This method drains the queue; all events are removed upon completion. Use PersistentQueue_ to preserve contents.
  */
  [[nodiscard]] inline ResultType meanTime() {
    if(std::queue<ev::Event_<T>>::empty()) {
      CV_Error(cv::Error::StsError, "ev::Queue_::meanTime: the container is empty.");
    }
    const std::size_t n = std::queue<ev::Event_<T>>::size();
    ResultType t{0};

    while(!std::queue<ev::Event_<T>>::empty()) {
      t += std::queue<ev::Event_<T>>::front().t;
      std::queue<ev::Event_<T>>::pop();
    }

    return t / n;
  }

  /*!
  \brief Compute the Shannon entropy of the spatial distribution of the events.
  \return Entropy in bits
  \note \f$ H = -\sum_i p_i \log_2 p_i \f$, where \f$ p_i \f$ is the fraction of events falling on the i-th pixel, so \f$ 2^H \f$ is the effective number of active pixels.
  \warning This method drains the queue; all events are removed upon completion. Use PersistentQueue_ to preserve contents.
  */
  [[nodiscard]] inline ResultType entropy() {
    if(std::queue<ev::Event_<T>>::empty()) {
      CV_Error(cv::Error::StsError, "ev::Queue_::entropy: the container is empty.");
    }
    std::vector<cv::Point> pixels;
    pixels.reserve(std::queue<ev::Event_<T>>::size());

    while(!std::queue<ev::Event_<T>>::empty()) {
      pixels.push_back(AbstractContainer_<Queue_<T>, T>::pixel_(std::queue<ev::Event_<T>>::front()));
      std::queue<ev::Event_<T>>::pop();
    }

    return AbstractContainer_<Queue_<T>, T>::entropy_(pixels);
  }
};
using Queuei = Queue_<int>;    /*!< Alias for Queue_ using int */
using Queuel = Queue_<long>;   /*!< Alias for Queue_ using long */
using Queuef = Queue_<float>;  /*!< Alias for Queue_ using float */
using Queued = Queue_<double>; /*!< Alias for Queue_ using double */
using Queue = Queuei;          /*!< Alias for Queue_ using Event */
} // namespace ev

#endif // OPENEV_CONTAINERS_QUEUE_HPP
