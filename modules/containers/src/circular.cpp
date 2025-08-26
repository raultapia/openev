/*!
\file circular.cpp
\brief Implementation of circular.
\author Raul Tapia
*/
#include "openev/containers/circular.hpp"
#include <boost/core/pointer_traits.hpp>
#include <numeric>

template <typename T>
[[nodiscard]] inline double ev::CircularBuffer_<T>::duration() const {
  return boost::circular_buffer<ev::Event_<T>>::back().t - boost::circular_buffer<ev::Event_<T>>::front().t;
}

template <typename T>
[[nodiscard]] inline double ev::CircularBuffer_<T>::rate() const {
  return boost::circular_buffer<ev::Event_<T>>::size() / duration();
}

template <typename T>
[[nodiscard]] ev::Eventd ev::CircularBuffer_<T>::mean() const {
  const double x = std::accumulate(boost::circular_buffer<ev::Event_<T>>::begin(), boost::circular_buffer<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / boost::circular_buffer<ev::Event_<T>>::size();
  const double y = std::accumulate(boost::circular_buffer<ev::Event_<T>>::begin(), boost::circular_buffer<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / boost::circular_buffer<ev::Event_<T>>::size();
  const double t = std::accumulate(boost::circular_buffer<ev::Event_<T>>::begin(), boost::circular_buffer<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / boost::circular_buffer<ev::Event_<T>>::size();
  const double p = std::accumulate(boost::circular_buffer<ev::Event_<T>>::begin(), boost::circular_buffer<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.p; }) / boost::circular_buffer<ev::Event_<T>>::size();
  return {x, y, t, p > 0.5};
}

template <typename T>
[[nodiscard]] inline cv::Point2d ev::CircularBuffer_<T>::meanPoint() const {
  const double x = std::accumulate(boost::circular_buffer<ev::Event_<T>>::begin(), boost::circular_buffer<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.x; }) / boost::circular_buffer<ev::Event_<T>>::size();
  const double y = std::accumulate(boost::circular_buffer<ev::Event_<T>>::begin(), boost::circular_buffer<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.y; }) / boost::circular_buffer<ev::Event_<T>>::size();
  return {x, y};
}

template <typename T>
[[nodiscard]] inline double ev::CircularBuffer_<T>::meanTime() const {
  return std::accumulate(boost::circular_buffer<ev::Event_<T>>::begin(), boost::circular_buffer<ev::Event_<T>>::end(), 0.0, [](double sum, const Event_<T> &e) { return sum + e.t; }) / boost::circular_buffer<ev::Event_<T>>::size();
}

template <typename T>
[[nodiscard]] inline double ev::CircularBuffer_<T>::midTime() const {
  return 0.5 * (boost::circular_buffer<ev::Event_<T>>::front().t + boost::circular_buffer<ev::Event_<T>>::back().t);
}

template class ev::CircularBuffer_<int>;
template class ev::CircularBuffer_<long>;
template class ev::CircularBuffer_<float>;
template class ev::CircularBuffer_<double>;
