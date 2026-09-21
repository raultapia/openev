/*!
\file space-time.hpp
\brief Perspective view of the events in the x-y-t space-time volume.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_SPACE_TIME_HPP
#define OPENEV_REPRESENTATIONS_SPACE_TIME_HPP

#include "openev/representations/abstract-representation.hpp"
#include "openev/representations/event-image.hpp"
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/viz/types.hpp>
#include <vector>

namespace ev {
[[maybe_unused]] constexpr bool USING_SPACE_TIME_HPP = true;

/*! \cond INTERNAL */
template <typename T>
class Event_;
/*! \endcond */

/*!
\brief This class extends ev::EventImage_<T> to draw the events as a cloud in the x-y-t volume seen in perspective.

A background given to clear() is an image in sensor coordinates. It is drawn in perspective on the plane of the oldest events,
behind the cloud, and follows the view.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using SpaceTime1b = SpaceTime_<uchar>;
using SpaceTime3b = SpaceTime_<cv::Vec3b>;
using SpaceTime1s = SpaceTime_<short>;
using SpaceTime3s = SpaceTime_<cv::Vec3s>;
using SpaceTime1w = SpaceTime_<ushort>;
using SpaceTime3w = SpaceTime_<cv::Vec3w>;
using SpaceTime1i = SpaceTime_<int>;
using SpaceTime3i = SpaceTime_<cv::Vec3i>;
using SpaceTime1f = SpaceTime_<float>;
using SpaceTime3f = SpaceTime_<cv::Vec3f>;
using SpaceTime1d = SpaceTime_<double>;
using SpaceTime3d = SpaceTime_<cv::Vec3d>;
using SpaceTime1 = SpaceTime1b;
using SpaceTime3 = SpaceTime3b;
using SpaceTime = SpaceTime1;
\endcode
*/
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE, typename E = int>
class SpaceTime_ : public EventImage_<T, Options, E> {
public:
  static constexpr double DEFAULT_YAW = 30.0;
  static constexpr double DEFAULT_PITCH = 20.0;
  static constexpr double DEFAULT_DEPTH = 1.0;
  static constexpr double DISTANCE = 2.5;
  static constexpr int MARGIN = 2;
  static constexpr double LABEL_SCALE = 0.35;
  static constexpr double DEFAULT_AXES_LENGTH = 0.25;
  static constexpr double DEFAULT_FADE = 0.8;

  /*!
  \brief Construct the canvas.
  \param canvas Canvas size in pixels
  \param sensor Sensor size in pixels, the canvas size if empty
  */
  explicit SpaceTime_(const cv::Size canvas, const cv::Size sensor = cv::Size());

  /*!
  \brief Set the view angles.
  \param yaw Rotation around the vertical axis in degrees, positive turns the right side of the sensor away
  \param pitch Rotation around the horizontal axis in degrees, positive shows the sensor from above
  */
  void setView(const double yaw, const double pitch);

  /*!
  \brief Set the depth of the box, that is, how long the time axis is drawn.
  \param depth Depth relative to the sensor width
  */
  void setDepth(const double depth);

  /*!
  \brief Draw the x, y and t axes from the sensor origin, labelled at their ends.
  \param state True to draw them
  \param length Length of each axis as a fraction of its side of the box
  */
  inline void showAxes(const bool state, const double length = DEFAULT_AXES_LENGTH) {
    axes_ = state;
    axesLength_ = std::clamp(length, 0.0, 1.0);
  }

  /*!
  \brief Fade the events towards the background as they recede, so the depth shows.
  \param state True to fade them
  \param strength How transparent the events on the back face are, from 0, opaque, to 1, invisible
  */
  inline void fadeWithDepth(const bool state, const double strength = DEFAULT_FADE) {
    fade_ = state;
    fadeStrength_ = std::clamp(strength, 0.0, 1.0);
  }

  /*!
  \brief Set the value the axes are drawn with.
  \param value Axes value
  */
  inline void setAxesValue(const typename TypeHelper<T>::Type &value) {
    axesValue_ = value;
  }

  /*!
  \brief Set the colour the axes are drawn with. For more information, please refer <a href="https://docs.opencv.org/master/d4/dba/classcv_1_1viz_1_1Color.html">here</a>.
  \param color Axes colour
  */
  inline void setAxesColor(const cv::viz::Color &color) {
    axesValue_ = TypeHelper<T>::convert(color);
  }

  /*!
  \brief Yaw of the view.
  \return Yaw in degrees
  */
  [[nodiscard]] inline double yaw() const {
    return yaw_;
  }

  /*!
  \brief Pitch of the view.
  \return Pitch in degrees
  */
  [[nodiscard]] inline double pitch() const {
    return pitch_;
  }

  /*!
  \brief Sensor size.
  \return Sensor size in pixels
  */
  [[nodiscard]] inline cv::Size sensorSize() const {
    return sensor_;
  }

  /*!
  \brief Project an event onto the canvas with the current view.
  \param e Event
  \param age Age of the event between 0, the front face, and 1, the back face
  \return Canvas coordinates
  */
  [[nodiscard]] cv::Point2d project(const Event_<E> &e, const double age) const;

  /*!
  The canvas is restored, the axes are drawn if enabled, and the events inserted since the last clear() are drawn from the oldest to the newest.
  \brief Render the space-time view.
  */
  cv::Mat &render();

private:
  cv::Size sensor_;
  double yaw_{DEFAULT_YAW};
  double pitch_{DEFAULT_PITCH};
  double depth_{DEFAULT_DEPTH};
  cv::Matx33d rotation_;
  double scale_{1};
  cv::Point2d center_;
  std::vector<Event_<E>> events_;
  cv::Mat frame_;
  cv::Mat backdrop_;
  bool axes_{false};
  double axesLength_{DEFAULT_AXES_LENGTH};
  bool fade_{false};
  double fadeStrength_{DEFAULT_FADE};
  typename TypeHelper<T>::Type axesValue_ = TypeHelper<T>::convert(cv::viz::Color::white());

  void fit_();
  void warp_();
  void drawAxes_();
  [[nodiscard]] typename TypeHelper<T>::Type faded_(const typename TypeHelper<T>::Type &value, const cv::Point &pixel, const double age) const;
  [[nodiscard]] cv::Point2d corner_(const double x, const double y, const double age) const;
  [[nodiscard]] cv::Point2d project_(const cv::Point3d &point) const;

  [[nodiscard]] cv::Size frameSize_() const override {
    return sensor_;
  }

  void clear_() override;
  void clear_(const cv::Mat &background) override;
  bool insert_(const Event_<E> &e) override;
};
using SpaceTime1b = SpaceTime_<uchar>;     /*!< Alias for SpaceTime_ using uchar */
using SpaceTime3b = SpaceTime_<cv::Vec3b>; /*!< Alias for SpaceTime_ using cv::Vec3b */
using SpaceTime1s = SpaceTime_<short>;     /*!< Alias for SpaceTime_ using short */
using SpaceTime3s = SpaceTime_<cv::Vec3s>; /*!< Alias for SpaceTime_ using cv::Vec3s */
using SpaceTime1w = SpaceTime_<ushort>;    /*!< Alias for SpaceTime_ using ushort */
using SpaceTime3w = SpaceTime_<cv::Vec3w>; /*!< Alias for SpaceTime_ using cv::Vec3w */
using SpaceTime1i = SpaceTime_<int>;       /*!< Alias for SpaceTime_ using int */
using SpaceTime3i = SpaceTime_<cv::Vec3i>; /*!< Alias for SpaceTime_ using cv::Vec3i */
using SpaceTime1f = SpaceTime_<float>;     /*!< Alias for SpaceTime_ using float */
using SpaceTime3f = SpaceTime_<cv::Vec3f>; /*!< Alias for SpaceTime_ using cv::Vec3f */
using SpaceTime1d = SpaceTime_<double>;    /*!< Alias for SpaceTime_ using double */
using SpaceTime3d = SpaceTime_<cv::Vec3d>; /*!< Alias for SpaceTime_ using cv::Vec3d */
using SpaceTime1 = SpaceTime1b;            /*!< Alias for SpaceTime_ using uchar */
using SpaceTime3 = SpaceTime3b;            /*!< Alias for SpaceTime_ using cv::Vec3b */
using SpaceTime = SpaceTime1;              /*!< Alias for SpaceTime_ using uchar */
} // namespace ev

/*! \cond INTERNAL */
#include "openev/representations/space-time.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATIONS_SPACE_TIME_HPP
