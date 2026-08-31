/*!
\file abstract-container.hpp
\brief Statistics shared by all the event containers.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_ABSTRACT_CONTAINER_HPP
#define OPENEV_CONTAINERS_ABSTRACT_CONTAINER_HPP

#include "openev/core/types.hpp"
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <opencv2/core/base.hpp>
#include <opencv2/core/types.hpp>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ev {
/*!
\brief This is an auxiliary class. This class cannot be instanced.

It holds the statistics every event container offers, so that each of them only has to derive from it passing its own type:
\code{.cpp}
template <typename T>
class Vector_ : public std::vector<Event_<T>>, public AbstractContainer_<Vector_<T>, T> { ... };
\endcode
*/
template <typename Container, typename T>
class AbstractContainer_ {
public:
  using ResultType = TimeType;

  /*!
  \brief Time difference between the last and the first event.
  \return Time difference
  */
  [[nodiscard]] inline ResultType duration() const {
    check_("duration");
    return self_().back().t - self_().front().t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline ResultType rate() const {
    check_("rate");
    const ResultType span = duration();
    if(span == 0) {
      CV_Error(cv::Error::StsDivByZero, "ev::AbstractContainer_::rate: the events span no time.");
    }
    return static_cast<ResultType>(self_().size()) / span;
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time
  */
  [[nodiscard]] inline ResultType midTime() const {
    check_("midTime");
    return 0.5 * (self_().front().t + self_().back().t);
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] inline Event_<ResultType> mean() const {
    check_("mean");
    ResultType x{0};
    ResultType y{0};
    ResultType t{0};
    ResultType p{0};
    for(const Event_<T> &e : self_()) {
      x += e.x;
      y += e.y;
      t += e.t;
      p += e.p;
    }
    const auto n = static_cast<ResultType>(self_().size());
    return {x / n, y / n, t / n, p / n > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point_<ResultType> meanPoint() const {
    check_("meanPoint");
    ResultType x{0};
    ResultType y{0};
    for(const Event_<T> &e : self_()) {
      x += e.x;
      y += e.y;
    }
    const auto n = static_cast<ResultType>(self_().size());
    return {x / n, y / n};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline ResultType meanTime() const {
    check_("meanTime");
    ResultType t{0};
    for(const Event_<T> &e : self_()) {
      t += e.t;
    }
    return t / static_cast<ResultType>(self_().size());
  }

  /*!
  \brief Compute the Shannon entropy of the spatial distribution of the events.
  \return Entropy in bits
  \note \f$ H = -\sum_i p_i \log_2 p_i \f$, where \f$ p_i \f$ is the fraction of events falling on the i-th pixel, so \f$ 2^H \f$ is the effective number of active pixels.
  */
  [[nodiscard]] inline ResultType entropy() const {
    check_("entropy");
    std::vector<cv::Point> pixels;
    pixels.reserve(self_().size());
    for(const Event_<T> &e : self_()) {
      pixels.push_back(pixel_(e));
    }
    return entropy_(pixels);
  }

protected:
  /*! \cond INTERNAL */
  [[nodiscard]] inline const Container &self_() const {
    return static_cast<const Container &>(*this);
  }

  inline void check_(const char *statistic) const {
    if(self_().empty()) {
      CV_Error(cv::Error::StsError, std::string("ev::AbstractContainer_::") + statistic + ": the container is empty.");
    }
  }

  [[nodiscard]] inline static cv::Point pixel_(const Event_<T> &e) {
    if constexpr(std::is_floating_point_v<T>) {
      return {static_cast<int>(std::lround(e.x)), static_cast<int>(std::lround(e.y))};
    } else {
      return {static_cast<int>(e.x), static_cast<int>(e.y)};
    }
  }

  [[nodiscard]] inline static ResultType entropy_(const std::vector<cv::Point> &pixels) {
    constexpr uint64_t MAX_AREA_PER_EVENT = 32;
    const ResultType n = static_cast<ResultType>(pixels.size());

    int left = INT_MAX;
    int right = INT_MIN;
    int top = INT_MAX;
    int bottom = INT_MIN;
    for(const cv::Point &pixel : pixels) {
      left = std::min(left, pixel.x);
      right = std::max(right, pixel.x);
      top = std::min(top, pixel.y);
      bottom = std::max(bottom, pixel.y);
    }

    const auto width = static_cast<uint64_t>(static_cast<int64_t>(right) - static_cast<int64_t>(left) + 1);
    const auto height = static_cast<uint64_t>(static_cast<int64_t>(bottom) - static_cast<int64_t>(top) + 1);

    ResultType h{0};
    if(width <= (MAX_AREA_PER_EVENT * pixels.size()) / height) {
      std::vector<uint32_t> counts(width * height, 0);
      for(const cv::Point &pixel : pixels) {
        counts[(static_cast<uint64_t>(pixel.y - top) * width) + static_cast<uint64_t>(pixel.x - left)]++;
      }
      for(const uint32_t count : counts) {
        if(count > 0) {
          const ResultType p = static_cast<ResultType>(count) / n;
          h -= p * std::log2(p);
        }
      }
    } else {
      std::unordered_map<uint64_t, std::size_t> histogram;
      for(const cv::Point &pixel : pixels) {
        histogram[(static_cast<uint64_t>(static_cast<uint32_t>(pixel.y)) << 32U) | static_cast<uint32_t>(pixel.x)]++;
      }
      for(const auto &[pixel, count] : histogram) {
        const ResultType p = static_cast<ResultType>(count) / n;
        h -= p * std::log2(p);
      }
    }
    return h;
  }
  /*! \endcond */
};
} // namespace ev

#endif // OPENEV_CONTAINERS_ABSTRACT_CONTAINER_HPP
