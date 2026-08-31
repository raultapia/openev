/*!
\file filtering.hpp
\brief Noise filters for event streams.
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_FILTERING_HPP
#define OPENEV_EVPROC_FILTERING_HPP

#include "openev/core/matrices.hpp"
#include "openev/core/types.hpp"
#include <opencv2/core/types.hpp>

namespace ev {

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

} // namespace ev

#endif // OPENEV_EVPROC_FILTERING_HPP
