/*!
\file reader.hpp
\brief Dataset reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_ABSTRACT_READER_HPP
#define OPENEV_READERS_ABSTRACT_READER_HPP

#include "openev/containers/array.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/core/types.hpp"
#include <atomic>
#include <cstddef>
#include <mutex>
#include <thread>

namespace ev {

/*!
\brief This is an auxiliary class. This class cannot be instanced.
*/
class AbstractReader_ {
public:
  /*! \cond INTERNAL */
  AbstractReader_(const AbstractReader_ &) = delete;
  AbstractReader_(AbstractReader_ &&) noexcept = delete;
  AbstractReader_ &operator=(const AbstractReader_ &) = delete;
  AbstractReader_ &operator=(AbstractReader_ &&) noexcept = delete;
  /*! \endcond */

  static constexpr std::size_t NO_BUFFER = 0;
  static constexpr std::size_t INF_BUFFER = std::numeric_limits<std::size_t>::max();

  /*!
  \brief Constructor for AbstractReader_.
  \param buffer_size The size of the buffer to be used by the reader.
  */
  AbstractReader_(const std::size_t buffer_size, const bool use_threading) : bufferSize_{buffer_size} {
    if(buffer_size > NO_BUFFER) {
      loadBuffer();
      if(use_threading) {
        threadRunning_.store(true);
        thread_ = std::thread(&AbstractReader_::threadFunction, this);
      }
    }
  }

  /*!
  \brief Destructor for AbstractReader_.
  */
  ~AbstractReader_() {
    if(thread_.joinable()) {
      threadRunning_.store(false);
      thread_.join();
    }
  }

  /*!
  \brief Read the next event.
  \param e Reference to an Event object where the next event will be stored.
  \return True if the event was successfully read, false otherwise.
  */
  bool read(Event &e) {
    if(bufferSize_ == NO_BUFFER) {
      return read_(e);
    }

    std::lock_guard<std::mutex> lock(bufferMutex_);
    if(buffer_.empty()) {
      if(!loadBuffer()) {
        return false;
      }
    }

    e = buffer_.front();
    buffer_.pop();
    return true;
  }

  /*!
  \brief Read next n events.
  \tparam N The size of the array.
  \param array Reference to an Array object where the events will be stored.
  \return True if all events were successfully read, false otherwise.
  */
  template <std::size_t N>
  bool read(Array<N> &array) {
    for(Event &e : array) {
      if(!read(e)) {
        return false;
      }
    }
    return true;
  }

  /*!
  \brief Read next n events.
  \param vector Event vector
  \param n Number of events to get
  \return True if vector populated with n new events
  */
  bool read(Vector &vector, const int n);

  /*!
  \brief Read next n events.
  \param n Number of events to get
  \param queue Event queue
  \param keep_size If true, pop one event for each insertion to maintain queue size.
  \return True if queue populated with n new events
  */
  bool read(Queue &queue, const int n, const bool keep_size = false);

  /*!
  \brief Read the next events until the specified duration is reached.
  \param vector Event vector to store the events.
  \param t Duration to get events for.
  \return True if the vector is populated with events for the specified duration.
  */
  bool read_t(Vector &vector, const double t);

  /*!
  \brief Get the next events until the specified duration is reached and store them in a queue.
  \param t Duration to get events for.
  \param queue Event queue to store the events.
  \param keep_size If true, pop one event for each insertion to maintain queue size.
  \return True if the queue is populated with events for the specified duration.
  */
  bool read_t(Queue &queue, const double t, const bool keep_size = false);

  /*!
  \brief Skip the next n events.
  \param n Number of events to skip.
  \return True if the skip was successful.
  */
  bool skip(int n);

  /*!
  \brief Skip events for the specified duration.
  \param t Duration to skip events for.
  \return True if the skip was successful.
  */
  bool skip_t(const double t);

  /*!
  \brief Start reading from the first event.
  \note The behaviour of reset should be implemented in the derived classes.
  */
  void reset() {
    bool reset_thread = false;
    if(thread_.joinable()) {
      threadRunning_.store(false);
      thread_.join();
      reset_thread = true;
    }

    reset_();

    if(bufferSize_ > NO_BUFFER) {
      while(!buffer_.empty()) {
        buffer_.pop();
      }
    }

    if(reset_thread) {
      threadRunning_.store(true);
      thread_ = std::thread(&AbstractReader_::threadFunction, this);
    }
  }

  /*!
  \brief Count the total number of events available.
  \return The total number of events available.
  */
  std::size_t count();

protected:
  const std::size_t bufferSize_;
  std::thread thread_;
  Queue buffer_;
  std::mutex bufferMutex_;
  std::atomic<bool> threadRunning_;

  virtual bool read_(Event &e) = 0;
  virtual void reset_() = 0;

private:
  void threadFunction() {
    while(threadRunning_.load() && loadBuffer()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  bool loadBuffer() {
    Event e;
    std::lock_guard<std::mutex> lock(bufferMutex_);
    while(buffer_.size() < bufferSize_) {
      if(read_(e)) {
        buffer_.push(e);
      } else {
        return false;
      }
    }
    return true;
  }
};

} // namespace ev

#endif // OPENEV_READERS_ABSTRACT_READER_HPP
