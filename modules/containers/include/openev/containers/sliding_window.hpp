/*!
\file sliding_window.hpp
\brief Sliding time-window container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_SLIDING_WINDOW_HPP
#define OPENEV_CONTAINERS_SLIDING_WINDOW_HPP

#include "openev/containers/deque.hpp"
#include "openev/core/types.hpp"
#include <opencv2/core/types.hpp>
#include <utility>

namespace ev {
constexpr bool USING_SLIDING_WINDOW_HPP = true;

/*!
\brief FIFO event container that retains only events within a trailing time window.

Events are pushed to the back; after each insertion, front events older than
\f$ t_{newest} - window \f$ are automatically discarded. Inherits all Deque_ statistics
(duration, rate, mean, etc.) computed over the retained events.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using SlidingWindowi = SlidingWindow_<int>;
using SlidingWindowl = SlidingWindow_<long>;
using SlidingWindowf = SlidingWindow_<float>;
using SlidingWindowd = SlidingWindow_<double>;
using SlidingWindow  = SlidingWindowi;
\endcode
*/
template <typename T>
class SlidingWindow_ : public Deque_<T> {
public:
  /*!
  \brief Construct a sliding window with the given temporal span.
  \param window Time window length in the same units as Event_::t (default 0, retains all events).
  */
  explicit SlidingWindow_(const TimeType window = 0) : window_{window} {}

  /*!
  \brief Return the current time window length.
  \return Window length
  */
  [[nodiscard]] inline TimeType window() const {
    return window_;
  }

  /*!
  \brief Set a new time window length and immediately discard events outside the new window.
  \param window New window length
  */
  inline void setWindow(const TimeType window) {
    window_ = window;
    trim();
  }

  /*!
  \brief Push an event to the back and discard front events older than the window.
  \param event Event to add
  */
  inline void push(const Event_<T> &event) {
    std::deque<Event_<T>>::push_back(event);
    trim();
  }

  /*!
  \brief Push an event (move) to the back and discard front events older than the window.
  \param event Event to add
  */
  inline void push(Event_<T> &&event) {
    std::deque<Event_<T>>::push_back(std::move(event));
    trim();
  }

  /*!
  \brief Construct an event in-place at the back and discard front events older than the window.
  */
  template <typename... Args>
  inline void emplace(Args &&...args) {
    std::deque<Event_<T>>::emplace_back(std::forward<Args>(args)...);
    trim();
  }

private:
  inline void trim() {
    if(window_ <= 0 || Deque_<T>::empty()) {
      return;
    }
    const auto newest = std::deque<Event_<T>>::back().t;
    while(Deque_<T>::size() > 1 && (newest - Deque_<T>::front().t) > window_) {
      Deque_<T>::pop_front();
    }
  }

  TimeType window_{0};
};

using SlidingWindowi = SlidingWindow_<int>;    /*!< Alias for SlidingWindow_ using int */
using SlidingWindowl = SlidingWindow_<long>;   /*!< Alias for SlidingWindow_ using long */
using SlidingWindowf = SlidingWindow_<float>;  /*!< Alias for SlidingWindow_ using float */
using SlidingWindowd = SlidingWindow_<double>; /*!< Alias for SlidingWindow_ using double */
using SlidingWindow = SlidingWindowi;          /*!< Alias for SlidingWindow_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_SLIDING_WINDOW_HPP
