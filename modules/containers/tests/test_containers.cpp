#include "openev/containers/array.hpp"
#include "openev/containers/circular.hpp"
#include "openev/containers/deque.hpp"
#include "openev/containers/persistent_queue.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/sliding_window.hpp"
#include "openev/containers/vector.hpp"
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

template <typename Container>
class ContainerTestFixture : public ::testing::Test {
protected:
  Container container;

  void SetUp() override {
    if constexpr(std::is_same_v<Container, ev::Vector> || std::is_same_v<Container, ev::CircularBuffer> || std::is_same_v<Container, ev::Deque>) {
      container.resize(3);
    }
    if constexpr(std::is_same_v<Container, ev::Queue> || std::is_same_v<Container, ev::PersistentQueue>) {
      container.push(ev::Event(34, 10, 1.2143, true));
      container.push(ev::Event(45, 14, 3.2342, false));
      container.push(ev::Event(87, 23, 5.3432, true));
    } else {
      container[0] = ev::Event(34, 10, 1.2143, true);
      container[1] = ev::Event(45, 14, 3.2342, false);
      container[2] = ev::Event(87, 23, 5.3432, true);
    }
  }
};

using ContainerTypes = ::testing::Types<ev::Array<3>, ev::Vector, ev::CircularBuffer, ev::Deque, ev::PersistentQueue, ev::Queue>;
TYPED_TEST_SUITE(ContainerTestFixture, ContainerTypes);

TYPED_TEST(ContainerTestFixture, Duration) {
  const double duration = this->container.duration();
  EXPECT_DOUBLE_EQ(duration, 5.3432 - 1.2143);
}

TYPED_TEST(ContainerTestFixture, Rate) {
  const double rate = this->container.rate();
  EXPECT_DOUBLE_EQ(rate, 3.0 / (5.3432 - 1.2143));
}

TYPED_TEST(ContainerTestFixture, Mean) {
  const ev::Eventd mean = this->container.mean();
  EXPECT_DOUBLE_EQ(mean.x, (34 + 45 + 87) / 3.0);
  EXPECT_DOUBLE_EQ(mean.y, (10 + 14 + 23) / 3.0);
  EXPECT_DOUBLE_EQ(mean.t, (1.2143 + 3.2342 + 5.3432) / 3.0);
  EXPECT_TRUE(mean.p);
}

TYPED_TEST(ContainerTestFixture, MeanPoint) {
  const cv::Point2d meanPoint = this->container.meanPoint();
  EXPECT_DOUBLE_EQ(meanPoint.x, (34 + 45 + 87) / 3.0);
  EXPECT_DOUBLE_EQ(meanPoint.y, (10 + 14 + 23) / 3.0);
}

TYPED_TEST(ContainerTestFixture, MeanTime) {
  const double meanTime = this->container.meanTime();
  EXPECT_DOUBLE_EQ(meanTime, (1.2143 + 3.2342 + 5.3432) / 3.0);
}

TYPED_TEST(ContainerTestFixture, MidTime) {
  const double midTime = this->container.midTime();
  EXPECT_DOUBLE_EQ(midTime, (1.2143 + 5.3432) / 2.0);
}

TEST(CircularBuffer, EmplaceFront) {
  ev::CircularBuffer buffer(2);
  buffer.emplace_front(10, 20, 1.0, true);
  buffer.emplace_front(30, 40, 2.0, false);
  EXPECT_EQ(buffer[0], ev::Event(30, 40, 2.0, false));
  EXPECT_EQ(buffer[1], ev::Event(10, 20, 1.0, true));
}

TEST(CircularBuffer, EmplaceBack) {
  ev::CircularBuffer buffer(2);
  buffer.emplace_back(10, 20, 1.0, true);
  buffer.emplace_back(30, 40, 2.0, false);
  EXPECT_EQ(buffer[0], ev::Event(10, 20, 1.0, true));
  EXPECT_EQ(buffer[1], ev::Event(30, 40, 2.0, false));
}

class SlidingWindowTest : public ::testing::Test {
protected:
  ev::SlidingWindow window{2.5};
};

TEST_F(SlidingWindowTest, PushEvictsExpiredEvents) {
  window.push(ev::Event(10, 20, 1.0, true));
  window.push(ev::Event(11, 21, 2.0, false));
  window.push(ev::Event(12, 22, 3.0, true));
  window.push(ev::Event(13, 23, 4.0, false));

  ASSERT_EQ(window.size(), 3U);
  EXPECT_EQ(window.front(), ev::Event(11, 21, 2.0, false));
  EXPECT_EQ(window.back(), ev::Event(13, 23, 4.0, false));
}

TEST_F(SlidingWindowTest, EmplaceEvictsExpiredEvents) {
  window.emplace(10, 20, 1.0, true);
  window.emplace(11, 21, 2.0, false);
  window.emplace(12, 22, 4.0, true);

  ASSERT_EQ(window.size(), 2U);
  EXPECT_EQ(window.front(), ev::Event(11, 21, 2.0, false));
  EXPECT_EQ(window.back(), ev::Event(12, 22, 4.0, true));
}

TEST_F(SlidingWindowTest, WindowSetterRetainsOnlyRecentEvents) {
  window.push(ev::Event(10, 20, 1.0, true));
  window.push(ev::Event(11, 21, 2.0, false));
  window.push(ev::Event(12, 22, 3.0, true));
  window.push(ev::Event(13, 23, 4.0, false));

  window.setWindow(1.0);

  ASSERT_EQ(window.size(), 2U);
  EXPECT_EQ(window.front(), ev::Event(12, 22, 3.0, true));
  EXPECT_EQ(window.back(), ev::Event(13, 23, 4.0, false));
}

TEST_F(SlidingWindowTest, StatisticsOperateOnCurrentWindow) {
  window.push(ev::Event(34, 10, 1.2143, true));
  window.push(ev::Event(45, 14, 3.2342, false));
  window.push(ev::Event(87, 23, 5.3432, true));

  const double duration = window.duration();
  const double rate = window.rate();
  const ev::Eventd mean = window.mean();
  const cv::Point2d meanPoint = window.meanPoint();
  const double meanTime = window.meanTime();
  const double midTime = window.midTime();

  EXPECT_DOUBLE_EQ(duration, 5.3432 - 3.2342);
  EXPECT_DOUBLE_EQ(rate, 2.0 / (5.3432 - 3.2342));
  EXPECT_DOUBLE_EQ(mean.x, (45 + 87) / 2.0);
  EXPECT_DOUBLE_EQ(mean.y, (14 + 23) / 2.0);
  EXPECT_DOUBLE_EQ(mean.t, (3.2342 + 5.3432) / 2.0);
  EXPECT_FALSE(mean.p);
  EXPECT_DOUBLE_EQ(meanPoint.x, (45 + 87) / 2.0);
  EXPECT_DOUBLE_EQ(meanPoint.y, (14 + 23) / 2.0);
  EXPECT_DOUBLE_EQ(meanTime, (3.2342 + 5.3432) / 2.0);
  EXPECT_DOUBLE_EQ(midTime, (3.2342 + 5.3432) / 2.0);
}
