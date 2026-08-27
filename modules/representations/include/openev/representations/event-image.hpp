/*!
\file event-image.hpp
\brief Event images.
\author Raul Tapia
*/
#ifndef OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP
#define OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP

#include "openev/representations/abstract-representation.hpp"
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
\brief This class extends cv::Mat_<T> for event images. For more information, please refer <a href="https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html">here</a>.

Analogously to OpenCV library, the following aliases are defined for convenience:
\code{.cpp}
using EventImage1b = EventImage_<uchar>;
using EventImage3b = EventImage_<cv::Vec3b>;
using EventImage1s = EventImage_<short>;
using EventImage3s = EventImage_<cv::Vec3s>;
using EventImage1w = EventImage_<ushort>;
using EventImage3w = EventImage_<cv::Vec3w>;
using EventImage1i = EventImage_<int>;
using EventImage3i = EventImage_<cv::Vec3i>;
using EventImage1f = EventImage_<float>;
using EventImage3f = EventImage_<cv::Vec3f>;
using EventImage1d = EventImage_<double>;
using EventImage3d = EventImage_<cv::Vec3d>;
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
  [[nodiscard]] cv::Size frameSize_() const override {
    return cv::Mat_<T>::size();
  }

public:
private:
  void clear_() override;
  void clear_(const cv::Mat &background) override;
  bool insert_(const Event_<E> &e) override;
};
using EventImage1b = EventImage_<uchar>;     /*!< Alias for EventImage_ using uchar */
using EventImage3b = EventImage_<cv::Vec3b>; /*!< Alias for EventImage_ using cv::Vec3b */
using EventImage1s = EventImage_<short>;     /*!< Alias for EventImage_ using short */
using EventImage3s = EventImage_<cv::Vec3s>; /*!< Alias for EventImage_ using cv::Vec3s */
using EventImage1w = EventImage_<ushort>;    /*!< Alias for EventImage_ using ushort */
using EventImage3w = EventImage_<cv::Vec3w>; /*!< Alias for EventImage_ using cv::Vec3w */
using EventImage1i = EventImage_<int>;       /*!< Alias for EventImage_ using int */
using EventImage3i = EventImage_<cv::Vec3i>; /*!< Alias for EventImage_ using cv::Vec3i */
using EventImage1f = EventImage_<float>;     /*!< Alias for EventImage_ using float */
using EventImage3f = EventImage_<cv::Vec3f>; /*!< Alias for EventImage_ using cv::Vec3f */
using EventImage1d = EventImage_<double>;    /*!< Alias for EventImage_ using double */
using EventImage3d = EventImage_<cv::Vec3d>; /*!< Alias for EventImage_ using cv::Vec3d */
using EventImage1 = EventImage1b;            /*!< Alias for EventImage_ using uchar */
using EventImage3 = EventImage3b;            /*!< Alias for EventImage_ using cv::Vec3b */
using EventImage = EventImage1;              /*!< Alias for EventImage_ using uchar */
} // namespace ev

/*! \cond INTERNAL */
#include "openev/representations/event-image.tpp"
/*! \endcond */

#endif // OPENEV_REPRESENTATIONS_EVENT_IMAGES_HPP
