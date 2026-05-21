/*!
\file matrices.hpp
\brief Event-based matrix representations for accumulating events into spatial maps.
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
constexpr bool USING_MATRICES_HPP = true;

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

/*!
\brief Spatial map marking whether any event has occurred at each pixel.

Each pixel is set to ON upon insertion regardless of polarity, and reset to OFF by clear().

The following alias is defined for convenience:
\code{.cpp}
using Binary = Binary_<uchar>;
\endcode
*/
template <typename Tb>
class Binary_ : public cv::Mat_<Tb> {
public:
  using cv::Mat_<Tb>::Mat_;

  /*!
  \brief Insert an event, setting the pixel at (e.x, e.y) to ON.
  \param e Event to insert
  \return New pixel value (ON)
  */
  template <typename T>
  inline Tb insert(const Event_<T> &e) {
    return set(e.x, e.y);
  }

  /*!
  \brief Set the pixel at (x, y) to ON without constructing an Event_.
  \param x Spatial coordinate x
  \param y Spatial coordinate y
  \return New pixel value (ON)
  */
  template <typename T>
  inline Tb emplace(const T x, const T y) {
    return set(x, y);
  }

  /*!
  \brief Reset all pixels to OFF.
  */
  inline void clear() {
    detail::clearZero(*this);
  }

  static constexpr Tb ON = std::numeric_limits<Tb>::max(); /*!< Value written on event insertion */
  static constexpr Tb OFF = static_cast<Tb>(0);            /*!< Value after clear() */

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

/*!
\brief Spatial map encoding event polarity at each pixel.

Positive-polarity events set the pixel to POSITIVE; negative-polarity events set it to NEGATIVE.
Pixels with no events remain at ZERO. Only the last inserted polarity per pixel is retained.

The following alias is defined for convenience:
\code{.cpp}
using Ternary = Ternary_<char>;
\endcode
*/
template <typename Tb>
class Ternary_ : public cv::Mat_<Tb> {
public:
  using cv::Mat_<Tb>::Mat_;

  /*!
  \brief Insert an event, writing POSITIVE or NEGATIVE at (e.x, e.y) based on e.p.
  \param e Event to insert
  \return New pixel value
  */
  template <typename T>
  inline Tb insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  /*!
  \brief Write polarity p at (x, y) without constructing an Event_.
  \param x Spatial coordinate x
  \param y Spatial coordinate y
  \param p Polarity (true → POSITIVE, false → NEGATIVE)
  \return New pixel value
  */
  template <typename T>
  inline Tb emplace(const T x, const T y, const bool p) {
    return set(x, y, p);
  }

  /*!
  \brief Reset all pixels to ZERO.
  */
  inline void clear() {
    detail::clearZero(*this);
  }

  static constexpr Tb POSITIVE = std::numeric_limits<Tb>::max(); /*!< Value for positive-polarity events */
  static constexpr Tb ZERO = static_cast<Tb>(0);                 /*!< Value after clear() */
  static constexpr Tb NEGATIVE = std::numeric_limits<Tb>::min(); /*!< Value for negative-polarity events */

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

/*!
\brief Spatial map storing the timestamp of the most recent event at each pixel.

Each insertion overwrites the stored timestamp unconditionally; only the latest timestamp
per pixel is retained. Timestamps are stored as TimeType (float by default).
*/
class Time : public cv::Mat_<TimeType> {
public:
  using cv::Mat_<TimeType>::Mat_;

  /*!
  \brief Insert an event, storing e.t at pixel (e.x, e.y).
  \param e Event to insert
  \return Stored timestamp
  */
  template <typename T>
  inline TimeType insert(const Event_<T> &e) {
    return set(e.x, e.y, e.t);
  }

  /*!
  \brief Store timestamp t at (x, y) without constructing an Event_.
  \param x Spatial coordinate x
  \param y Spatial coordinate y
  \param t Timestamp
  \return Stored timestamp
  */
  template <typename T>
  inline TimeType emplace(const T x, const T y, const TimeType t) {
    return set(x, y, t);
  }

  /*!
  \brief Reset all timestamps to zero.
  */
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

/*!
\brief Spatial map storing the polarity of the most recent event at each pixel.

Each insertion overwrites the stored polarity unconditionally; only the latest polarity
per pixel is retained.
*/
class Polarity : public cv::Mat_<PolarityType> {
public:
  using cv::Mat_<PolarityType>::Mat_;

  /*!
  \brief Insert an event, storing e.p at pixel (e.x, e.y).
  \param e Event to insert
  \return Stored polarity
  */
  template <typename T>
  inline PolarityType insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  /*!
  \brief Store polarity p at (x, y) without constructing an Event_.
  \param x Spatial coordinate x
  \param y Spatial coordinate y
  \param p Polarity
  \return Stored polarity
  */
  template <typename T>
  inline PolarityType emplace(const T x, const T y, const PolarityType p) {
    return set(x, y, p);
  }

  /*!
  \brief Reset all polarities to zero.
  */
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

/*!
\brief Spatial map accumulating signed event counts per pixel.

Each positive-polarity event increments the pixel counter by +1; each negative-polarity
event decrements it by -1. Counters are stored as CounterType (int16_t by default).
*/
class Counter : public cv::Mat_<CounterType> {
public:
  using cv::Mat_<CounterType>::Mat_;

  /*!
  \brief Insert an event, incrementing (p=true) or decrementing (p=false) the counter at (e.x, e.y).
  \param e Event to insert
  \return Updated counter value
  */
  template <typename T>
  inline CounterType insert(const Event_<T> &e) {
    return set(e.x, e.y, e.p);
  }

  /*!
  \brief Increment or decrement the counter at (x, y) without constructing an Event_.
  \param x Spatial coordinate x
  \param y Spatial coordinate y
  \param p true to increment (+1), false to decrement (-1)
  \return Updated counter value
  */
  template <typename T>
  inline CounterType emplace(const T x, const T y, const bool p) {
    return set(x, y, p);
  }

  /*!
  \brief Reset all counters to zero.
  */
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

#endif // OPENEV_CORE_MATRICES_HPP
