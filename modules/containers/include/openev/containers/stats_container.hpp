/*!
\file stats_container.hpp
\brief Running statistics of an event stream without storing the events.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_STATS_CONTAINER_HPP
#define OPENEV_CONTAINERS_STATS_CONTAINER_HPP

#include "openev/core/stats.hpp"
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
[[maybe_unused]] constexpr bool USING_STATS_CONTAINER_HPP = true;

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
\brief This class keeps the ingredients of the statistics of Stats_ up to date without storing the events.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using StatsContaineri = StatsContainer_<int>;
using StatsContainerl = StatsContainer_<long>;
using StatsContainerf = StatsContainer_<float>;
using StatsContainerd = StatsContainer_<double>;
using StatsContainer  = StatsContaineri;
\endcode
*/
template <typename T>
class StatsContainer_ : public Stats_<StatsContainer_<T>> {
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

    if(a_.n == 0) {
      a_.first = e.t;
    }
    a_.last = e.t;
    a_.n++;
    a_.sum += cv::Point2d(e.x, e.y);
    a_.st += e.t;
    a_.sp += e.p;
    a_.squares += cv::Vec3d(static_cast<double>(e.x) * static_cast<double>(e.x), static_cast<double>(e.y) * static_cast<double>(e.y), static_cast<double>(e.x) * static_cast<double>(e.y));
    a_.bounds |= cv::Rect(x, y, 1, 1);

    if(static_cast<unsigned>(x - box_.x) >= static_cast<unsigned>(box_.width) || static_cast<unsigned>(y - box_.y) >= static_cast<unsigned>(box_.height)) {
      grow_(x, y);
    }
    uint32_t &c = counts_[(static_cast<std::size_t>(y - box_.y) * static_cast<std::size_t>(box_.width)) + static_cast<std::size_t>(x - box_.x)];
    a_.active += static_cast<std::size_t>(c == 0);
    a_.sumCLogC += stats::cLog2C(c + 1) - stats::cLog2C(c);
    c++;
    a_.peak = std::max(a_.peak, c);
  }

  /*!
  \brief Number of events accounted for.
  \return Event count
  */
  [[nodiscard]] inline std::size_t size() const {
    return a_.n;
  }

  /*!
  \brief Check whether no event was accounted for.
  \return True if empty
  */
  [[nodiscard]] inline bool empty() const {
    return a_.n == 0;
  }

  [[nodiscard]] inline TimeType firstTimestamp() const { return a_.first; }  /*!< Timestamp of the first event */
  [[nodiscard]] inline TimeType lastTimestamp() const { return a_.last; }    /*!< Timestamp of the last event */
  [[nodiscard]] inline cv::Point2d sum() const { return a_.sum; }            /*!< Sum of x and y */
  [[nodiscard]] inline double sumT() const { return a_.st; }                 /*!< Sum of t */
  [[nodiscard]] inline double sumP() const { return a_.sp; }                 /*!< Sum of p */
  [[nodiscard]] inline cv::Vec3d squares() const { return a_.squares; }      /*!< Sums of x^2, y^2 and x*y */
  [[nodiscard]] inline cv::Rect bounds() const { return a_.bounds; }         /*!< Smallest rectangle of pixels holding the events */
  [[nodiscard]] inline std::size_t activeCount() const { return a_.active; } /*!< Number of pixels with events */
  [[nodiscard]] inline std::size_t peakCount() const { return a_.peak; }     /*!< Largest number of events on a pixel */
  [[nodiscard]] inline double sumCLogC() const { return a_.sumCLogC; }       /*!< Sum of c*log2(c) over the pixel counts c */

  /*!
  \brief Forget every event. The per-pixel count keeps its extent, so accounting for a similar stream again does not grow it.
  */
  inline void clear() {
    a_ = Accumulators_();
    std::fill(counts_.begin(), counts_.end(), 0);
  }

private:
  cv::Rect box_;
  std::vector<uint32_t> counts_;
  struct Accumulators_ {
    std::size_t n{0};
    TimeType first{0};
    TimeType last{0};
    cv::Point2d sum;
    double st{0};
    double sp{0};
    cv::Vec3d squares;
    cv::Rect bounds;
    std::size_t active{0};
    uint32_t peak{0};
    double sumCLogC{0};
  };
  Accumulators_ a_;

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

using StatsContaineri = StatsContainer_<int>;    /*!< Alias for StatsContainer_ using int */
using StatsContainerl = StatsContainer_<long>;   /*!< Alias for StatsContainer_ using long */
using StatsContainerf = StatsContainer_<float>;  /*!< Alias for StatsContainer_ using float */
using StatsContainerd = StatsContainer_<double>; /*!< Alias for StatsContainer_ using double */
using StatsContainer = StatsContaineri;          /*!< Alias for StatsContainer_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_STATS_CONTAINER_HPP
