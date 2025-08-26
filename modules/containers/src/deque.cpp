/*!
\file deque.cpp
\brief Implementation of deque.
\author Raul Tapia
*/
#include "openev/containers/deque.hpp"
#include <numeric>

template <typename T>
[[nodiscard]] inline double ev::Deque_<T>::duration() const {
  return std::deque<ev::Event_<T>>::back().t - std::deque<ev::Event_<T>>::front().t;
}

template <typename T>
[[nodiscard]] inline double ev::Deque_<T>::rate() const {
  return std::deque<ev::Event_<T>>::size() / duration();
}

template <typename T>
[[nodiscard]] ev::Eventd ev::Deque_<T>::mean() const {
  const double x = std::accumulate(std::deque<ev::Event_<T>>::begin(), std::deque<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::deque<ev::Event_<T>>::size();
  const double y = std::accumulate(std::deque<ev::Event_<T>>::begin(), std::deque<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::deque<ev::Event_<T>>::size();
  const double t = std::accumulate(std::deque<ev::Event_<T>>::begin(), std::deque<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::deque<ev::Event_<T>>::size();
  const double p = std::accumulate(std::deque<ev::Event_<T>>::begin(), std::deque<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.p; }) / std::deque<ev::Event_<T>>::size();
  return {x, y, t, p > 0.5};
}

template <typename T>
[[nodiscard]] inline cv::Point2d ev::Deque_<T>::meanPoint() const {
  const double x = std::accumulate(std::deque<ev::Event_<T>>::begin(), std::deque<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::deque<ev::Event_<T>>::size();
  const double y = std::accumulate(std::deque<ev::Event_<T>>::begin(), std::deque<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::deque<ev::Event_<T>>::size();
  return {x, y};
}

template <typename T>
[[nodiscard]] inline double ev::Deque_<T>::meanTime() const {
  return std::accumulate(std::deque<ev::Event_<T>>::begin(), std::deque<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::deque<ev::Event_<T>>::size();
}

template <typename T>
[[nodiscard]] inline double ev::Deque_<T>::midTime() const {
  return 0.5 * (std::deque<ev::Event_<T>>::front().t + std::deque<ev::Event_<T>>::back().t);
}

template class ev::Deque_<int>;
template class ev::Deque_<long>;
template class ev::Deque_<float>;
template class ev::Deque_<double>;
