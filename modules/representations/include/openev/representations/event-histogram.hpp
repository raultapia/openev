/*!
\file event-histogram.hpp
\brief 2D histogram of events.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_HPP
#define OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_HPP

#include "openev/core/matrices.hpp"
#include "openev/representations/abstract-representation.hpp"
#include "openev/representations/event-image.hpp"
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <utility>

namespace ev {
/*! \cond INTERNAL */
template <typename T>
class Event_;
/*! \endcond */

/*!
\brief This class extends ev::EventImage_<T> for event 2D histograms.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using EventHistogram1b = EventHistogram_<uchar>;
using EventHistogram3b = EventHistogram_<cv::Vec3b>;
using EventHistogram1s = EventHistogram_<short>;
using EventHistogram3s = EventHistogram_<cv::Vec3s>;
using EventHistogram1w = EventHistogram_<ushort>;
using EventHistogram3w = EventHistogram_<cv::Vec3w>;
using EventHistogram1i = EventHistogram_<int>;
using EventHistogram3i = EventHistogram_<cv::Vec3i>;
using EventHistogram1f = EventHistogram_<float>;
using EventHistogram3f = EventHistogram_<cv::Vec3f>;
using EventHistogram1d = EventHistogram_<double>;
using EventHistogram3d = EventHistogram_<cv::Vec3d>;
using EventHistogram1 = EventHistogram1b;
using EventHistogram3 = EventHistogram3b;
using EventHistogram = EventHistogram1;
\endcode
*/
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE, typename E = int>
class EventHistogram_ : public EventImage_<T, Options, E> {
public:
  template <typename... Args>
  explicit EventHistogram_(Args &&...args) : EventImage_<T, Options, E>(std::forward<Args>(args)...) {
    EventImage_<T, Options, E>::clear();
  }

  Mat::Counter counter{cv::Mat_<int>(EventImage_<T, Options, E>::size())}; /*!< Event counter */

  /*!
  Event histogram matrix is generated from counter matrix.
  \brief Render event histogram matrix.
  */
  cv::Mat &render();

private:
  void clear_() override;
  void clear_(const cv::Mat &background) override;
  bool insert_(const Event_<E> &e) override;
  int peak_{0};
};
using EventHistogram1b = EventHistogram_<uchar>;     /*!< Alias for EventHistogram_ using uchar */
using EventHistogram3b = EventHistogram_<cv::Vec3b>; /*!< Alias for EventHistogram_ using cv::Vec3b */
using EventHistogram1s = EventHistogram_<short>;     /*!< Alias for EventHistogram_ using short */
using EventHistogram3s = EventHistogram_<cv::Vec3s>; /*!< Alias for EventHistogram_ using cv::Vec3s */
using EventHistogram1w = EventHistogram_<ushort>;    /*!< Alias for EventHistogram_ using ushort */
using EventHistogram3w = EventHistogram_<cv::Vec3w>; /*!< Alias for EventHistogram_ using cv::Vec3w */
using EventHistogram1i = EventHistogram_<int>;       /*!< Alias for EventHistogram_ using int */
using EventHistogram3i = EventHistogram_<cv::Vec3i>; /*!< Alias for EventHistogram_ using cv::Vec3i */
using EventHistogram1f = EventHistogram_<float>;     /*!< Alias for EventHistogram_ using float */
using EventHistogram3f = EventHistogram_<cv::Vec3f>; /*!< Alias for EventHistogram_ using cv::Vec3f */
using EventHistogram1d = EventHistogram_<double>;    /*!< Alias for EventHistogram_ using double */
using EventHistogram3d = EventHistogram_<cv::Vec3d>; /*!< Alias for EventHistogram_ using cv::Vec3d */
using EventHistogram1 = EventHistogram1b;            /*!< Alias for EventHistogram_ using uchar */
using EventHistogram3 = EventHistogram3b;            /*!< Alias for EventHistogram_ using cv::Vec3b */
using EventHistogram = EventHistogram1;              /*!< Alias for EventHistogram_ using uchar */
} // namespace ev

/*! \cond INTERNAL */
#include "openev/representations/event-histogram.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATIONS_EVENT_HISTOGRAM_HPP
