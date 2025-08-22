/*!
\file time-surface.hpp
\brief Time surface.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_TIME_SURFACE_HPP
#define OPENEV_REPRESENTATIONS_TIME_SURFACE_HPP

#include "openev/core/matrices.hpp"
#include "openev/representations/abstract-representation.hpp"
#include "openev/representations/event-image.hpp"
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/matx.hpp>
#include <utility>

namespace cv {
/*! \cond INTERNAL */
class Mat;
/*! \endcond */
} // namespace cv

namespace ev {
/*! \cond INTERNAL */
template <typename T>
class Event_;
/*! \endcond */

/*!
\brief This class extends ev::EventImage_<T> for time surfaces.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using TimeSurface1b = TimeSurface_<uchar>;
using TimeSurface2b = TimeSurface_<cv::Vec2b>;
using TimeSurface3b = TimeSurface_<cv::Vec3b>;
using TimeSurface4b = TimeSurface_<cv::Vec4b>;
using TimeSurface1s = TimeSurface_<short>;
using TimeSurface2s = TimeSurface_<cv::Vec2s>;
using TimeSurface3s = TimeSurface_<cv::Vec3s>;
using TimeSurface4s = TimeSurface_<cv::Vec4s>;
using TimeSurface1w = TimeSurface_<ushort>;
using TimeSurface2w = TimeSurface_<cv::Vec2w>;
using TimeSurface3w = TimeSurface_<cv::Vec3w>;
using TimeSurface4w = TimeSurface_<cv::Vec4w>;
using TimeSurface1i = TimeSurface_<int>;
using TimeSurface2i = TimeSurface_<cv::Vec2i>;
using TimeSurface3i = TimeSurface_<cv::Vec3i>;
using TimeSurface4i = TimeSurface_<cv::Vec4i>;
using TimeSurface1f = TimeSurface_<float>;
using TimeSurface2f = TimeSurface_<cv::Vec2f>;
using TimeSurface3f = TimeSurface_<cv::Vec3f>;
using TimeSurface4f = TimeSurface_<cv::Vec4f>;
using TimeSurface1d = TimeSurface_<double>;
using TimeSurface2d = TimeSurface_<cv::Vec2d>;
using TimeSurface3d = TimeSurface_<cv::Vec3d>;
using TimeSurface4d = TimeSurface_<cv::Vec4d>;
using TimeSurface1 = TimeSurface1b;
using TimeSurface3 = TimeSurface3b;
using TimeSurface = TimeSurface1;
\endcode
*/
enum class Kernel { NONE,
                    LINEAR,
                    EXPONENTIAL };
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE, typename E = int>
class TimeSurface_ : public EventImage_<T, Options, E> {
public:
  template <typename... Args>
  explicit TimeSurface_(Args &&...args) : EventImage_<T, Options, E>(std::forward<Args>(args)...) {
    EventImage_<T, Options, E>::clear();
  }

  Mat::Time time{this->size()};         /*!< Time matrix */
  Mat::Polarity polarity{this->size()}; /*!< Polarity matrix */

  /*!
  Timesurface matrix is generated from timestamp and polarity matrices.
  \brief Render timesurface matrix.
  \param kernel Kernel type
  \param tau Time constant
  \see TimeSurface_::Kernel
  */
  cv::Mat &render(const Kernel kernel = Kernel::NONE, const double tau = 0);

private:
  void clear_() override;
  void clear_(const cv::Mat &background) override;
  bool insert_(const Event_<E> &e) override;
};
using TimeSurface1b = TimeSurface_<uchar>;     /*!< Alias for TimeSurface_ using uchar */
using TimeSurface2b = TimeSurface_<cv::Vec2b>; /*!< Alias for TimeSurface_ using cv::Vec2b */
using TimeSurface3b = TimeSurface_<cv::Vec3b>; /*!< Alias for TimeSurface_ using cv::Vec3b */
using TimeSurface4b = TimeSurface_<cv::Vec4b>; /*!< Alias for TimeSurface_ using cv::Vec4b */
using TimeSurface1s = TimeSurface_<short>;     /*!< Alias for TimeSurface_ using short */
using TimeSurface2s = TimeSurface_<cv::Vec2s>; /*!< Alias for TimeSurface_ using cv::Vec2s */
using TimeSurface3s = TimeSurface_<cv::Vec3s>; /*!< Alias for TimeSurface_ using cv::Vec3s */
using TimeSurface4s = TimeSurface_<cv::Vec4s>; /*!< Alias for TimeSurface_ using cv::Vec4s */
using TimeSurface1w = TimeSurface_<ushort>;    /*!< Alias for TimeSurface_ using ushort */
using TimeSurface2w = TimeSurface_<cv::Vec2w>; /*!< Alias for TimeSurface_ using cv::Vec2w */
using TimeSurface3w = TimeSurface_<cv::Vec3w>; /*!< Alias for TimeSurface_ using cv::Vec3w */
using TimeSurface4w = TimeSurface_<cv::Vec4w>; /*!< Alias for TimeSurface_ using cv::Vec4w */
using TimeSurface1i = TimeSurface_<int>;       /*!< Alias for TimeSurface_ using int */
using TimeSurface2i = TimeSurface_<cv::Vec2i>; /*!< Alias for TimeSurface_ using cv::Vec2i */
using TimeSurface3i = TimeSurface_<cv::Vec3i>; /*!< Alias for TimeSurface_ using cv::Vec3i */
using TimeSurface4i = TimeSurface_<cv::Vec4i>; /*!< Alias for TimeSurface_ using cv::Vec4i */
using TimeSurface1f = TimeSurface_<float>;     /*!< Alias for TimeSurface_ using float */
using TimeSurface2f = TimeSurface_<cv::Vec2f>; /*!< Alias for TimeSurface_ using cv::Vec2f */
using TimeSurface3f = TimeSurface_<cv::Vec3f>; /*!< Alias for TimeSurface_ using cv::Vec3f */
using TimeSurface4f = TimeSurface_<cv::Vec4f>; /*!< Alias for TimeSurface_ using cv::Vec4f */
using TimeSurface1d = TimeSurface_<double>;    /*!< Alias for TimeSurface_ using double */
using TimeSurface2d = TimeSurface_<cv::Vec2d>; /*!< Alias for TimeSurface_ using cv::Vec2d */
using TimeSurface3d = TimeSurface_<cv::Vec3d>; /*!< Alias for TimeSurface_ using cv::Vec3d */
using TimeSurface4d = TimeSurface_<cv::Vec4d>; /*!< Alias for TimeSurface_ using cv::Vec4d */
using TimeSurface1 = TimeSurface1b;            /*!< Alias for TimeSurface_ using uchar */
using TimeSurface3 = TimeSurface3b;            /*!< Alias for TimeSurface_ using cv::Vec3b */
using TimeSurface = TimeSurface1;              /*!< Alias for TimeSurface_ using uchar */
} // namespace ev

/*! \cond INTERNAL */
#include "openev/representations/time-surface.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATIONS_TIME_SURFACE_HPP
