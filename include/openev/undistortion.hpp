/*!
\file undistortion.hpp
\brief Undistortion utilities.
\author Raul Tapia
*/
#ifndef OPENEV_UNDISTORTION_HPP
#define OPENEV_UNDISTORTION_HPP

#include "openev/logger.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

namespace ev {

class UndistortMap : public cv::Mat_<cv::Vec<double, 2>> {
public:
  enum class VisualizationOptions : uint8_t {
    COLOR,
    NET
  };

  UndistortMap() = default;

  UndistortMap(const cv::Mat &cam_matrix, const cv::Mat &dist_coeff, const cv::Size &sz);

  UndistortMap(const std::vector<double> &intrinsics, std::vector<double> &dist_coeff, const cv::Size &sz);

  template <typename T>
  UndistortMap(const std::vector<cv::Point_<T>> &data, const cv::Size &sz) {
    logger::error("UndistortMap: Data size does not match frame size", data.size() == static_cast<std::size_t>(sz.area()));
    cv::Mat_<cv::Vec<double, 2>>::create(sz);
    cv::Mat_<cv::Vec<double, 2>>::iterator it = cv::Mat_<cv::Vec<double, 2>>::begin();
    std::for_each(data.begin(), data.end(), [&it](const cv::Point_<T> &p) { *it++ = cv::Vec<double, 2>(p.x, p.y); });
  }

  template <typename T>
  inline bool operator()(cv::Point_<T> &p) const {
    logger::error("UndistortMap: Input point is out of this map", p.inside(UndistortMap::operator cv::Rect()));
    const cv::Vec<double, 2> &q = cv::Mat_<cv::Vec<double, 2>>::operator()(p.y, p.x);
    p.x = static_cast<T>(q[0]);
    p.y = static_cast<T>(q[1]);
    return p.inside(UndistortMap::operator cv::Rect());
  }

  [[nodiscard]] inline explicit operator cv::Rect() const {
    return {0, 0, cv::Mat_<cv::Vec<double, 2>>::cols, cv::Mat_<cv::Vec<double, 2>>::rows};
  }

  [[nodiscard]] cv::Mat visualize(const VisualizationOptions options = VisualizationOptions::COLOR) const;

private:
  void init(const cv::Mat &cam_matrix, const cv::Mat &dist_coeff, const cv::Size &sz);
};

} // namespace ev

#endif // OPENEV_UNDISTORTION_HPP
