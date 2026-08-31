/*!
\file abstract-container.hpp
\brief Statistics shared by all the event containers.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_ABSTRACT_CONTAINER_HPP
#define OPENEV_CONTAINERS_ABSTRACT_CONTAINER_HPP

#include "openev/core/types.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <opencv2/core/base.hpp>
#include <opencv2/core/types.hpp>
#include <type_traits>
#include <unordered_map>

namespace ev {
/*!
\brief This is an auxiliary class. This class cannot be instanced.

It holds the statistics every event container offers, so that each of them only has to derive from it passing its own type:
\code{.cpp}
template <typename T>
class Vector_ : public std::vector<Event_<T>>, public AbstractContainer_<Vector_<T>, T> { ... };
\endcode
\note duration(), rate(), and midTime() only need the first and the last event, so they work on containers that cannot be traversed,
such as the ones built on std::queue. Those containers hide mean(), meanPoint(), meanTime(), and entropy() with their own version.
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
    return {add_([](const Event_<T> &e) { return e.x; }), add_([](const Event_<T> &e) { return e.y; }), add_([](const Event_<T> &e) { return e.t; }), add_([](const Event_<T> &e) { return e.p; }) > 0.5};
  }

  /*!
  \brief Compute the mean x,y point of the events.
  \return Mean point
  */
  [[nodiscard]] inline cv::Point_<ResultType> meanPoint() const {
    check_("meanPoint");
    return {add_([](const Event_<T> &e) { return e.x; }), add_([](const Event_<T> &e) { return e.y; })};
  }

  /*!
  \brief Compute the mean time of the events.
  \return Mean time
  */
  [[nodiscard]] inline ResultType meanTime() const {
    check_("meanTime");
    return add_([](const Event_<T> &e) { return e.t; });
  }

  /*!
  \brief Compute the Shannon entropy of the spatial distribution of the events.
  \return Entropy in bits
  \note \f$ H = -\sum_i p_i \log_2 p_i \f$, where \f$ p_i \f$ is the fraction of events falling on the i-th pixel, so \f$ 2^H \f$ is the effective number of active pixels.
  */
  [[nodiscard]] inline ResultType entropy() const {
    check_("entropy");
    std::unordered_map<uint64_t, std::size_t> histogram;
    for(const Event_<T> &e : self_()) {
      histogram[pixel_(e)]++;
    }
    return entropy_(histogram, static_cast<ResultType>(self_().size()));
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

  [[nodiscard]] inline static uint64_t pixel_(const Event_<T> &e) {
    if constexpr(std::is_floating_point_v<T>) {
      return (static_cast<uint64_t>(static_cast<uint32_t>(std::lround(e.y))) << 32U) | static_cast<uint32_t>(std::lround(e.x));
    } else {
      return (static_cast<uint64_t>(static_cast<uint32_t>(e.y)) << 32U) | static_cast<uint32_t>(e.x);
    }
  }

  [[nodiscard]] inline static ResultType entropy_(const std::unordered_map<uint64_t, std::size_t> &histogram, const ResultType n) {
    ResultType h{0};
    for(const auto &[pixel, count] : histogram) {
      const ResultType p = static_cast<ResultType>(count) / n;
      h -= p * std::log2(p);
    }
    return h;
  }

  template <typename F>
  [[nodiscard]] inline ResultType add_(F &&field) const {
    return std::accumulate(self_().begin(), self_().end(), ResultType{0}, [&field](const ResultType sum, const Event_<T> &e) { return sum + field(e); }) / static_cast<ResultType>(self_().size());
  }
  /*! \endcond */
};
} // namespace ev

#endif // OPENEV_CONTAINERS_ABSTRACT_CONTAINER_HPP
