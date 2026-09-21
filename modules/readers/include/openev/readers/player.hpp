/*!
\file player.hpp
\brief Real-time playback of a reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_PLAYER_HPP
#define OPENEV_READERS_PLAYER_HPP

#include "openev/containers/concurrent_queue.hpp"
#include "openev/containers/queue.hpp"
#include "openev/core/frames.hpp"
#include "openev/core/imu.hpp"
#include "openev/core/types.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <opencv2/imgcodecs.hpp>
#include <type_traits>
#include <utility>

namespace ev {
[[maybe_unused]] constexpr bool USING_PLAYER_HPP = true;

/*! \cond INTERNAL */
template <typename R, typename = void>
struct has_frames_ : std::false_type {};
template <typename R>
struct has_frames_<R, std::void_t<decltype(std::declval<const R &>().frames())>> : std::true_type {};

template <typename R, typename = void>
struct has_imu_ : std::false_type {};
template <typename R>
struct has_imu_<R, std::void_t<decltype(std::declval<const R &>().imu())>> : std::true_type {};
/*! \endcond */

/*!
\brief This class plays a reader, or anything offering events() and reset() as readers do.

\note Timestamps are expected in microseconds.
*/
template <typename Reader>
class Player_ {
public:
  /*!
  \brief Construct the player. The playback starts with the first call to next().
  \param reader Reader to play
  */
  explicit Player_(Reader &reader) : reader_{reader} {}

  /*!
  \brief Set the playback speed.
  \param speed Speed relative to real time
  */
  inline void setSpeed(const double speed) {
    speed_ = speed;
  }

  /*!
  \brief Get the playback speed.
  \return Speed relative to real time
  */
  [[nodiscard]] inline double speed() const {
    return speed_;
  }

  /*!
  \brief Hold the playhead, so next() delivers nothing.
  \param paused True to pause, false to resume
  */
  inline void pause(const bool paused) {
    paused_ = paused;
  }

  /*!
  \brief Check if the playback is paused.
  \return True if paused
  */
  [[nodiscard]] inline bool isPaused() const {
    return paused_;
  }

  /*!
  \brief Choose what happens when the reader runs out of events.
  \param loop True to start again, false to stop
  */
  inline void setLoop(const bool loop) {
    loop_ = loop;
  }

  /*!
  \brief Start again from the beginning.
  */
  void restart() {
    reader_.reset();
    playhead_ = 0;
    frame_ = 0;
    sample_ = 0;
    finished_ = false;
    loops_++;
  }

  /*!
  \brief Number of times the playback has started again, either by restart() or by reaching the end.
  \return Count
  */
  [[nodiscard]] inline std::size_t loops() const {
    return loops_;
  }

  /*!
  \brief Current position of the playback.
  \return Playhead in microseconds
  */
  [[nodiscard]] inline TimeType playhead() const {
    return playhead_;
  }

  /*!
  \brief Check if the reader ran out of events with setLoop(false).
  \return True if there is nothing else to deliver
  */
  [[nodiscard]] inline bool isFinished() const {
    return finished_;
  }

  /*!
  \brief Deliver what happened since the previous call.
  \param events Events are pushed here
  */
  void next(Queue &events) {
    advance_(elapsed_(), events, nullptr, nullptr);
  }

  /*!
  \overload
  \param frames Frames are pushed here
  */
  void next(Queue &events, StampedMatQueue &frames) {
    advance_(elapsed_(), events, &frames, nullptr);
  }

  /*!
  \overload
  \param imus Inertial measurements are pushed here
  */
  void next(Queue &events, StampedMatQueue &frames, ImuQueue &imus) {
    advance_(elapsed_(), events, &frames, &imus);
  }

private:
  static constexpr double SECONDS_TO_MICROSECONDS = 1e6;
  static constexpr std::size_t BATCH = 256;

  Reader &reader_;
  std::chrono::steady_clock::time_point last_;
  bool started_{false};
  bool paused_{false};
  bool loop_{true};
  bool finished_{false};
  double speed_{1.0};
  TimeType playhead_{0};
  std::size_t frame_{0};
  std::size_t sample_{0};
  std::size_t loops_{0};

  [[nodiscard]] std::size_t batch_() const {
    const std::size_t level = reader_.prefetchCapacity();
    return level > 0 ? std::min(level, BATCH) : BATCH;
  }

  [[nodiscard]] TimeType elapsed_() {
    const auto now = std::chrono::steady_clock::now();
    const TimeType elapsed = started_ && !paused_ ? std::chrono::duration<double>(now - last_).count() * SECONDS_TO_MICROSECONDS * speed_ : 0;
    last_ = now;
    started_ = true;
    return elapsed;
  }

protected:
  /*! \cond INTERNAL */
  void advance_(const TimeType time, Queue &events, StampedMatQueue *frames, ImuQueue *imus) {
    if(finished_) {
      return;
    }
    playhead_ += time;

    while(true) {
      ConcurrentQueue *pending = &reader_.events();
      if(pending->empty()) {
        pending = &reader_.events(batch_());
      }
      ConcurrentQueue &queue = *pending;
      if(queue.empty()) {
        if(!loop_) {
          finished_ = true;
          break;
        }
        restart();
        finished_ = reader_.events(batch_()).empty();
        break;
      }
      const Event &e = queue.front();
      if(e.t > playhead_) {
        break;
      }
      events.push(e);
      queue.pop();
    }

    if constexpr(has_frames_<Reader>::value) {
      if(frames != nullptr) {
        const auto &all = reader_.frames();
        while(frame_ < all.size() && all[frame_].t <= playhead_) {
          StampedMat frame;
          cv::imread(all[frame_].path, cv::IMREAD_GRAYSCALE).copyTo(frame);
          frame.t = all[frame_].t;
          if(!frame.empty()) {
            frames->push(frame);
          }
          frame_++;
        }
      }
    }

    if constexpr(has_imu_<Reader>::value) {
      if(imus != nullptr) {
        const auto &all = reader_.imu();
        while(sample_ < all.size() && all[sample_].t <= playhead_) {
          imus->push(all[sample_]);
          sample_++;
        }
      }
    }
  }
  /*! \endcond */
};

} // namespace ev

#endif // OPENEV_READERS_PLAYER_HPP
