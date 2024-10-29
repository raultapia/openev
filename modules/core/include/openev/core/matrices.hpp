/*!
\file matrices.hpp
\brief Basic event-based vision structures based on OpenCV components.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_MATRICES_HPP
#define OPENEV_CORE_MATRICES_HPP

#include "openev/core/types.hpp"

namespace ev {
class TimeMat : public cv::Mat_<double> {
public:
  template <typename... Args>
  explicit TimeMat(Args &&...args) : cv::Mat_<double>(std::forward<Args>(args)...) {
    TimeMat::clear();
  }

  template <typename T>
  inline const double &insert(const Event_<T> &e) {
    if constexpr(std::is_floating_point<T>::value) {
      return cv::Mat_<double>::operator()(std::round(e.y), std::round(e.x)) = e.t;
    } else {
      return cv::Mat_<double>::operator()(e) = e.t;
    }
  }

  template <typename T>
  inline const double &emplace(const T x, const T y, const double t) {
    if constexpr(std::is_floating_point<T>::value) {
      return cv::Mat_<double>::operator()(std::round(y), std::round(x)) = t;
    } else {
      return cv::Mat_<double>::operator()(y, x) = t;
    }
  }

  inline void clear() {
    cv::Mat_<double>::setTo(0);
  }
};

class PolarityMat : public cv::Mat_<bool> {
public:
  template <typename... Args>
  explicit PolarityMat(Args &&...args) : cv::Mat_<bool>(std::forward<Args>(args)...) {
    PolarityMat::clear();
  }

  template <typename T>
  inline const bool &insert(const Event_<T> &e) {
    if constexpr(std::is_floating_point<T>::value) {
      return cv::Mat_<bool>::operator()(std::round(e.y), std::round(e.x)) = e.p;
    } else {
      return cv::Mat_<bool>::operator()(e) = e.t;
    }
  }

  template <typename T>
  inline const bool &emplace(const T x, const T y, const bool p) {
    if constexpr(std::is_floating_point<T>::value) {
      return cv::Mat_<bool>::operator()(std::round(y), std::round(x)) = p;
    } else {
      return cv::Mat_<bool>::operator()(y, x) = p;
    }
  }

  inline void clear() {
    cv::Mat_<bool>::setTo(false);
  }
};

class CounterMat : public cv::Mat_<int> {
public:
  template <typename... Args>
  explicit CounterMat(Args &&...args) : cv::Mat_<int>(std::forward<Args>(args)...) {
    CounterMat::clear();
  }

  template <typename T>
  inline const int &insert(const Event_<T> &e) {
    if constexpr(std::is_floating_point<T>::value) {
      return cv::Mat_<int>::operator()(std::round(e.y), std::round(e.x)) += (e.p ? +1 : -1);
    } else {
      return cv::Mat_<int>::operator()(e) += (e.p ? +1 : -1);
    }
  }

  template <typename T>
  inline const int &emplace(const T x, const T y, const bool p) {
    if constexpr(std::is_floating_point<T>::value) {
      return cv::Mat_<int>::operator()(std::round(y), std::round(x)) += (p ? +1 : -1);
    } else {
      return cv::Mat_<int>::operator()(y, x) += (p ? +1 : -1);
    }
  }

  inline void clear() {
    cv::Mat_<int>::setTo(0);
  }
};

} // namespace ev

#endif
