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

template <typename Container>
class EmptyContainerTestFixture : public ::testing::Test {
protected:
  Container container;
};

using EmptyContainerTypes = ::testing::Types<ev::Vector, ev::CircularBuffer, ev::Deque, ev::PersistentQueue, ev::Queue>;
TYPED_TEST_SUITE(EmptyContainerTestFixture, EmptyContainerTypes);

TYPED_TEST(EmptyContainerTestFixture, DurationThrows) {
  EXPECT_TRUE(this->container.empty());
  EXPECT_THROW((void)this->container.duration(), cv::Exception);
}

TYPED_TEST(EmptyContainerTestFixture, RateThrows) {
  EXPECT_THROW((void)this->container.rate(), cv::Exception);
}

TYPED_TEST(EmptyContainerTestFixture, MidTimeThrows) {
  EXPECT_THROW((void)this->container.midTime(), cv::Exception);
}

TYPED_TEST(EmptyContainerTestFixture, MeanThrows) {
  EXPECT_THROW((void)this->container.mean(), cv::Exception);
}

TYPED_TEST(EmptyContainerTestFixture, MeanPointThrows) {
  EXPECT_THROW((void)this->container.meanPoint(), cv::Exception);
}

TYPED_TEST(EmptyContainerTestFixture, MeanTimeThrows) {
  EXPECT_THROW((void)this->container.meanTime(), cv::Exception);
}

TYPED_TEST(EmptyContainerTestFixture, EntropyThrows) {
  EXPECT_THROW((void)this->container.entropy(), cv::Exception);
}

TEST(EmptyContainer, SlidingWindowThrows) {
  ev::SlidingWindow window(1.0);
  EXPECT_TRUE(window.empty());
  EXPECT_THROW((void)window.duration(), cv::Exception);
  EXPECT_THROW((void)window.mean(), cv::Exception);
}

TEST(ZeroSpan, SingleEventRateThrows) {
  ev::Vector v;
  v.emplace_back(1, 1, 5.0, true);
  EXPECT_DOUBLE_EQ(v.duration(), 0.0);
  EXPECT_THROW((void)v.rate(), cv::Exception);
}

TEST(ZeroSpan, SimultaneousEventsRateThrows) {
  ev::Vector v;
  for(int i = 0; i < 100; i++) {
    v.emplace_back(i, i, 7.0, true);
  }
  EXPECT_DOUBLE_EQ(v.duration(), 0.0);
  EXPECT_THROW((void)v.rate(), cv::Exception);
}

TEST(ZeroSpan, DefaultArrayRateThrows) {
  const ev::Array<3> a;
  EXPECT_DOUBLE_EQ(a.duration(), 0.0);
  EXPECT_THROW((void)a.rate(), cv::Exception);
}

TEST(SlidingWindow, DefaultWindowRetainsEverything) {
  ev::SlidingWindow window;
  EXPECT_DOUBLE_EQ(window.window(), 0.0);
  for(int i = 0; i < 100; i++) {
    window.push(ev::Event(i, i, i * 1e-3, true));
  }
  EXPECT_EQ(window.size(), 100U);
  EXPECT_DOUBLE_EQ(window.duration(), 99e-3);
}

TEST(SlidingWindow, NegativeWindowRetainsEverything) {
  ev::SlidingWindow window(-1.0);
  for(int i = 0; i < 10; i++) {
    window.push(ev::Event(i, i, i * 1e-3, true));
  }
  EXPECT_EQ(window.size(), 10U);
}

TEST(Entropy, SinglePixelIsZero) {
  ev::Vector v;
  for(int i = 0; i < 100; i++) {
    v.emplace_back(5, 5, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(v.entropy(), 0.0);
}

TEST(Entropy, TwoEqualPixelsIsOneBit) {
  ev::Vector v;
  for(int i = 0; i < 100; i++) {
    v.emplace_back(i % 2, 0, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(v.entropy(), 1.0);
}

TEST(Entropy, FourEqualPixelsIsTwoBits) {
  ev::Vector v;
  for(int i = 0; i < 100; i++) {
    v.emplace_back(i % 2, (i / 2) % 2, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(v.entropy(), 2.0);
}

TEST(Entropy, EffectivePixelsMatchesFootprint) {
  ev::Vector v;
  for(int i = 0; i < 1600; i++) {
    v.emplace_back(i % 4, (i / 4) % 4, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(std::pow(2.0, v.entropy()), 16.0);
}

TEST(Entropy, ConcentrationLowersEntropy) {
  ev::Vector spread;
  ev::Vector concentrated;
  for(int i = 0; i < 100; i++) {
    spread.emplace_back(i % 4, 0, i * 1e-3, true);
    concentrated.emplace_back(i < 97 ? 0 : i % 4, 0, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(spread.entropy(), 2.0);
  EXPECT_LT(concentrated.entropy(), spread.entropy());
}

TEST(Entropy, SamePixelDifferentTimeIsStillZero) {
  ev::Vector v;
  v.emplace_back(3, 7, 0.0, true);
  v.emplace_back(3, 7, 1.0, false);
  EXPECT_DOUBLE_EQ(v.entropy(), 0.0);
}

TEST(Entropy, FloatCoordinatesRoundToPixel) {
  ev::Vectorf v;
  v.emplace_back(1.4F, 1.4F, 0.0, true);
  v.emplace_back(1.0F, 1.0F, 1.0, true);
  EXPECT_DOUBLE_EQ(v.entropy(), 0.0);
}

TEST(Entropy, QueueDrains) {
  ev::Queue q;
  for(int i = 0; i < 100; i++) {
    q.emplace(i % 2, 0, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(q.entropy(), 1.0);
  EXPECT_TRUE(q.empty());
}

TEST(Entropy, PersistentQueuePreserves) {
  ev::PersistentQueue q;
  for(int i = 0; i < 100; i++) {
    q.emplace(i % 2, 0, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(q.entropy(), 1.0);
  EXPECT_EQ(q.size(), 100U);
  EXPECT_DOUBLE_EQ(q.entropy(), 1.0);
}

TEST(Entropy, SparseFallbackMatchesCompact) {
  ev::Vector compact;
  ev::Vector sparse;
  for(int i = 0; i < 100; i++) {
    compact.emplace_back(i % 2, (i / 2) % 2, i * 1e-3, true);
    sparse.emplace_back((i % 2) * 50000, ((i / 2) % 2) * 50000, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(compact.entropy(), 2.0);
  EXPECT_DOUBLE_EQ(sparse.entropy(), compact.entropy());
}

TEST(Entropy, SparseSinglePixelIsZero) {
  ev::Vector v;
  v.emplace_back(0, 0, 0.0, true);
  v.emplace_back(100000, 100000, 1.0, true);
  v.emplace_back(100000, 100000, 2.0, true);
  v.emplace_back(0, 0, 3.0, true);
  EXPECT_DOUBLE_EQ(v.entropy(), 1.0);
}

TEST(Entropy, NegativeCoordinatesAreDistinctPixels) {
  ev::Vector v;
  for(int i = 0; i < 100; i++) {
    v.emplace_back(i % 2 ? -1 : 1, 0, i * 1e-3, true);
  }
  EXPECT_DOUBLE_EQ(v.entropy(), 1.0);
}
