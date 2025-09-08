/*!
\file undistortion.hpp
\brief Undistortion utilities.
\author Raul Tapia
*/
#ifndef OPENEV_EVPROC_UNDISTORTION_HPP
#define OPENEV_EVPROC_UNDISTORTION_HPP

#include "openev/utils/logger.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/traits.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <stdint.h>
#include <vector>

namespace ev {

class UndistortMap : public cv::Mat_<cv::Point_<double>> {
public:
  enum class VisualizationOptions : uint8_t {
    COLOR,
    NET
  };

  UndistortMap() = default;

  UndistortMap(const cv::Mat &cam_matrix, const cv::Mat &dist_coeff, const cv::Size &sz);

  UndistortMap(const std::vector<double> &intrinsics, const std::vector<double> &dist_coeff, const cv::Size &sz);

  template <typename T>
  UndistortMap(const std::vector<cv::Point_<T>> &data, const cv::Size &sz) {
    logger::error("UndistortMap: Data size does not match frame size", data.size() == static_cast<std::size_t>(sz.area()));
    cv::Mat_<cv::Point_<double>>::create(sz);
    cv::Mat_<cv::Point_<double>>::iterator it = cv::Mat_<cv::Point_<double>>::begin();
    std::for_each(data.begin(), data.end(), [&it](const cv::Point_<T> &p) { *it++ = cv::Point_<double>(p.x, p.y); });
  }

  template <typename T>
  inline bool operator()(cv::Point_<T> &p) const {
    p = static_cast<cv::Point_<T>>(cv::Mat_<cv::Point_<double>>::ptr<cv::Point_<double>>(static_cast<int>(p.y))[static_cast<int>(p.x)]);
    return p.inside(UndistortMap::operator cv::Rect());
  }

  inline void operator()(const cv::Mat &src, cv::Mat &dst) {
    cv::remap(src, dst, cvUndistortionMap_[0], cvUndistortionMap_[1], cv::INTER_LINEAR);
  }

  [[nodiscard]] inline explicit operator cv::Rect() const {
    return {0, 0, cv::Mat_<cv::Point_<double>>::cols, cv::Mat_<cv::Point_<double>>::rows};
  }

  [[nodiscard]] inline explicit operator cv::Size() const {
    return {cv::Mat_<cv::Point_<double>>::cols, cv::Mat_<cv::Point_<double>>::rows};
  }

  [[nodiscard]] cv::Mat visualize(const VisualizationOptions options = VisualizationOptions::COLOR) const;

private:
  void init(const cv::Mat &cam_matrix, const cv::Mat &dist_coeff, const cv::Size &sz);
  std::array<cv::Mat, 2> cvUndistortionMap_;
};

} // namespace ev

#endif // OPENEV_EVPROC_UNDISTORTION_HPP
