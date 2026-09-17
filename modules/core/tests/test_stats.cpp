#include "openev/core/stats.hpp"
#include "openev/core/types.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <vector>

class Timed : public ev::Stats_<Timed> {
public:
  [[nodiscard]] std::size_t count() const { return 10; }
  [[nodiscard]] cv::Size size() const { return {640, 480}; }
  [[nodiscard]] ev::TimeType firstTimestamp() const { return 2.0; }
  [[nodiscard]] ev::TimeType lastTimestamp() const { return 7.0; }
};

class Listed : public std::vector<ev::Event>, public ev::Stats_<Listed> {};

TEST(Stats, GettersAvoidTraversingTheEvents) {
  const Timed timed;
  EXPECT_DOUBLE_EQ(timed.duration(), 5.0);
  EXPECT_DOUBLE_EQ(timed.midTime(), 4.5);
}

TEST(Stats, CountTakesPrecedenceOverSize) {
  const Timed timed;
  EXPECT_DOUBLE_EQ(timed.rate(), 10.0 / 5.0);
  EXPECT_DOUBLE_EQ(timed.density(cv::Size(5, 2)), 1.0);
}

TEST(Stats, SizeIsTheCountWithoutCount) {
  Listed listed;
  listed.emplace_back(1, 1, 1.0, true);
  listed.emplace_back(3, 5, 3.0, false);
  EXPECT_DOUBLE_EQ(listed.rate(), 2.0 / 2.0);
  EXPECT_DOUBLE_EQ(listed.density(cv::Size(4, 1)), 0.5);
}

TEST(Stats, EventsAreTraversedWithoutGetters) {
  Listed listed;
  listed.emplace_back(1, 1, 1.0, true);
  listed.emplace_back(3, 5, 3.0, false);
  EXPECT_DOUBLE_EQ(listed.duration(), 2.0);
  EXPECT_DOUBLE_EQ(listed.meanPoint().x, 2.0);
  EXPECT_DOUBLE_EQ(listed.meanPoint().y, 3.0);
  EXPECT_EQ(listed.boundingBox(), cv::Rect(1, 1, 3, 5));
  EXPECT_EQ(listed.activePixels(), 2U);
  EXPECT_DOUBLE_EQ(listed.entropy(), 1.0);
}

TEST(Stats, BaseAddsNoStorage) {
  EXPECT_EQ(sizeof(Listed), sizeof(std::vector<ev::Event>));
}

class Boxed : public std::vector<ev::Event>, public ev::Stats_<Boxed> {
public:
  mutable int asked = 0;
  [[nodiscard]] cv::Rect bounds() const {
    asked++;
    return {0, 0, 8, 6};
  }
};

TEST(Stats, PixelCountsUseTheBoundsGetter) {
  Boxed boxed;
  boxed.emplace_back(1, 1, 1.0, true);
  boxed.emplace_back(3, 5, 2.0, false);
  boxed.emplace_back(3, 5, 3.0, true);
  EXPECT_EQ(boxed.activePixels(), 2U);
  EXPECT_EQ(boxed.peak(), 2U);
  EXPECT_EQ(boxed.asked, 2);
  EXPECT_EQ(boxed.boundingBox(), cv::Rect(0, 0, 8, 6));
}
