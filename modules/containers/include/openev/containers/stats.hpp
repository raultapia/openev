/*!
\file stats.hpp
\brief Running statistics of an event stream without storing the events.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_STATS_HPP
#define OPENEV_CONTAINERS_STATS_HPP

#include "openev/containers/abstract-container.hpp"
#include "openev/core/types.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <type_traits>
#include <vector>

namespace ev {
[[maybe_unused]] constexpr bool USING_STATS_HPP = true;

/*! \cond INTERNAL */
namespace stats {
constexpr std::size_t TABULATED = 4096;
constexpr int GROWTH_NUM = 3;
constexpr int GROWTH_DEN = 2;

inline double cLog2C(const uint32_t c) {
  static const std::array<double, TABULATED> table = [] {
    std::array<double, TABULATED> t{};
    for(std::size_t i = 1; i < TABULATED; i++) {
      t[i] = static_cast<double>(i) * std::log2(static_cast<double>(i));
    }
    return t;
  }();
  return c < TABULATED ? table[c] : static_cast<double>(c) * std::log2(static_cast<double>(c));
}
} // namespace stats
/*! \endcond */

/*!
\brief This class keeps the ingredients of the statistics of AbstractContainer_ up to date without storing the events.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using Statsi = Stats_<int>;
using Statsl = Stats_<long>;
using Statsf = Stats_<float>;
using Statsd = Stats_<double>;
using Stats  = Statsi;
\endcode
*/
template <typename T>
class Stats_ : public AbstractContainer_<Stats_<T>, T> {
public:
  using value_type = Event_<T>;

  /*!
  \brief Account for an event.
  \param e Event
  */
  inline void push(const Event_<T> &e) {
    int x = 0;
    int y = 0;
    if constexpr(std::is_floating_point_v<T>) {
      x = round_(e.x);
      y = round_(e.y);
    } else {
      x = static_cast<int>(e.x);
      y = static_cast<int>(e.y);
    }

    if(n_ == 0) {
      first_ = e.t;
    }
    last_ = e.t;
    n_++;
    sum_ += cv::Point2d(e.x, e.y);
    st_ += e.t;
    sp_ += e.p;
    squares_ += cv::Vec3d(static_cast<double>(e.x) * static_cast<double>(e.x), static_cast<double>(e.y) * static_cast<double>(e.y), static_cast<double>(e.x) * static_cast<double>(e.y));
    bounds_ |= cv::Rect(x, y, 1, 1);

    if(static_cast<unsigned>(x - box_.x) >= static_cast<unsigned>(box_.width) || static_cast<unsigned>(y - box_.y) >= static_cast<unsigned>(box_.height)) {
      grow_(x, y);
    }
    uint32_t &c = counts_[(static_cast<std::size_t>(y - box_.y) * static_cast<std::size_t>(box_.width)) + static_cast<std::size_t>(x - box_.x)];
    active_ += static_cast<std::size_t>(c == 0);
    sumCLogC_ += stats::cLog2C(c + 1) - stats::cLog2C(c);
    c++;
    peak_ = std::max(peak_, c);
  }

  /*!
  \brief Number of events accounted for.
  \return Event count
  */
  [[nodiscard]] inline std::size_t size() const {
    return n_;
  }

  /*!
  \brief Check whether no event was accounted for.
  \return True if empty
  */
  [[nodiscard]] inline bool empty() const {
    return n_ == 0;
  }

  [[nodiscard]] inline TimeType firstTimestamp() const { return first_; }  /*!< Timestamp of the first event */
  [[nodiscard]] inline TimeType lastTimestamp() const { return last_; }    /*!< Timestamp of the last event */
  [[nodiscard]] inline cv::Point2d sum() const { return sum_; }            /*!< Sum of x and y */
  [[nodiscard]] inline double sumT() const { return st_; }                 /*!< Sum of t */
  [[nodiscard]] inline double sumP() const { return sp_; }                 /*!< Sum of p */
  [[nodiscard]] inline cv::Vec3d squares() const { return squares_; }      /*!< Sums of x^2, y^2 and x*y */
  [[nodiscard]] inline cv::Rect bounds() const { return bounds_; }         /*!< Smallest rectangle of pixels holding the events */
  [[nodiscard]] inline std::size_t activeCount() const { return active_; } /*!< Number of pixels with events */
  [[nodiscard]] inline std::size_t peakCount() const { return peak_; }     /*!< Largest number of events on a pixel */
  [[nodiscard]] inline double sumCLogC() const { return sumCLogC_; }       /*!< Sum of c*log2(c) over the pixel counts c */

  /*!
  \brief Forget every event. The per-pixel count keeps its extent, so accounting for a similar stream again does not grow it.
  */
  inline void clear() {
    n_ = 0;
    first_ = 0;
    last_ = 0;
    sum_ = cv::Point2d();
    st_ = 0;
    sp_ = 0;
    squares_ = cv::Vec3d();
    bounds_ = cv::Rect();
    active_ = 0;
    peak_ = 0;
    sumCLogC_ = 0;
    std::fill(counts_.begin(), counts_.end(), 0);
  }

private:
  cv::Rect box_;
  std::vector<uint32_t> counts_;
  std::size_t n_{0};
  TimeType first_{0};
  TimeType last_{0};
  cv::Point2d sum_;
  double st_{0};
  double sp_{0};
  cv::Vec3d squares_;
  cv::Rect bounds_;
  std::size_t active_{0};
  uint32_t peak_{0};
  double sumCLogC_{0};

  void grow_(const int x, const int y) {
    if(box_.empty()) {
      box_ = {x, y, 1, 1};
      counts_.assign(1, 0);
      return;
    }
    int x0 = std::min(box_.x, x);
    int y0 = std::min(box_.y, y);
    const int x1 = std::max(box_.x + box_.width, x + 1);
    const int y1 = std::max(box_.y + box_.height, y + 1);
    int width = x1 - x0;
    int height = y1 - y0;
    if(width > box_.width) {
      width = std::max(width, (box_.width * stats::GROWTH_NUM) / stats::GROWTH_DEN);
    }
    if(height > box_.height) {
      height = std::max(height, (box_.height * stats::GROWTH_NUM) / stats::GROWTH_DEN);
    }
    if(x < box_.x) {
      x0 = x1 - width;
    }
    if(y < box_.y) {
      y0 = y1 - height;
    }

    std::vector<uint32_t> next(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 0);
    for(int r = 0; r < box_.height; r++) {
      std::copy_n(counts_.begin() + static_cast<std::ptrdiff_t>(r) * box_.width, box_.width, next.begin() + (static_cast<std::ptrdiff_t>(r + box_.y - y0) * width) + (box_.x - x0));
    }
    counts_.swap(next);
    box_ = {x0, y0, width, height};
  }
};

using Statsi = Stats_<int>;    /*!< Alias for Stats_ using int */
using Statsl = Stats_<long>;   /*!< Alias for Stats_ using long */
using Statsf = Stats_<float>;  /*!< Alias for Stats_ using float */
using Statsd = Stats_<double>; /*!< Alias for Stats_ using double */
using Stats = Statsi;          /*!< Alias for Stats_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_STATS_HPP
