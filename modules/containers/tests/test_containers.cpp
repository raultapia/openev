#include "openev/containers/array.hpp"
#include "openev/containers/circular.hpp"
#include "openev/containers/deque.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/core/types.hpp"
#include <gtest/gtest.h>
#include <opencv2/core/types.hpp>
#include <string>
#include <type_traits>

template <typename Container>
class ContainerTestFixture : public ::testing::Test {
protected:
  Container container;

  void SetUp() override {
    if constexpr(std::is_same_v<Container, ev::Vector> || std::is_same_v<Container, ev::CircularBuffer> || std::is_same_v<Container, ev::Deque>) {
      container.resize(3);
    }
    if constexpr(std::is_same_v<Container, ev::Queue>) {
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

using ContainerTypes = ::testing::Types<ev::Array<3>, ev::Vector, ev::CircularBuffer, ev::Deque, ev::Queue>;
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
