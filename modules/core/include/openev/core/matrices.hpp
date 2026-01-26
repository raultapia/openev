/*!
\file matrices.hpp
\brief Basic event-based vision structures based on OpenCV components.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_MATRICES_HPP
#define OPENEV_CORE_MATRICES_HPP

#include <cmath>
#include <limits>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/traits.hpp>
#include <ostream>
#include <type_traits>

namespace ev {
/*! \cond INTERNAL */
template <typename T>
class Event_;
/*! \endcond */

namespace Mat {
template <typename Tb>
class Binary_ : public cv::Mat_<Tb> {
public:
  using cv::Mat_<Tb>::Mat_;

  template <typename T>
  inline Tb insert(const Event_<T> &e) {
    return set(e.x, e.y);
  }

  template <typename T>
  inline Tb emplace(const T x, const T y) {
    return set(x, y);
  }

  inline void clear() {
    cv::Mat_<Tb>::setTo(0);
  }

  static constexpr Tb ON = std::numeric_limits<Tb>::max();
  static constexpr Tb OFF = static_cast<Tb>(0);

  friend std::ostream &operator<<(std::ostream &os, const Binary_ &binary) {
    os << "Binary " << binary.cols << "x" << binary.rows;
    return os;
  }

private:
  template <typename T>
  inline Tb set(const T x, const T y) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->template ptr<Tb>(std::lround(y)) + std::lround(x)) = ON;
    } else {
      return *(this->template ptr<Tb>(y) + x) = ON;
    }
  }
};
using Binary = Binary_<uchar>;

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

  friend std::ostream &operator<<(std::ostream &os, const Time &time) {
    os << "Time " << time.cols << "x" << time.rows;
    return os;
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
    cv::Mat_<bool>::setTo(0);
  }

  friend std::ostream &operator<<(std::ostream &os, const Polarity &polarity) {
    os << "Polarity " << polarity.cols << "x" << polarity.rows;
    return os;
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

  friend std::ostream &operator<<(std::ostream &os, const Counter &counter) {
    os << "Counter " << counter.cols << "x" << counter.rows;
    return os;
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
