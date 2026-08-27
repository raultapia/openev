#include "openev/evproc/undistortion.hpp"
#include <gtest/gtest.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

static ev::UndistortMap makeIdentityMap(const cv::Size &sz) {
  const cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, sz.width / 2.0,
                     0.0, 600.0, sz.height / 2.0,
                     0.0, 0.0, 1.0);
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

TEST(UndistortMapTest, PointOutsideBoundsIsLeftUntouched) {
  const cv::Size sz{640, 480};
  ev::UndistortMap map = makeIdentityMap(sz);
  cv::Point_<int> p(9999, 9999);
  EXPECT_FALSE(map(p));
  EXPECT_EQ(p.x, 9999);
  EXPECT_EQ(p.y, 9999);
}

TEST(UndistortMapTest, NegativePointIsLeftUntouched) {
  const cv::Size sz{640, 480};
  ev::UndistortMap map = makeIdentityMap(sz);
  cv::Point_<int> p(-1, -1);
  EXPECT_FALSE(map(p));
  EXPECT_EQ(p.x, -1);
  EXPECT_EQ(p.y, -1);
}

TEST(UndistortMapTest, VectorOverloadSkipsPointsOutsideBounds) {
  const cv::Size sz{640, 480};
  ev::UndistortMap map = makeIdentityMap(sz);
  ev::Vector_<int> events;
  events.emplace_back(100, 150, 1.0, true);
  events.emplace_back(9999, 9999, 2.0, true);
  map(events);
  EXPECT_NEAR(events[0].x, 100, 1);
  EXPECT_NEAR(events[0].y, 150, 1);
  EXPECT_EQ(events[1].x, 9999);
  EXPECT_EQ(events[1].y, 9999);
}

TEST(UndistortMapTest, MismatchedDataSizeYieldsEmptyMap) {
  const cv::Size sz{20, 10};
  const std::vector<cv::Point_<double>> tooFew(3, cv::Point_<double>(0.0, 0.0));
  const ev::UndistortMap map(tooFew, sz);
  EXPECT_TRUE(map.empty());
}

TEST(UndistortMapTest, MatchingDataSizeBuildsMap) {
  const cv::Size sz{20, 10};
  const std::vector<cv::Point_<double>> data(sz.area(), cv::Point_<double>(1.0, 2.0));
  const ev::UndistortMap map(data, sz);
  EXPECT_FALSE(map.empty());
  EXPECT_EQ(map.cols, sz.width);
  EXPECT_EQ(map.rows, sz.height);
}

TEST(UndistortMapTest, InvalidCameraMatrixYieldsEmptyMap) {
  const cv::Mat bad = cv::Mat::eye(2, 2, CV_64F);
  const cv::Mat D = cv::Mat::zeros(1, 5, CV_64F);
  const ev::UndistortMap map(bad, D, cv::Size(640, 480));
  EXPECT_TRUE(map.empty());
}

TEST(UndistortMapTest, InvalidIntrinsicsYieldEmptyMap) {
  const std::vector<double> bad{600.0, 600.0};
  const std::vector<double> D(5, 0.0);
  const ev::UndistortMap map(bad, D, cv::Size(640, 480));
  EXPECT_TRUE(map.empty());
}
