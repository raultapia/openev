/*!
\file frames.hpp
\brief Timestamped images.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_FRAMES_HPP
#define OPENEV_CORE_FRAMES_HPP

#include "openev/core/types.hpp"
#include <opencv2/core/mat.hpp>
#include <queue>
#include <vector>

namespace ev {
[[maybe_unused]] constexpr bool USING_FRAMES_HPP = true;

/*!
\brief This class extends cv::Mat to include timestamp.

The following aliases are defined for convenience:
\code{.cpp}
using StampedMatVector = std::vector<StampedMat>;
using StampedMatQueue = std::queue<StampedMat>;
\endcode
*/
class StampedMat : public cv::Mat {
public:
  TimeType t{0};

  using cv::Mat::copyTo;

  /*!
  \brief Copy image data and timestamp to another StampedMat.
  \param dst Destination
  */
  void copyTo(StampedMat &dst) const {
    cv::Mat::copyTo(dst);
    dst.t = t;
  }

  /*!
  \brief Release the image data and reset the timestamp.
  */
  void release() {
    cv::Mat::release();
    t = 0;
  }
};
using StampedMatVector = std::vector<StampedMat>;
using StampedMatQueue = std::queue<StampedMat>;

} // namespace ev

#endif // OPENEV_CORE_FRAMES_HPP
