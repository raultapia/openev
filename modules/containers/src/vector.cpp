/*!
\file vector.cpp
\brief Implementation of vector.
\author Raul Tapia
*/
#include "openev/containers/vector.hpp"
#include <numeric>

template <typename T>
[[nodiscard]] inline double ev::Vector_<T>::duration() const {
  return std::vector<ev::Event_<T>>::back().t - std::vector<ev::Event_<T>>::front().t;
}

template <typename T>
[[nodiscard]] inline double ev::Vector_<T>::rate() const {
  return std::vector<ev::Event_<T>>::size() / duration();
}

template <typename T>
[[nodiscard]] ev::Eventd ev::Vector_<T>::mean() const {
  const double x = std::accumulate(std::vector<ev::Event_<T>>::begin(), std::vector<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::vector<ev::Event_<T>>::size();
  const double y = std::accumulate(std::vector<ev::Event_<T>>::begin(), std::vector<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::vector<ev::Event_<T>>::size();
  const double t = std::accumulate(std::vector<ev::Event_<T>>::begin(), std::vector<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::vector<ev::Event_<T>>::size();
  const double p = std::accumulate(std::vector<ev::Event_<T>>::begin(), std::vector<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.p; }) / std::vector<ev::Event_<T>>::size();
  return {x, y, t, p > 0.5};
}

template <typename T>
[[nodiscard]] inline cv::Point2d ev::Vector_<T>::meanPoint() const {
  const double x = std::accumulate(std::vector<ev::Event_<T>>::begin(), std::vector<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / std::vector<ev::Event_<T>>::size();
  const double y = std::accumulate(std::vector<ev::Event_<T>>::begin(), std::vector<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / std::vector<ev::Event_<T>>::size();
  return {x, y};
}

template <typename T>
[[nodiscard]] inline double ev::Vector_<T>::meanTime() const {
  return std::accumulate(std::vector<ev::Event_<T>>::begin(), std::vector<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / std::vector<ev::Event_<T>>::size();
}

template <typename T>
[[nodiscard]] inline double ev::Vector_<T>::midTime() const {
  return 0.5 * (std::vector<ev::Event_<T>>::front().t + std::vector<ev::Event_<T>>::back().t);
}

template class ev::Vector_<int>;
template class ev::Vector_<long>;
template class ev::Vector_<float>;
template class ev::Vector_<double>;
