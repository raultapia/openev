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
\brief FIFO event container that retains only the trailing time window.
*/
template <typename T>
class SlidingWindow_ : public Deque_<T> {
public:
  explicit SlidingWindow_(const TimeType window = 0) : window_{window} {}

  [[nodiscard]] inline TimeType window() const {
    return window_;
  }

  inline void setWindow(const TimeType window) {
    window_ = window;
    trim();
  }

  inline void push(const Event_<T> &event) {
    std::deque<Event_<T>>::push_back(event);
    trim();
  }

  inline void push(Event_<T> &&event) {
    std::deque<Event_<T>>::push_back(std::move(event));
    trim();
  }

  template <typename... Args>
  inline void emplace(Args &&...args) {
    std::deque<Event_<T>>::emplace_back(std::forward<Args>(args)...);
    trim();
  }

private:
  inline void trim() {
    if(Deque_<T>::empty()) {
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
