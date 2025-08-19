/*!
\file matrices.hpp
\brief Basic event-based vision structures based on OpenCV components.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_MATRICES_HPP
#define OPENEV_CORE_MATRICES_HPP

#include "openev/core/types.hpp"

namespace ev {
namespace Mat {
template <typename Tm = std::uint8_t>
class Binary : public cv::Mat_<Tm> {
public:
  using cv::Mat_<Tm>::Mat_;

  template <typename T>
  inline Tm insert(const Event_<T> &e) {
    return set(e.x, e.y);
  }

  template <typename T>
  inline Tm emplace(const T x, const T y) {
    return set(x, y);
  }

  inline void clear() {
    cv::Mat_<Tm>::setTo(0);
  }

private:
  static constexpr Tm value_ = std::numeric_limits<Tm>::max();

  template <typename T>
  inline Tm set(const T x, const T y) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->template ptr<Tm>(std::lround(y)) + std::lround(x)) = value_;
    } else {
      return *(this->template ptr<Tm>(y) + x) = value_;
    }
  }
};

class Time : public cv::Mat_<double> {
public:
  using cv::Mat_<double>::Mat_;

  template <typename T>
  inline double insert(const Event_<T> &e) {
    return set(e.x, e.y, e.t);
  }

  template <typename T>
  inline double emplace(const T x, const T y, const double t) {
    return set(x, y, t);
  }

  inline void clear() {
    cv::Mat_<double>::setTo(0);
  }

private:
  template <typename T>
  inline double set(const T x, const T y, const double t) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->ptr<double>(std::lround(y)) + std::lround(x)) = t;
    } else {
      return *(this->ptr<double>(y) + x) = t;
    }
  }
};

class Polarity : public cv::Mat_<bool> {
public:
  using cv::Mat_<bool>::Mat_;

  template <typename T>
  inline bool insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  template <typename T>
  inline bool emplace(const T x, const T y, const bool p) {
    return set(x, y, p);
  }

  inline void clear() {
    cv::Mat_<bool>::setTo(false);
  }

private:
  template <typename T>
  inline bool set(const T x, const T y, const bool p) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->ptr<bool>(std::lround(y)) + std::lround(x)) = p;
    } else {
      return *(this->ptr<bool>(y) + x) = p;
    }
  }
};

class Counter : public cv::Mat_<int> {
public:
  using cv::Mat_<int>::Mat_;

  template <typename T>
  inline int insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  template <typename T>
  inline int emplace(const T x, const T y, const bool p) {
    return set(x, y, p);
  }

  inline void clear() {
    cv::Mat_<int>::setTo(0);
  }

private:
  template <typename T>
  inline int set(const T x, const T y, const bool p) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->ptr<int>(std::lround(y)) + std::lround(x)) += (p ? +1 : -1);
    } else {
      return *(this->ptr<int>(y) + x) += (p ? +1 : -1);
    }
  }
};
} // namespace Mat
} // namespace ev

#endif
