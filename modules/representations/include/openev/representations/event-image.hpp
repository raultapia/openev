/*!
\file event-image.hpp
\brief Event images.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP
#define OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP

#include "openev/core/types.hpp"
#include "openev/representations/abstract-representation.hpp"
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <utility>

namespace ev {
/*!
\brief This class extends cv::Mat_<T> for event images. For more information, please refer <a href="https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using EventImage1b = EventImage_<uchar>;
using EventImage2b = EventImage_<cv::Vec2b>;
using EventImage3b = EventImage_<cv::Vec3b>;
using EventImage4b = EventImage_<cv::Vec4b>;
using EventImage1s = EventImage_<short>;
using EventImage2s = EventImage_<cv::Vec2s>;
using EventImage3s = EventImage_<cv::Vec3s>;
using EventImage4s = EventImage_<cv::Vec4s>;
using EventImage1w = EventImage_<ushort>;
using EventImage2w = EventImage_<cv::Vec2w>;
using EventImage3w = EventImage_<cv::Vec3w>;
using EventImage4w = EventImage_<cv::Vec4w>;
using EventImage1i = EventImage_<int>;
using EventImage2i = EventImage_<cv::Vec2i>;
using EventImage3i = EventImage_<cv::Vec3i>;
using EventImage4i = EventImage_<cv::Vec4i>;
using EventImage1f = EventImage_<float>;
using EventImage2f = EventImage_<cv::Vec2f>;
using EventImage3f = EventImage_<cv::Vec3f>;
using EventImage4f = EventImage_<cv::Vec4f>;
using EventImage1d = EventImage_<double>;
using EventImage2d = EventImage_<cv::Vec2d>;
using EventImage3d = EventImage_<cv::Vec3d>;
using EventImage4d = EventImage_<cv::Vec4d>;
using EventImage1 = EventImage1b;
using EventImage3 = EventImage3b;
using EventImage = EventImage1;
\endcode
*/
template <typename T, const RepresentationOptions Options = RepresentationOptions::NONE, typename E = int>
class EventImage_ : public cv::Mat_<T>, public AbstractRepresentation_<T, Options, E> {
public:
  template <typename... Args>
  explicit EventImage_(Args &&...args) : cv::Mat_<T>(std::forward<Args>(args)...) {
    AbstractRepresentation_<T, Options, E>::clear();
  }

  cv::Mat &render() { return *this; }

private:
  void clear_() override;
  void clear_(const cv::Mat &background) override;
  bool insert_(const Event_<E> &e) override;
};
using EventImage1b = EventImage_<uchar>;     /*!< Alias for EventImage_ using uchar */
using EventImage2b = EventImage_<cv::Vec2b>; /*!< Alias for EventImage_ using cv::Vec2b */
using EventImage3b = EventImage_<cv::Vec3b>; /*!< Alias for EventImage_ using cv::Vec3b */
using EventImage4b = EventImage_<cv::Vec4b>; /*!< Alias for EventImage_ using cv::Vec4b */
using EventImage1s = EventImage_<short>;     /*!< Alias for EventImage_ using short */
using EventImage2s = EventImage_<cv::Vec2s>; /*!< Alias for EventImage_ using cv::Vec2s */
using EventImage3s = EventImage_<cv::Vec3s>; /*!< Alias for EventImage_ using cv::Vec3s */
using EventImage4s = EventImage_<cv::Vec4s>; /*!< Alias for EventImage_ using cv::Vec4s */
using EventImage1w = EventImage_<ushort>;    /*!< Alias for EventImage_ using ushort */
using EventImage2w = EventImage_<cv::Vec2w>; /*!< Alias for EventImage_ using cv::Vec2w */
using EventImage3w = EventImage_<cv::Vec3w>; /*!< Alias for EventImage_ using cv::Vec3w */
using EventImage4w = EventImage_<cv::Vec4w>; /*!< Alias for EventImage_ using cv::Vec4w */
using EventImage1i = EventImage_<int>;       /*!< Alias for EventImage_ using int */
using EventImage2i = EventImage_<cv::Vec2i>; /*!< Alias for EventImage_ using cv::Vec2i */
using EventImage3i = EventImage_<cv::Vec3i>; /*!< Alias for EventImage_ using cv::Vec3i */
using EventImage4i = EventImage_<cv::Vec4i>; /*!< Alias for EventImage_ using cv::Vec4i */
using EventImage1f = EventImage_<float>;     /*!< Alias for EventImage_ using float */
using EventImage2f = EventImage_<cv::Vec2f>; /*!< Alias for EventImage_ using cv::Vec2f */
using EventImage3f = EventImage_<cv::Vec3f>; /*!< Alias for EventImage_ using cv::Vec3f */
using EventImage4f = EventImage_<cv::Vec4f>; /*!< Alias for EventImage_ using cv::Vec4f */
using EventImage1d = EventImage_<double>;    /*!< Alias for EventImage_ using double */
using EventImage2d = EventImage_<cv::Vec2d>; /*!< Alias for EventImage_ using cv::Vec2d */
using EventImage3d = EventImage_<cv::Vec3d>; /*!< Alias for EventImage_ using cv::Vec3d */
using EventImage4d = EventImage_<cv::Vec4d>; /*!< Alias for EventImage_ using cv::Vec4d */
using EventImage1 = EventImage1b;            /*!< Alias for EventImage_ using uchar */
using EventImage3 = EventImage3b;            /*!< Alias for EventImage_ using cv::Vec3b */
using EventImage = EventImage1;              /*!< Alias for EventImage_ using uchar */
} // namespace ev

/*! \cond INTERNAL */
#include "openev/representations/event-image.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP
