/*!
\file queue.cpp
\brief Implementation of queue.
\author Raul Tapia
*/
#include "openev/containers/queue.hpp"

template <typename T>
[[nodiscard]] inline double ev::Queue_<T>::duration() const {
  return std::queue<ev::Event_<T>>::back().t - std::queue<ev::Event_<T>>::front().t;
}

template <typename T>
[[nodiscard]] inline double ev::Queue_<T>::rate() const {
  return std::queue<ev::Event_<T>>::size() / duration();
}

template <typename T>
[[nodiscard]] ev::Eventd ev::Queue_<T>::mean() const {
  const std::size_t n = std::queue<ev::Event_<T>>::size();
  double x{0};
  double y{0};
  double t{0};
  double p{0};

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

template <typename T>
[[nodiscard]] inline cv::Point2d ev::Queue_<T>::meanPoint() const {
  const std::size_t n = std::queue<ev::Event_<T>>::size();
  double x{0};
  double y{0};

  while(!std::queue<ev::Event_<T>>::empty()) {
    const Event_<T> &e = std::queue<ev::Event_<T>>::front();
    x += e.x;
    y += e.y;
    std::queue<ev::Event_<T>>::pop();
  }

  return {x / n, y / n};
}

template <typename T>
[[nodiscard]] inline double ev::Queue_<T>::meanTime() const {
  const std::size_t n = std::queue<ev::Event_<T>>::size();
  double t{0};

  while(!std::queue<ev::Event_<T>>::empty()) {
    t += std::queue<ev::Event_<T>>::front().t;
    std::queue<ev::Event_<T>>::pop();
  }

  return t / n;
}

template <typename T>
[[nodiscard]] inline double ev::Queue_<T>::midTime() const {
  return 0.5 * (std::queue<ev::Event_<T>>::front().t + std::queue<ev::Event_<T>>::back().t);
}
