/*!
\file point-cloud.hpp
\brief Point cloud of events.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_POINT_CLOUD_HPP
#define OPENEV_REPRESENTATIONS_POINT_CLOUD_HPP

#include "openev/core/types.hpp"
#include "openev/options.hpp"
#include "openev/representations/abstract-representation.hpp"
#include <array>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

#if OE_HAVE_VIZ
#include <opencv2/viz/viz3d.hpp>
#endif

namespace ev {
/*!
\brief This class is used to represent events as point clouds.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using PointCloud1b = PointCloud_<uchar>;
using PointCloud3b = PointCloud_<cv::Vec3b>;
using PointCloud1s = PointCloud_<short>;
using PointCloud3s = PointCloud_<cv::Vec3s>;
using PointCloud1w = PointCloud_<ushort>;
using PointCloud3w = PointCloud_<cv::Vec3w>;
using PointCloud1i = PointCloud_<int>;
using PointCloud3i = PointCloud_<cv::Vec3i>;
using PointCloud1f = PointCloud_<float>;
using PointCloud3f = PointCloud_<cv::Vec3f>;
using PointCloud1d = PointCloud_<double>;
using PointCloud3d = PointCloud_<cv::Vec3d>;
using PointCloud1 = PointCloud1b;
using PointCloud3 = PointCloud3b;
using PointCloud = PointCloud1;
\endcode
*/
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE, typename E = int>
class PointCloud_ : public AbstractRepresentation_<T, Options, E> {
public:
  /*!
  \brief Check if an event is included in the point cloud.
  \param e Event to check
  */
  [[nodiscard]] inline bool contains(const Event_<E> &e) const {
    return std::find(points_[e.p].begin(), points_[e.p].end(), cv::Point3f(e.x, e.y, e.t)) != points_[e.p].end();
  }

#if OE_HAVE_VIZ
  /*!
  \brief Visualize point cloud
  \param t Amount of time in milliseconds for the event loop to keep running. Zero means "forever"
  \param time_scale Events will be represented as (x, y, time_scale * t).
  \param axis_size Size of the 3D axis in the representation
  \param point_size Size of each point representing an event
  */
  void visualize(const int t, const double time_scale = 1.0, const double axis_size = 1.0, const double point_size = 2.0);
#endif

private:
  std::array<std::vector<cv::Point3_<typename TypeHelper<T>::FloatingPointType>>, 2> points_;
#if OE_HAVE_VIZ
  cv::viz::Viz3d window_{"OpenEV"};
#endif

  void clear_() override;
  void clear_(const cv::Mat &background) override;
  bool insert_(const Event_<E> &e) override;
};
using PointCloud1b = PointCloud_<uchar>;     /*!< Alias for PointCloud_ using uchar */
using PointCloud3b = PointCloud_<cv::Vec3b>; /*!< Alias for PointCloud_ using cv::Vec3b */
using PointCloud1s = PointCloud_<short>;     /*!< Alias for PointCloud_ using short */
using PointCloud3s = PointCloud_<cv::Vec3s>; /*!< Alias for PointCloud_ using cv::Vec3s */
using PointCloud1w = PointCloud_<ushort>;    /*!< Alias for PointCloud_ using ushort */
using PointCloud3w = PointCloud_<cv::Vec3w>; /*!< Alias for PointCloud_ using cv::Vec3w */
using PointCloud1i = PointCloud_<int>;       /*!< Alias for PointCloud_ using int */
using PointCloud3i = PointCloud_<cv::Vec3i>; /*!< Alias for PointCloud_ using cv::Vec3i */
using PointCloud1f = PointCloud_<float>;     /*!< Alias for PointCloud_ using float */
using PointCloud3f = PointCloud_<cv::Vec3f>; /*!< Alias for PointCloud_ using cv::Vec3f */
using PointCloud1d = PointCloud_<double>;    /*!< Alias for PointCloud_ using double */
using PointCloud3d = PointCloud_<cv::Vec3d>; /*!< Alias for PointCloud_ using cv::Vec3d */
using PointCloud1 = PointCloud1b;            /*!< Alias for PointCloud_ using uchar */
using PointCloud3 = PointCloud3b;            /*!< Alias for PointCloud_ using cv::Vec3b */
using PointCloud = PointCloud1;              /*!< Alias for PointCloud_ using uchar */
} // namespace ev

/*! \cond INTERNAL */
#include "openev/representations/point-cloud.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATIONS_POINT_CLOUD_HPP
