/*!
\file abstract-reader.hpp
\brief Abstract base class for event readers.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_ABSTRACT_READER_HPP
#define OPENEV_READERS_ABSTRACT_READER_HPP

#include "openev/containers/concurrent_queue.hpp"
#include "openev/core/types.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <opencv2/core/types.hpp>
#include <thread>

namespace ev {
[[maybe_unused]] constexpr bool USING_ABSTRACT_READER_HPP = true;

/*!
\brief This is an auxiliary class. This class cannot be instanced.

A reader delivers its events through a queue: events(n) reads n events and adds them to the queue. With prefetch(), a background thread keeps the queue at a given number of events.

\note The queue is safe between the thread of prefetch() and one thread using the reader.
*/
class AbstractReader_ {
public:
  /*! \cond INTERNAL */
  AbstractReader_() = default;
  virtual ~AbstractReader_();
  AbstractReader_(const AbstractReader_ &) = delete;
  AbstractReader_(AbstractReader_ &&) noexcept = delete;
  AbstractReader_ &operator=(const AbstractReader_ &) = delete;
  AbstractReader_ &operator=(AbstractReader_ &&) noexcept = delete;
  /*! \endcond */

  /*!
  \brief Read a given number of events, add them to the queue, and return it.
  \param n Number of events to add to the queue; none by default, which just returns the queue
  \return Reference to the queue.
  \note With prefetch(), the thread is who adds events, so this waits until the queue holds n events.
  \note With prefetch(), asking for more events than it keeps produces an exception.
  */
  [[nodiscard]] inline ConcurrentQueue &events(const std::size_t n = 0) {
    if(n == 0 && queue_) {
      return *queue_;
    }
    return fill_(n);
  }

  /*!
  \brief Keep the queue at a given number of events from a background thread.
  \param n Number of events to keep in the queue; zero stops the thread
  */
  void prefetch(const std::size_t n);

  /*!
  \brief Number of events prefetch() keeps in the queue.
  \return Number of events, zero if there is no thread
  */
  [[nodiscard]] inline std::size_t prefetchCapacity() const {
    return level_;
  }

  /*!
  \brief Go back to the beginning, discarding the events in the queue.
  */
  void reset();

  /*!
  \brief Transform the timestamp of every event read from now on as (t - offset) * scale.
  \param offset Time subtracted from the timestamps, in the units of the file
  \param scale Factor applied afterwards, for instance 1e6 to deliver microseconds from a file in seconds
  \note Meant to be set before reading.
  */
  void setTimeTransform(const TimeType offset, const TimeType scale = 1);

  /*!
  \brief Deliver only the events inside a region of interest. The events keep their coordinates.
  \param roi Region of interest in pixels; an empty region delivers every event
  \note Meant to be set before reading. Events already read are not affected.
  */
  void setRoi(const cv::Rect &roi);

  /*!
  \brief Get the region of interest.
  \return Region of interest in pixels, empty if every event is delivered
  */
  [[nodiscard]] inline cv::Rect getRoi() const {
    return roi_;
  }

protected:
  /*! \cond INTERNAL */
  virtual std::size_t read_(Event *events, const std::size_t max) = 0;
  virtual void reset_() = 0;
  bool stop_();
  /*! \endcond */

private:
  std::unique_ptr<ConcurrentQueue> queue_;
  std::thread thread_;
  std::atomic<bool> running_{false};
  std::atomic<bool> finished_{false};
  std::size_t level_{0};
  TimeType timeOffset_{0};
  TimeType timeScale_{1};
  cv::Rect roi_;

  static constexpr std::size_t BLOCK = 512;
  std::array<Event, BLOCK> block_;
  std::size_t blockPos_{0};
  std::size_t blockCount_{0};

  ConcurrentQueue &fill_(const std::size_t n);
  void readBlock_(const std::size_t max);
  void resize_(const std::size_t capacity);
  void start_();
};

} // namespace ev

#endif // OPENEV_READERS_ABSTRACT_READER_HPP
