#include "openev/evproc/undistortion.hpp"
#include <gtest/gtest.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

static ev::UndistortMap makeIdentityMap(const cv::Size &sz) {
  const cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, sz.width / 2.0,
                                                0.0, 600.0, sz.height / 2.0,
                                                0.0,   0.0,            1.0);
  const cv::Mat D = cv::Mat::zeros(1, 5, CV_64F);
  return {K, D, sz};
}

TEST(UndistortMapTest, ZeroDistortionMapsPointToItself) {
  const cv::Size sz{640, 480};
  ev::UndistortMap map = makeIdentityMap(sz);
  cv::Point_<double> p(100.0, 150.0);
  map(p);
  EXPECT_NEAR(p.x, 100.0, 0.5);
  EXPECT_NEAR(p.y, 150.0, 0.5);
}

TEST(UndistortMapTest, OperatorRectReturnsCorrectSize) {
  const cv::Size sz{640, 480};
  ev::UndistortMap map = makeIdentityMap(sz);
  const cv::Rect rect = static_cast<cv::Rect>(map);
  EXPECT_EQ(rect.width, 640);
  EXPECT_EQ(rect.height, 480);
}

TEST(UndistortMapTest, OperatorSizeReturnsCorrectSize) {
  const cv::Size sz{640, 480};
  ev::UndistortMap map = makeIdentityMap(sz);
  const cv::Size size = static_cast<cv::Size>(map);
  EXPECT_EQ(size.width, 640);
  EXPECT_EQ(size.height, 480);
}

TEST(UndistortMapTest, PointInsideBoundsReturnsTrue) {
  const cv::Size sz{640, 480};
  ev::UndistortMap map = makeIdentityMap(sz);
  cv::Point_<double> p(320.0, 240.0);
  EXPECT_TRUE(map(p));
}
