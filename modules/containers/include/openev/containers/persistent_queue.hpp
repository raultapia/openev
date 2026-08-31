/*!
\file queue.hpp
\brief Persistent queue container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_PERSISTENT_QUEUE_HPP
#define OPENEV_CONTAINERS_PERSISTENT_QUEUE_HPP

#include "openev/containers/queue.hpp"
#include "openev/core/types.hpp"
#include <cstddef>
#include <cstdint>
#include <opencv2/core/types.hpp>
#include <unordered_map>

namespace ev {
constexpr bool USING_PERSISTENT_QUEUE_HPP = true;

/*!
\brief This class extends std::queue to implement persistent event queues. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/queue">here</a>.

Persistent event queues inherit the FIFO behaviour of standard C++ queues while preserving their contents after aggregate queries.
*/
template <typename T>
class PersistentQueue_ : public Queue_<T> {
  using Queue_<T>::Queue_;
  using ResultType = TimeType;

public:
  /*!
  \brief Compute the mean of the events without consuming the queue.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  \note Unlike Queue_::mean(), the queue contents are preserved after this call.
  */
  [[nodiscard]] inline Event_<ResultType> mean() {
    if(Queue_<T>::empty()) {
      CV_Error(cv::Error::StsError, "ev::PersistentQueue_::mean: the container is empty.");
    }
    const std::size_t n = Queue_<T>::size();
    ResultType x{0};
    ResultType y{0};
    ResultType t{0};
    ResultType p{0};

    for(int i = 0; i < n; i++) {
      const Event_<T> &e = Queue_<T>::front();
      x += e.x;
      y += e.y;
      t += e.t;
      p += e.p;
      Queue_<T>::pop();
      Queue_<T>::emplace(e);
    }

    return {x / n, y / n, t / n, p / n > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events without consuming the queue.
  \return Mean point
  \note Unlike Queue_::meanPoint(), the queue contents are preserved after this call.
  */
  [[nodiscard]] inline cv::Point_<ResultType> meanPoint() {
    if(Queue_<T>::empty()) {
      CV_Error(cv::Error::StsError, "ev::PersistentQueue_::meanPoint: the container is empty.");
    }
    const std::size_t n = Queue_<T>::size();
    ResultType x{0};
    ResultType y{0};

    for(int i = 0; i < n; i++) {
      const Event_<T> &e = Queue_<T>::front();
      x += e.x;
      y += e.y;
      Queue_<T>::pop();
      Queue_<T>::emplace(e);
    }

    return {x / n, y / n};
  }

  /*!
  \brief Compute the mean time of the events without consuming the queue.
  \return Mean time
  \note Unlike Queue_::meanTime(), the queue contents are preserved after this call.
  */
  [[nodiscard]] inline ResultType meanTime() {
    if(Queue_<T>::empty()) {
      CV_Error(cv::Error::StsError, "ev::PersistentQueue_::meanTime: the container is empty.");
    }
    const std::size_t n = Queue_<T>::size();
    ResultType t{0};

    for(int i = 0; i < n; i++) {
      t += Queue_<T>::front().t;
      Queue_<T>::emplace(Queue_<T>::front());
      Queue_<T>::pop();
    }

    return t / n;
  }

  /*!
  \brief Compute the Shannon entropy of the spatial distribution of the events without consuming the queue.
  \return Entropy in bits
  \note \f$ H = -\sum_i p_i \log_2 p_i \f$, where \f$ p_i \f$ is the fraction of events falling on the i-th pixel, so \f$ 2^H \f$ is the effective number of active pixels.
  \note Unlike Queue_::entropy(), the queue contents are preserved after this call.
  */
  [[nodiscard]] inline ResultType entropy() {
    if(Queue_<T>::empty()) {
      CV_Error(cv::Error::StsError, "ev::PersistentQueue_::entropy: the container is empty.");
    }
    const std::size_t n = Queue_<T>::size();
    std::unordered_map<uint64_t, std::size_t> histogram;

    for(int i = 0; i < n; i++) {
      const Event_<T> &e = Queue_<T>::front();
      histogram[AbstractContainer_<Queue_<T>, T>::pixel_(e)]++;
      Queue_<T>::pop();
      Queue_<T>::emplace(e);
    }

    return AbstractContainer_<Queue_<T>, T>::entropy_(histogram, static_cast<ResultType>(n));
  }
};
using PersistentQueuei = PersistentQueue_<int>;    /*!< Alias for PersistentQueue_ using int */
using PersistentQueuel = PersistentQueue_<long>;   /*!< Alias for PersistentQueue_ using long */
using PersistentQueuef = PersistentQueue_<float>;  /*!< Alias for PersistentQueue_ using float */
using PersistentQueued = PersistentQueue_<double>; /*!< Alias for PersistentQueue_ using double */
using PersistentQueue = PersistentQueuei;          /*!< Alias for PersistentQueue_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_PERSISTENT_QUEUE_HPP
