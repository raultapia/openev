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
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <type_traits>
#include <vector>

namespace ev {
/*! \cond INTERNAL */
#define OPENEV_HAS_GETTER_(name)             \
  template <typename C, typename = void>     \
  struct has_##name##_ : std::false_type {}; \
  template <typename C>                      \
  struct has_##name##_<C, std::void_t<decltype(std::declval<const C &>().name())>> : std::true_type {};

OPENEV_HAS_GETTER_(firstTimestamp)
OPENEV_HAS_GETTER_(lastTimestamp)
OPENEV_HAS_GETTER_(sum)
OPENEV_HAS_GETTER_(sumT)
OPENEV_HAS_GETTER_(sumP)
OPENEV_HAS_GETTER_(squares)
OPENEV_HAS_GETTER_(bounds)
OPENEV_HAS_GETTER_(activeCount)
OPENEV_HAS_GETTER_(peakCount)
OPENEV_HAS_GETTER_(sumCLogC)
#undef OPENEV_HAS_GETTER_
/*! \endcond */

/*!
\brief This is an auxiliary class. This class cannot be instanced.

It holds the statistics every event container offers, so that each of them only has to derive from it passing its own type:
\code{.cpp}
template <typename T>
class Vector_ : public std::vector<Event_<T>>, public AbstractContainer_<Vector_<T>, T> { ... };
\endcode
\note Incremental behaviour: Each statistic preferes the getters of its ingredients when the container offers them.
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
    if constexpr(has_firstTimestamp_<Container>::value && has_lastTimestamp_<Container>::value) {
      return self_().lastTimestamp() - self_().firstTimestamp();
    } else {
      return self_().back().t - self_().front().t;
    }
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event.
  \return Event rate
  */
  [[nodiscard]] inline ResultType rate() const {
    const ResultType span = duration();
    return static_cast<ResultType>(self_().size()) / span;
  }

  /*!
  \brief Compute event density as the ratio between the number of events and the number of pixels.
  \param size Size in pixels
  \return Events per square pixel
  */
  [[nodiscard]] inline ResultType density(const cv::Size size) const {
    return static_cast<ResultType>(self_().size()) / (static_cast<ResultType>(size.width) * static_cast<ResultType>(size.height));
  }

  /*!
  \brief Count the pixels with at least one event.
  \return Number of active pixels
  */
  [[nodiscard]] inline std::size_t activePixels() const {
    if constexpr(has_activeCount_<Container>::value) {
      return self_().activeCount();
    } else {
      std::size_t active = 0;
      forEachPixelCount_(pixels_(), [&active](const uint32_t) { active++; });
      return active;
    }
  }

  /*!
  \brief Compute fill ratio as the fraction of pixels with at least one event.
  \param size Size in pixels
  \return Fraction of active pixels, between 0 and 1
  */
  [[nodiscard]] inline ResultType fillRatio(const cv::Size size) const {
    return static_cast<ResultType>(activePixels()) / (static_cast<ResultType>(size.width) * static_cast<ResultType>(size.height));
  }

  /*!
  \brief Compute polarity ratio as the fraction of events with the given polarity.
  \param p Polarity, POSITIVE or NEGATIVE
  \return Fraction of events with that polarity, between 0 and 1
  */
  [[nodiscard]] inline ResultType polarityRatio(const PolarityType p) const {
    if constexpr(has_sumP_<Container>::value) {
      const ResultType positive = self_().sumP() / static_cast<ResultType>(self_().size());
      return p ? positive : 1 - positive;
    } else {
      std::size_t matching = 0;
      for(const Event_<T> &e : self_()) {
        matching += e.p == p ? 1 : 0;
      }
      return static_cast<ResultType>(matching) / static_cast<ResultType>(self_().size());
    }
  }

  /*!
  \brief Compute the mean of the events.
  \return An Eventd object containing the mean values of x, y, t, and p attributes.
  */
  [[nodiscard]] inline Event_<ResultType> mean() const {
    if constexpr(has_sum_<Container>::value && has_sumT_<Container>::value && has_sumP_<Container>::value) {
      const auto n = static_cast<ResultType>(self_().size());
      return {self_().sum().x / n, self_().sum().y / n, self_().sumT() / n, self_().sumP() / n > 0.5};
    } else {
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
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point_<ResultType> meanPoint() const {
    if constexpr(has_sum_<Container>::value) {
      return self_().sum() / static_cast<ResultType>(self_().size());
    } else {
      ResultType x{0};
      ResultType y{0};
      for(const Event_<T> &e : self_()) {
        x += e.x;
        y += e.y;
      }
      const auto n = static_cast<ResultType>(self_().size());
      return {x / n, y / n};
    }
  }

  /*!
  \brief Compute the covariance of the x,y coordinates of the events around meanPoint().
  \return Covariance matrix, with the variances of x and y in the diagonal
  */
  [[nodiscard]] inline cv::Matx<ResultType, 2, 2> covariance() const {
    if constexpr(has_sum_<Container>::value && has_squares_<Container>::value) {
      const auto n = static_cast<ResultType>(self_().size());
      const cv::Point_<ResultType> mean = self_().sum() / n;
      const cv::Vec<ResultType, 3> squares = self_().squares() / n;
      const ResultType xy = squares[2] - mean.x * mean.y;
      return {squares[0] - mean.x * mean.x, xy, xy, squares[1] - mean.y * mean.y};
    } else {
      const cv::Point_<ResultType> mean = meanPoint();
      ResultType xx{0};
      ResultType yy{0};
      ResultType xy{0};
      for(const Event_<T> &e : self_()) {
        const ResultType dx = e.x - mean.x;
        const ResultType dy = e.y - mean.y;
        xx += dx * dx;
        yy += dy * dy;
        xy += dx * dy;
      }
      const auto n = static_cast<ResultType>(self_().size());
      return {xx / n, xy / n, xy / n, yy / n};
    }
  }

  /*!
  \brief Compute the smallest rectangle of pixels containing the events.
  \return Bounding box in pixels
  */
  [[nodiscard]] inline cv::Rect boundingBox() const {
    if constexpr(has_bounds_<Container>::value) {
      return self_().bounds();
    } else {
      return bounds_(self_(), pixel_);
    }
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline ResultType meanTime() const {
    if constexpr(has_sumT_<Container>::value) {
      return self_().sumT() / static_cast<ResultType>(self_().size());
    } else {
      ResultType t{0};
      for(const Event_<T> &e : self_()) {
        t += e.t;
      }
      return t / static_cast<ResultType>(self_().size());
    }
  }

  /*!
  \brief Calculate the midpoint time between the oldest and the newest event.
  \return Midpoint time
  */
  [[nodiscard]] inline ResultType midTime() const {
    if constexpr(has_firstTimestamp_<Container>::value && has_lastTimestamp_<Container>::value) {
      return 0.5 * (self_().firstTimestamp() + self_().lastTimestamp());
    } else {
      return 0.5 * (self_().front().t + self_().back().t);
    }
  }

  /*!
  \brief Find the largest number of events on a single pixel.
  \return Peak count
  */
  [[nodiscard]] inline std::size_t peak() const {
    if constexpr(has_peakCount_<Container>::value) {
      return self_().peakCount();
    } else {
      uint32_t peak = 0;
      forEachPixelCount_(pixels_(), [&peak](const uint32_t count) { peak = std::max(peak, count); });
      return peak;
    }
  }

  /*!
  \brief Compute the Shannon entropy of the spatial distribution of the events.
  \return Entropy in bits
  \note \f$ H = -\sum_i p_i \log_2 p_i \f$, where \f$ p_i \f$ is the fraction of events falling on the i-th pixel, so \f$ 2^H \f$ is the effective number of active pixels.
  */
  [[nodiscard]] inline ResultType entropy() const {
    const auto n = static_cast<ResultType>(self_().size());
    if constexpr(has_sumCLogC_<Container>::value) {
      const ResultType h = std::log2(n) - self_().sumCLogC() / n;
      return h < 0 ? 0 : h;
    } else {
      ResultType h{0};
      forEachPixelCount_(pixels_(), [&h, n](const uint32_t count) {
        const ResultType p = static_cast<ResultType>(count) / n;
        h -= p * std::log2(p);
      });
      return h;
    }
  }

protected:
  /*! \cond INTERNAL */
  [[nodiscard]] inline const Container &self_() const {
    return static_cast<const Container &>(*this);
  }

  [[nodiscard]] inline static cv::Point pixel_(const Event_<T> &e) {
    if constexpr(std::is_floating_point_v<T>) {
      return {static_cast<int>(std::lround(e.x)), static_cast<int>(std::lround(e.y))};
    } else {
      return {static_cast<int>(e.x), static_cast<int>(e.y)};
    }
  }

  [[nodiscard]] inline std::vector<cv::Point> pixels_() const {
    std::vector<cv::Point> pixels;
    pixels.reserve(self_().size());
    for(const Event_<T> &e : self_()) {
      pixels.push_back(pixel_(e));
    }
    return pixels;
  }

  template <typename Range, typename Pixel>
  [[nodiscard]] inline static cv::Rect bounds_(const Range &range, Pixel toPixel) {
    int left = INT_MAX;
    int right = INT_MIN;
    int top = INT_MAX;
    int bottom = INT_MIN;
    for(const auto &item : range) {
      const cv::Point pixel = toPixel(item);
      left = std::min(left, pixel.x);
      right = std::max(right, pixel.x);
      top = std::min(top, pixel.y);
      bottom = std::max(bottom, pixel.y);
    }
    return {left, top, right - left + 1, bottom - top + 1};
  }

  template <typename Fn>
  inline static void forEachPixelCount_(const std::vector<cv::Point> &pixels, Fn fn) {
    constexpr uint64_t MAX_AREA_PER_EVENT = 32;
    const cv::Rect bounds = bounds_(pixels, [](const cv::Point &pixel) { return pixel; });
    const auto width = static_cast<uint64_t>(bounds.width);
    const auto height = static_cast<uint64_t>(bounds.height);

    if(width <= (MAX_AREA_PER_EVENT * pixels.size()) / height) {
      std::vector<uint32_t> counts(width * height, 0);
      for(const cv::Point &pixel : pixels) {
        counts[(static_cast<uint64_t>(pixel.y - bounds.y) * width) + static_cast<uint64_t>(pixel.x - bounds.x)]++;
      }
      for(const uint32_t count : counts) {
        if(count > 0) {
          fn(count);
        }
      }
    } else {
      std::vector<uint64_t> keys;
      keys.reserve(pixels.size());
      for(const cv::Point &pixel : pixels) {
        keys.push_back((static_cast<uint64_t>(static_cast<uint32_t>(pixel.y)) << 32U) | static_cast<uint32_t>(pixel.x));
      }
      std::sort(keys.begin(), keys.end());
      uint32_t run = 1;
      for(std::size_t i = 1; i <= keys.size(); i++) {
        if(i == keys.size() || keys[i] != keys[i - 1]) {
          fn(run);
          run = 1;
        } else {
          run++;
        }
      }
    }
  }
  /*! \endcond */
};
} // namespace ev

#endif // OPENEV_CONTAINERS_ABSTRACT_CONTAINER_HPP
