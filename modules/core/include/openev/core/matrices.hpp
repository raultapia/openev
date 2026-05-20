/*!
\file matrices.hpp
\brief Basic event-based vision structures based on OpenCV components.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_MATRICES_HPP
#define OPENEV_CORE_MATRICES_HPP

#include "openev/core/types.hpp"
#include <cmath>
#include <cstring>
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
namespace detail {
template <typename MatType>
inline void clearZero(MatType &mat) {
  if(mat.empty()) {
    return;
  }
  if(mat.isContinuous()) {
    std::memset(mat.data, 0, mat.total() * mat.elemSize());
    return;
  }
  mat.setTo(0);
}
} // namespace detail

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
    detail::clearZero(*this);
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

template <typename Tb>
class Ternary_ : public cv::Mat_<Tb> {
public:
  using cv::Mat_<Tb>::Mat_;

  template <typename T>
  inline Tb insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  template <typename T>
  inline Tb emplace(const T x, const T y, const bool p) {
    return set(x, y, p);
  }

  inline void clear() {
    detail::clearZero(*this);
  }

  static constexpr Tb POSITIVE = std::numeric_limits<Tb>::max();
  static constexpr Tb ZERO = static_cast<Tb>(0);
  static constexpr Tb NEGATIVE = std::numeric_limits<Tb>::min();

  friend std::ostream &operator<<(std::ostream &os, const Ternary_ &ternary) {
    os << "Ternary " << ternary.cols << "x" << ternary.rows;
    return os;
  }

private:
  template <typename T>
  inline Tb set(const T x, const T y, const bool p) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->template ptr<Tb>(std::lround(y)) + std::lround(x)) = (p ? POSITIVE : NEGATIVE);
    } else {
      return *(this->template ptr<Tb>(y) + x) = (p ? POSITIVE : NEGATIVE);
    }
  }
};
using Ternary = Ternary_<char>;

class Time : public cv::Mat_<TimeType> {
public:
  using cv::Mat_<TimeType>::Mat_;

  template <typename T>
  inline TimeType insert(const Event_<T> &e) {
    return set(e.x, e.y, e.t);
  }

  template <typename T>
  inline TimeType emplace(const T x, const T y, const TimeType t) {
    return set(x, y, t);
  }

  inline void clear() {
    detail::clearZero(*this);
  }

  friend std::ostream &operator<<(std::ostream &os, const Time &time) {
    os << "Time " << time.cols << "x" << time.rows;
    return os;
  }

private:
  template <typename T>
  inline TimeType set(const T x, const T y, const TimeType t) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->ptr<TimeType>(std::lround(y)) + std::lround(x)) = t;
    } else {
      return *(this->ptr<TimeType>(y) + x) = t;
    }
  }
};

class Polarity : public cv::Mat_<PolarityType> {
public:
  using cv::Mat_<PolarityType>::Mat_;

  template <typename T>
  inline PolarityType insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  template <typename T>
  inline PolarityType emplace(const T x, const T y, const PolarityType p) {
    return set(x, y, p);
  }

  inline void clear() {
    detail::clearZero(*this);
  }

  friend std::ostream &operator<<(std::ostream &os, const Polarity &polarity) {
    os << "Polarity " << polarity.cols << "x" << polarity.rows;
    return os;
  }

private:
  template <typename T>
  inline PolarityType set(const T x, const T y, const PolarityType p) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->ptr<PolarityType>(std::lround(y)) + std::lround(x)) = p;
    } else {
      return *(this->ptr<PolarityType>(y) + x) = p;
    }
  }
};

class Counter : public cv::Mat_<CounterType> {
public:
  using cv::Mat_<CounterType>::Mat_;

  template <typename T>
  inline CounterType insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  template <typename T>
  inline CounterType emplace(const T x, const T y, const bool p) {
    return set(x, y, p);
  }

  inline void clear() {
    detail::clearZero(*this);
  }

  friend std::ostream &operator<<(std::ostream &os, const Counter &counter) {
    os << "Counter " << counter.cols << "x" << counter.rows;
    return os;
  }

private:
  template <typename T>
  inline CounterType set(const T x, const T y, const bool p) {
    if constexpr(std::is_floating_point_v<T>) {
      return *(this->ptr<CounterType>(std::lround(y)) + std::lround(x)) += (p ? +1 : -1);
    } else {
      return *(this->ptr<CounterType>(y) + x) += (p ? +1 : -1);
    }
  }
};
} // namespace Mat
} // namespace ev

#endif
