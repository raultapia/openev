/*!
\file array.cpp
\brief Implementation of array.
\author Raul Tapia
*/
#include "openev/containers/array.hpp"
#include <numeric>

template <typename T, std::size_t N>
[[nodiscard]] inline double ev::Array_<T, N>::duration() const {
  return std::array<ev::Event_<T>, N>::back().t - std::array<ev::Event_<T>, N>::front().t;
}

template <typename T, std::size_t N>
[[nodiscard]] inline double ev::Array_<T, N>::rate() const {
  return std::array<ev::Event_<T>, N>::size() / duration();
}

template <typename T, std::size_t N>
[[nodiscard]] ev::Eventd ev::Array_<T, N>::mean() const {
  const double x = std::accumulate(std::array<ev::Event_<T>, N>::begin(), std::array<ev::Event_<T>, N>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / N;
  const double y = std::accumulate(std::array<ev::Event_<T>, N>::begin(), std::array<ev::Event_<T>, N>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / N;
  const double t = std::accumulate(std::array<ev::Event_<T>, N>::begin(), std::array<ev::Event_<T>, N>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / N;
  const double p = std::accumulate(std::array<ev::Event_<T>, N>::begin(), std::array<ev::Event_<T>, N>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.p; }) / N;
  return {x, y, t, p > 0.5};
}

template <typename T, std::size_t N>
[[nodiscard]] inline cv::Point2d ev::Array_<T, N>::meanPoint() const {
  const double x = std::accumulate(std::array<ev::Event_<T>, N>::begin(), std::array<ev::Event_<T>, N>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / N;
  const double y = std::accumulate(std::array<ev::Event_<T>, N>::begin(), std::array<ev::Event_<T>, N>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / N;
  return {x, y};
}

template <typename T, std::size_t N>
[[nodiscard]] inline double ev::Array_<T, N>::meanTime() const {
  return std::accumulate(std::array<ev::Event_<T>, N>::begin(), std::array<ev::Event_<T>, N>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / N;
}

template <typename T, std::size_t N>
[[nodiscard]] inline double ev::Array_<T, N>::midTime() const {
  return 0.5 * (std::array<ev::Event_<T>, N>::front().t + std::array<ev::Event_<T>, N>::back().t);
}
