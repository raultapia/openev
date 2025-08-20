/*!
\file undistortion.hpp
\brief Implementation of undistortion.
\author Raul Tapia
*/
#include "openev/core/undistortion.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.inl.hpp>
#include <opencv2/core/saturate.hpp>
#include <opencv2/core/traits.hpp>
#include <opencv2/imgproc.hpp>
#include <utility>

ev::UndistortMap::UndistortMap(const cv::Mat &cam_matrix, const cv::Mat &dist_coeff, const cv::Size &sz) {
  ev::logger::error("UndistortMap: Camera matrix size should be 3x3", cam_matrix.rows == 3 && cam_matrix.cols == 3);
  ev::logger::error("UndistortMap: Distortion coefficients size should be 4, 5, 8, 12, 14, or 0", dist_coeff.size() == cv::Size(4, 1) ||
                                                                                                      dist_coeff.size() == cv::Size(5, 1) ||
                                                                                                      dist_coeff.size() == cv::Size(8, 1) ||
                                                                                                      dist_coeff.size() == cv::Size(12, 1) ||
                                                                                                      dist_coeff.size() == cv::Size(14, 1) ||
                                                                                                      dist_coeff.size() == cv::Size(1, 4) ||
                                                                                                      dist_coeff.size() == cv::Size(1, 5) ||
                                                                                                      dist_coeff.size() == cv::Size(1, 8) ||
                                                                                                      dist_coeff.size() == cv::Size(1, 12) ||
                                                                                                      dist_coeff.size() == cv::Size(1, 14) ||
                                                                                                      dist_coeff.size() == cv::Size(0, 0));
  init(cam_matrix, dist_coeff, sz);
}

ev::UndistortMap::UndistortMap(const std::vector<double> &intrinsics, const std::vector<double> &dist_coeff, const cv::Size &sz) {
  ev::logger::error("UndistortMap: Intrinsic parameters size should be 4: fx, fy, cx, cy", intrinsics.size() == 4);
  ev::logger::error("UndistortMap: Distortion coefficients size should be 4, 5, 8, 12, 14, or 0", dist_coeff.size() == 4 || dist_coeff.size() == 5 || dist_coeff.size() == 8 || dist_coeff.size() == 12 || dist_coeff.size() == 14 || dist_coeff.empty());
  std::vector<double> d;
  std::copy(dist_coeff.begin(), dist_coeff.end(), std::back_inserter(d));
  init((cv::Mat_<double>(3, 3) << intrinsics[0], 0.0, intrinsics[2], 0.0, intrinsics[1], intrinsics[3], 0.0, 0.0, 1.0), cv::Mat(cv::Size(1, static_cast<int>(d.size())), CV_64F, d.data()), sz);
}

static inline void draw_center(cv::Mat &m) {
  cv::line(m, cv::Point(static_cast<int>(0.45 * m.cols), m.rows / 2), cv::Point(static_cast<int>(0.55 * m.cols), m.rows / 2), {255, 255, 255}, 1);
  cv::line(m, cv::Point(m.cols / 2, static_cast<int>(0.45 * m.rows)), cv::Point(m.cols / 2, static_cast<int>(0.55 * m.rows)), {255, 255, 255}, 1);
}

void ev::UndistortMap::init(const cv::Mat &cam_matrix, const cv::Mat &dist_coeff, const cv::Size &sz) {
  std::vector<cv::Point_<double>> points_src(static_cast<std::size_t>(sz.width * sz.height));
  std::vector<cv::Point_<double>> points_dst;

  std::generate(points_src.begin(), points_src.end(), [sz, x = -1.0, y = 0.0]() mutable {
    if(++x >= sz.width) {
      return cv::Point_<double>(x = 0, y++);
    }
    return cv::Point_<double>(x, y);
  });

  cv::undistortPoints(points_src, points_dst, cam_matrix, dist_coeff, cv::noArray(), cam_matrix);
  cv::Mat_<cv::Point_<double>>::create(sz);
  cv::Mat_<cv::Point_<double>>::iterator it = cv::Mat_<cv::Point_<double>>::begin();
  std::for_each(points_dst.begin(), points_dst.end(), [&it](const cv::Point_<double> &p) { *it++ = cv::Point_<double>(p.x, p.y); });
  cv::initUndistortRectifyMap(cam_matrix, dist_coeff, cv::Mat(), cam_matrix, sz, CV_32FC1, cvUndistortionMap_[0], cvUndistortionMap_[1]);
}

[[nodiscard]] cv::Mat ev::UndistortMap::visualize(const VisualizationOptions options /*= VisualizationMode::COLOR*/) const {
  switch(options) {
  case VisualizationOptions::COLOR: {
    cv::Mat viz32FC1(cv::Mat_<cv::Point_<double>>::size(), CV_32FC1);
    cv::Mat viz8UC1(cv::Mat_<cv::Point_<double>>::size(), CV_8UC1);
    cv::Mat viz8UC3(cv::Mat_<cv::Point_<double>>::size(), CV_8UC3);

    for(int r = 0; r < cv::Mat_<cv::Point_<double>>::size().height; r++) {
      for(int c = 0; c < cv::Mat_<cv::Point_<double>>::size().width; c++) {
        viz32FC1.at<float>(r, c) = static_cast<float>(sqrt(cv::norm(cv::Point_<double>(c, r) - cv::Mat_<cv::Point_<double>>::at<cv::Point_<double>>(r, c))));
      }
    }

    double min{0};
    double max{0};
    cv::minMaxLoc(viz32FC1, &min, &max);
    const double scale = 255.0 / (max - min);
    viz32FC1.convertTo(viz8UC1, CV_8UC1, scale, -min * scale);
    cv::applyColorMap(viz8UC1, viz8UC3, cv::COLORMAP_AUTUMN);
    draw_center(viz8UC3);
    return viz8UC3;
  }
  case VisualizationOptions::NET: {
    constexpr std::array<uint8_t, 3> rgb_p1 = {255, 0, 0};
    constexpr std::array<uint8_t, 3> rgb_p2 = {0, 255, 0};
    constexpr std::array<uint8_t, 3> rgb_line = {80, 80, 80};

    cv::Mat viz8UC3 = cv::Mat::zeros(cv::Mat_<cv::Point_<double>>::size(), CV_8UC3);

    cv::Mat_<cv::Point_<double>> inverse_map(cv::Mat_<cv::Point_<double>>::size(), cv::Point_<double>(0, 0));
    for(int r = 0; r < cv::Mat_<cv::Point_<double>>::size().height; r++) {
      for(int c = 0; c < cv::Mat_<cv::Point_<double>>::size().width; c++) {
        const cv::Point_<double> &p = cv::Mat_<cv::Point_<double>>::at<cv::Point_<double>>(r, c);
        if(cv::Point(p).inside(ev::UndistortMap::operator cv::Rect()) && cv::Point(p + cv::Point_<double>(1, 1)).inside(ev::UndistortMap::operator cv::Rect())) {
          inverse_map.at<cv::Point_<double>>(static_cast<int>(p.y), static_cast<int>(p.x)) = cv::Point_<double>(r, c);
          inverse_map.at<cv::Point_<double>>(static_cast<int>(p.y) + 1, static_cast<int>(p.x)) = cv::Point_<double>(r, c);
          inverse_map.at<cv::Point_<double>>(static_cast<int>(p.y), static_cast<int>(p.x) + 1) = cv::Point_<double>(r, c);
          inverse_map.at<cv::Point_<double>>(static_cast<int>(p.y) + 1, static_cast<int>(p.x) + 1) = cv::Point_<double>(r, c);
        }
      }
    }

    for(int r = 3; r < cv::Mat_<cv::Point_<double>>::size().height; r += 25) {
      for(int c = 3; c < cv::Mat_<cv::Point_<double>>::size().width; c += 25) {
        const cv::Point_<double> p1(c, r);
        cv::Point_<double> p2 = inverse_map.at<cv::Point_<double>>(r, c);
        std::swap(p2.x, p2.y);
        cv::line(viz8UC3, cv::Point(p1), cv::Point(p2), cv::Scalar(rgb_line[2], rgb_line[1], rgb_line[0]));
        cv::rectangle(viz8UC3, cv::Point(static_cast<int>(p1.x) - 2, static_cast<int>(p1.y) - 2), cv::Point(static_cast<int>(p1.x) + 2, static_cast<int>(p1.y) + 2), cv::Scalar(rgb_p1[2], rgb_p1[1], rgb_p1[0]));
        cv::rectangle(viz8UC3, cv::Point(static_cast<int>(p2.x) - 2, static_cast<int>(p2.y) - 2), cv::Point(static_cast<int>(p2.x) + 2, static_cast<int>(p2.y) + 2), cv::Scalar(rgb_p2[2], rgb_p2[1], rgb_p2[0]));
      }
    }

    draw_center(viz8UC3);
    return viz8UC3;
  }
  default:
    ev::logger::error("UndistortMap::visualize: Bad option");
    return {};
  }
}
