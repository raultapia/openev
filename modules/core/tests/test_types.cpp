#include "openev/core/types.hpp"
#include <gtest/gtest.h>
#include <opencv2/core/base.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <sstream>
#include <string>

// Test Enums
TEST(EnumsTest, StereoValues) {
  EXPECT_EQ(static_cast<char>(ev::Stereo::LEFT), 'L');
  EXPECT_EQ(static_cast<char>(ev::Stereo::RIGHT), 'R');
}

TEST(EnumsTest, DistanceTypesValues) {
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_NORM_INF), cv::NORM_INF);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_NORM_L1), cv::NORM_L1);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_NORM_L2), cv::NORM_L2);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_NORM_L2SQR), cv::NORM_L2SQR);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_NORM_MANHATTAN), ev::DISTANCE_NORM_L1);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_NORM_EUCLIDEAN), ev::DISTANCE_NORM_L2);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_FLAG_SPATIAL), 0b00010000);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_FLAG_TEMPORAL), 0b00100000);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_FLAG_SPATIOTEMPORAL), 0b01000000);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_FLAG_3D), ev::DISTANCE_FLAG_SPATIOTEMPORAL);
  EXPECT_EQ(static_cast<int>(ev::DistanceTypes::DISTANCE_FLAG_2D), ev::DISTANCE_FLAG_SPATIAL);
}

// Test Event_ Class
TEST(EventTest, ConstructorDefault) {
  const ev::Event_<double> event;
  EXPECT_EQ(event.t, 0.0);
  EXPECT_TRUE(event.p);
}

TEST(EventTest, ConstructorWithXYValues) {
  const ev::Event_<int> e1(1, 2);
  EXPECT_EQ(e1.x, 1);
  EXPECT_EQ(e1.y, 2);
  EXPECT_EQ(e1.t, 0.0);
  EXPECT_TRUE(e1.p);
}

TEST(EventTest, ConstructorWithPointValues) {
  const ev::Event_<int> e2(cv::Point_<int>(1, 2));
  EXPECT_EQ(e2.x, 1);
  EXPECT_EQ(e2.y, 2);
  EXPECT_EQ(e2.t, 0.0);
  EXPECT_TRUE(e2.p);
}

TEST(EventTest, ConstructorWithXYTPValues) {
  const ev::Event_<int> e3(1, 2, 8, false);
  EXPECT_EQ(e3.x, 1);
  EXPECT_EQ(e3.y, 2);
  EXPECT_EQ(e3.t, 8);
  EXPECT_FALSE(e3.p);
}

TEST(EventTest, ConstructorWithPointTPValues) {
  const ev::Event_<int> e4(cv::Point_<int>(1, 2), 8, false);
  EXPECT_EQ(e4.x, 1);
  EXPECT_EQ(e4.y, 2);
  EXPECT_EQ(e4.t, 8);
  EXPECT_FALSE(e4.p);
}

TEST(EventTest, AssignmentOperator) {
  const ev::Event_<int> event1(1, 2);
  ev::Event_<double> event2;
  event2 = event1;
  EXPECT_EQ(event2.x, 1);
  EXPECT_EQ(event2.y, 2);
  EXPECT_EQ(event2.t, 0.0);
  EXPECT_TRUE(event2.p);
}

TEST(EventTest, EqualityOperatorEvent) {
  const ev::Event_<int> event1(1, 2, 3, true);
  const ev::Event_<int> event2(1, 2, 3, true);
  const ev::Event_<int> event3(4, 5, 6, false);

  EXPECT_TRUE(event1 == event2);
  EXPECT_FALSE(event1 == event3);
}

TEST(EventTest, EqualityOperatorPoint) {
  const ev::Event_<int> event(1, 2, 3, true);
  const cv::Point_<int> point1(1, 2);
  const cv::Point_<int> point2(3, 4);

  EXPECT_TRUE(event == point1);
  EXPECT_FALSE(event == point2);
}

TEST(EventTest, EqualityOperatorPoint3) {
  const ev::Event_<int> event(1, 2, 3, true);
  const cv::Point3_<int> point3_1(1, 2, 3);
  const cv::Point3_<int> point3_2(4, 5, 6);

  EXPECT_TRUE(event == point3_1);
  EXPECT_FALSE(event == point3_2);
}

TEST(EventTest, LessThanOperator) {
  const ev::Event_<double> event1(1.0, 2.0, 3.0, true);
  const ev::Event_<double> event2(4.0, 5.0, 6.0, false);
  const ev::Event_<double> event3(7.0, 8.0, 2.0, true);

  EXPECT_TRUE(event1 < event2);
  EXPECT_FALSE(event2 < event1);
  EXPECT_FALSE(event1 < event3);
  EXPECT_TRUE(event3 < event1);
}

TEST(EventTest, StreamOperator) {
  const ev::Event_<int> event(1, 2, 3, true);
  std::ostringstream oss;
  [[maybe_unused]] auto &result = oss << event;
  EXPECT_EQ(oss.str(), std::string("(1,2) 3.000000 [+]"));
}

TEST(EventTest, CastToPoint) {
  const ev::Event_<int> event(3, 4, 5, true);
  const cv::Point_<double> point = static_cast<cv::Point_<double>>(event);
  EXPECT_DOUBLE_EQ(point.x, 3.0);
  EXPECT_DOUBLE_EQ(point.y, 4.0);
}

TEST(EventTest, CastToPoint3) {
  const ev::Event_<int> event(3, 4, 5, true);
  const cv::Point3_<double> point3 = static_cast<cv::Point3_<double>>(event);
  EXPECT_DOUBLE_EQ(point3.x, 3.0);
  EXPECT_DOUBLE_EQ(point3.y, 4.0);
  EXPECT_DOUBLE_EQ(point3.z, 5.0);
}

TEST(EventTest, DistanceMethod) {
  const ev::Event_<double> event1(1.0, 2.0, 3.0, true);
  const ev::Event_<double> event2(4.0, 6.0, 8.0, false);
  EXPECT_DOUBLE_EQ(event1.distance(event2), cv::norm(cv::Matx<double, 2, 1>(-3.0, -4.0), cv::NORM_L2));
  EXPECT_DOUBLE_EQ(event1.distance(event2, ev::DISTANCE_NORM_L2 | ev::DISTANCE_FLAG_SPATIOTEMPORAL), cv::norm(cv::Matx<double, 3, 1>(-3.0, -4.0, -5.0), cv::NORM_L2));
  EXPECT_DOUBLE_EQ(event1.distance(event2, ev::DISTANCE_FLAG_TEMPORAL), -5.0);
  EXPECT_DOUBLE_EQ(event1.distance(event2, ev::DISTANCE_NORM_L2 | ev::DISTANCE_FLAG_SPATIAL), cv::norm(cv::Matx<double, 2, 1>(-3.0, -4.0), cv::NORM_L2));
}

// Test AugmentedEvent_ Class
TEST(AugmentedEventTest, ConstructorDefault) {
  const ev::AugmentedEvent_<double> event;
  EXPECT_EQ(event.t, 0.0);
  EXPECT_TRUE(event.p);
  EXPECT_DOUBLE_EQ(event.weight, 1.0);
  EXPECT_DOUBLE_EQ(event.depth, 0.0);
  EXPECT_EQ(event.stereo, ev::Stereo::LEFT);
}

TEST(AugmentedEventTest, StreamOperator) {
  ev::AugmentedEvent_<int> event(1, 2, 3, true);
  event.weight = 2.5;
  event.depth = 10.0;
  event.stereo = ev::Stereo::RIGHT;
  std::ostringstream oss;
  [[maybe_unused]] auto &result = oss << event;
  EXPECT_EQ(oss.str(), std::string("(1,2) 3.000000 [+] w=2.500000 d=10.000000 s=RIGHT"));
}

TEST(AugmentedEventTest, ConstructorWithXYValues) {
  const ev::AugmentedEvent_<int> e1(1, 2);
  EXPECT_EQ(e1.x, 1);
  EXPECT_EQ(e1.y, 2);
  EXPECT_EQ(e1.t, 0.0);
  EXPECT_TRUE(e1.p);
  EXPECT_DOUBLE_EQ(e1.weight, 1.0);
  EXPECT_DOUBLE_EQ(e1.depth, 0.0);
  EXPECT_EQ(e1.stereo, ev::Stereo::LEFT);
}

TEST(AugmentedEventTest, ConstructorWithXYTPValues) {
  const ev::AugmentedEvent_<int> e2(1, 2, 8, false);
  EXPECT_EQ(e2.x, 1);
  EXPECT_EQ(e2.y, 2);
  EXPECT_EQ(e2.t, 8);
  EXPECT_FALSE(e2.p);
  EXPECT_DOUBLE_EQ(e2.weight, 1.0);
  EXPECT_DOUBLE_EQ(e2.depth, 0.0);
  EXPECT_EQ(e2.stereo, ev::Stereo::LEFT);
}

TEST(AugmentedEventTest, ModifyWeightDepthStereo) {
  ev::AugmentedEvent_<double> event(1, 2, 3, true);
  event.weight = 2.5;
  event.depth = 10.0;
  event.stereo = ev::Stereo::RIGHT;

  EXPECT_DOUBLE_EQ(event.weight, 2.5);
  EXPECT_DOUBLE_EQ(event.depth, 10.0);
  EXPECT_EQ(event.stereo, ev::Stereo::RIGHT);
}

// Test Size2 Class
TEST(Size2Test, ConstructorDefault) {
  const ev::Size2 size;
  EXPECT_EQ(size.width, 0);
  EXPECT_EQ(size.height, 0);
}

TEST(Size2Test, ConstructorWithValues) {
  const ev::Size2 size(3, 4);
  EXPECT_EQ(size.width, 3);
  EXPECT_EQ(size.height, 4);
}

TEST(Size2Test, AreaCalculation) {
  const ev::Size2 size(3, 4);
  EXPECT_EQ(size.area(), 12);
}

// Test Rect2 Class
TEST(Rect2Test, ConstructorDefault) {
  const ev::Rect2 rect;
  EXPECT_EQ(rect.width, 0);
  EXPECT_EQ(rect.height, 0);
}

TEST(Rect2Test, ConstructorWithValues) {
  const ev::Rect2 rect(1, 2, 3, 4);
  EXPECT_EQ(rect.x, 1);
  EXPECT_EQ(rect.y, 2);
  EXPECT_EQ(rect.width, 3);
  EXPECT_EQ(rect.height, 4);
}

TEST(Rect2Test, ContainsPoint) {
  const ev::Rect2 rect(1, 2, 3, 4);
  EXPECT_TRUE(rect.contains(cv::Point(2, 3)));
  EXPECT_FALSE(rect.contains(cv::Point(5, 6)));
}

// Test Size3_ Class
TEST(Size3Test, ConstructorDefault) {
  const ev::Size3_<int> size;
  EXPECT_EQ(size.length, 0);
  EXPECT_TRUE(size.empty());
}

TEST(Size3Test, ConstructorWithValues) {
  const ev::Size3_<int> size(3, 4, 5);
  EXPECT_EQ(size.length, 5);
  EXPECT_EQ(size.volume(), 60);
}

TEST(Size3Test, EmptyMethod) {
  const ev::Size3_<int> size(0, 0, 0);
  EXPECT_TRUE(size.empty());
}

TEST(Size3Test, NegativeDimensions) {
  const ev::Size3_<int> size(-3, 4, 5);
  EXPECT_EQ(size.length, 5);
  EXPECT_EQ(size.volume(), -60);
}

// Test Rect3_ Class
TEST(Rect3Test, ConstructorDefault) {
  const ev::Rect3_<int> rect;
  EXPECT_EQ(rect.length, 0);
  EXPECT_TRUE(rect.empty());
}

TEST(Rect3Test, ConstructorWithValues) {
  const ev::Rect3_<int> rect(1, 1, 1, 3, 4, 5);
  EXPECT_EQ(rect.length, 5);
  EXPECT_EQ(rect.volume(), 60);
}

TEST(Rect3Test, ContainsMethod) {
  const ev::Rect3_<int> rect(1, 1, 1, 3, 4, 5);
  EXPECT_TRUE(rect.contains(ev::Event_<int>(1, 1, 1, true)));
  EXPECT_FALSE(rect.contains(ev::Event_<int>(4, 4, 4, true)));
}

TEST(Rect3Test, ZeroDimensions) {
  const ev::Rect3_<int> rect(1, 1, 1, 0, 0, 0);
  EXPECT_TRUE(rect.empty());
  EXPECT_FALSE(rect.contains(ev::Event_<int>(1, 1, 1, true)));
}

TEST(Rect3Test, SizeMethod) {
  const ev::Rect3_<int> rect(1, 1, 1, 3, 4, 5);
  auto size = rect.size();
  EXPECT_EQ(size.width, 3);
  EXPECT_EQ(size.height, 4);
  EXPECT_EQ(size.length, 5);
}

// Test Circ_ Class
TEST(CircTest, ConstructorDefault) {
  const ev::Circ_<int> circ;
  EXPECT_EQ(circ.radius, 0);
  EXPECT_TRUE(circ.empty());
}

TEST(CircTest, ConstructorWithValues) {
  const ev::Circ_<int> circ(cv::Point_<int>(5, 5), 10);
  EXPECT_EQ(circ.radius, 10);
  EXPECT_DOUBLE_EQ(circ.area(), 314.1592653589793);
}

TEST(CircTest, ContainsMethod) {
  const ev::Circ_<int> circ(cv::Point_<int>(5, 5), 10);
  EXPECT_TRUE(circ.contains(ev::Event_<int>(7, 7, 7, true)));
  EXPECT_FALSE(circ.contains(ev::Event_<int>(20, 20, 20, true)));
}

TEST(CircTest, ZeroRadius) {
  const ev::Circ_<int> circ(cv::Point_<int>(5, 5), 0);
  EXPECT_TRUE(circ.empty());
  EXPECT_FALSE(circ.contains(ev::Event_<int>(5, 5, 5, true)));
}

TEST(CircTest, NegativeRadius) {
  const ev::Circ_<int> circ(cv::Point_<int>(5, 5), -10);
  EXPECT_TRUE(circ.empty());
  EXPECT_FALSE(circ.contains(ev::Event_<int>(5, 5, 5, true)));
}

TEST(CircTest, SizeMethod) {
  const ev::Circ_<int> circ(cv::Point_<int>(5, 5), 10);
  auto size = circ.size();
  EXPECT_EQ(size.width, 10);
  EXPECT_EQ(size.height, 10);
}
