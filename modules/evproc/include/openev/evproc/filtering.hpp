/*!
\file filtering.hpp
\brief Noise filters for event streams.
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_FILTERING_HPP
#define OPENEV_EVPROC_FILTERING_HPP

#include "openev/core/matrices.hpp"
#include "openev/core/types.hpp"
#include <cstddef>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace ev {
[[maybe_unused]] constexpr bool USING_FILTERING_HPP = true;

/*!
\brief Background activity filter for event noise removal.

Discards isolated events that have no correlated neighbor within a spatial radius and
time window. An event passes if at least one pixel in its neighborhood fired within the
last \p dt time units.

\note T. Delbruck, "Frame-free dynamic digital vision"
*/
class BackgroundActivityFilter {
public:
  /*!
  Constructor.
  \param size Sensor resolution (width x height)
  \param dt Time threshold; events with no neighbor firing within this window are discarded
  \param radius Spatial neighborhood half-size (default 1 -> 3x3 / 8-connected)
  */
  BackgroundActivityFilter(const cv::Size &size, ev::TimeType dt, int radius = 1);

  ~BackgroundActivityFilter() = default;
  BackgroundActivityFilter(const BackgroundActivityFilter &) = default;
  BackgroundActivityFilter(BackgroundActivityFilter &&) noexcept = default;
  BackgroundActivityFilter &operator=(const BackgroundActivityFilter &) = default;
  BackgroundActivityFilter &operator=(BackgroundActivityFilter &&) noexcept = default;

  /*!
  \brief Set the time threshold.
  \param dt Time threshold; events with no neighbor firing within this window are discarded
  */
  inline void setDt(const ev::TimeType dt) {
    dt_ = dt;
  }

  /*!
  \brief Forget every recorded event.
  */
  void reset();

  /*!
  Test and record a single event.
  \param e Event to evaluate
  \return True if the event passes (at least one neighbor fired within \p dt)
  \note The internal timestamp map is updated regardless of whether the event passes.
  */
  [[nodiscard]] bool operator()(const ev::Event &e);

private:
  ev::Mat::Time map_;
  ev::TimeType dt_;
  int radius_;
};

/*!
\brief Refractory period filter for event rate limiting.

Inhibits each pixel for \p dt time units after one of its events is accepted, so no pixel
can fire faster than \f$ 1/dt \f$.

\note T. Delbruck, R. Graca and M. Paluch, "Feedback control of event cameras"
*/
class RefractoryPeriodFilter {
public:
  /*!
  Constructor.
  \param size Sensor resolution (width x height)
  \param dt Inhibition time; events fired before this time has elapsed since the last accepted event of the same pixel are discarded
  */
  RefractoryPeriodFilter(const cv::Size &size, ev::TimeType dt);

  ~RefractoryPeriodFilter() = default;
  RefractoryPeriodFilter(const RefractoryPeriodFilter &) = default;
  RefractoryPeriodFilter(RefractoryPeriodFilter &&) noexcept = default;
  RefractoryPeriodFilter &operator=(const RefractoryPeriodFilter &) = default;
  RefractoryPeriodFilter &operator=(RefractoryPeriodFilter &&) noexcept = default;

  /*!
  \brief Set the inhibition time.
  \param dt Inhibition time; events fired before this time has elapsed since the last accepted event of the same pixel are discarded
  */
  inline void setDt(const ev::TimeType dt) {
    dt_ = dt;
  }

  /*!
  \brief Forget every recorded event.
  */
  void reset();

  /*!
  Test and record a single event.
  \param e Event to evaluate
  \return True if the event passes (its pixel has been idle for at least \p dt)
  \note The internal timestamp map is updated only when the event passes.
  */
  [[nodiscard]] bool operator()(const ev::Event &e);

private:
  ev::Mat::Time map_;
  ev::TimeType dt_;
};

/*!
\brief Hot pixel filter for event noise removal.
*/
class HotPixelFilter {
public:
  static constexpr double DEFAULT_FACTOR = 10.0;
  static constexpr std::size_t DISABLED = 0;

  /*!
  Constructor.
  \param size Sensor resolution (width x height)
  \param window Duration of the time windows over which events are counted
  \param factor Number of times the mean count a pixel has to exceed to be marked as hot; 0 disables this criterion
  \param limit Number of events per window a pixel has to exceed to be marked as hot; 0 disables this criterion
  */
  HotPixelFilter(const cv::Size &size, ev::TimeType window, double factor = DEFAULT_FACTOR, std::size_t limit = DISABLED);

  ~HotPixelFilter() = default;
  HotPixelFilter(const HotPixelFilter &) = default;
  HotPixelFilter(HotPixelFilter &&) noexcept = default;
  HotPixelFilter &operator=(const HotPixelFilter &) = default;
  HotPixelFilter &operator=(HotPixelFilter &&) noexcept = default;

  /*!
  \brief Set the duration of the time windows.
  \param window Duration of the time windows over which events are counted
  */
  inline void setWindow(const ev::TimeType window) {
    window_ = window;
  }

  /*!
  \brief Set the criterion relative to the mean count.
  \param factor Number of times the mean count a pixel has to exceed to be marked as hot; 0 disables this criterion
  */
  inline void setFactor(const double factor) {
    factor_ = factor;
  }

  /*!
  \brief Set the absolute criterion.
  \param limit Number of events per window a pixel has to exceed to be marked as hot; 0 disables this criterion
  */
  inline void setLimit(const std::size_t limit) {
    limit_ = limit;
  }

  /*!
  \brief Get the pixels marked as hot when the last window ended.
  \return Mask with non-zero values on the hot pixels
  */
  [[nodiscard]] inline const cv::Mat_<uchar> &mask() const {
    return mask_;
  }

  /*!
  \brief Forget every recorded event and every hot pixel.
  */
  void reset();

  /*!
  Test and record a single event.
  \param e Event to evaluate
  \return True if the event passes (its pixel is not marked as hot)
  */
  [[nodiscard]] bool operator()(const ev::Event &e);

private:
  void evaluate_();

  cv::Mat_<int> counts_;
  cv::Mat_<uchar> mask_;
  ev::TimeType window_;
  double factor_;
  std::size_t limit_;
  ev::TimeType start_{0};
  bool started_{false};
};

} // namespace ev

#endif // OPENEV_EVPROC_FILTERING_HPP
