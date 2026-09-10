#include "openev/containers/array.hpp"
#include "openev/containers/circular.hpp"
#include "openev/containers/deque.hpp"
#include "openev/containers/grid.hpp"
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

using EmptyContainerTypes = ::testing::Types<ev::Vector, ev::CircularBuffer, ev::Deque, ev::Queue>;
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

TEST(Entropy, QueuePreserves) {
  ev::Queue q;
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

TEST(Queue, StatisticsPreserveContents) {
  ev::Queue q;
  for(int i = 0; i < 100; i++) {
    q.emplace(i % 40, i % 30, i * 1e-3, i % 2);
  }

  (void)q.duration();
  (void)q.rate();
  (void)q.midTime();
  (void)q.mean();
  (void)q.meanPoint();
  (void)q.meanTime();
  (void)q.entropy();

  EXPECT_EQ(q.size(), 100U);
  EXPECT_EQ(q.front(), ev::Event(0, 0, 0.0, false));
  EXPECT_EQ(q.back(), ev::Event(99 % 40, 99 % 30, 99e-3, true));
}

TEST(SlidingWindow, TinyWindowKeepsLastEvent) {
  ev::SlidingWindow window(1e-12);
  for(int i = 0; i < 100; i++) {
    window.push(ev::Event(i, i, i * 1.0, true));
    ASSERT_EQ(window.size(), 1U);
    EXPECT_EQ(window.back(), ev::Event(i, i, i * 1.0, true));
  }
}

TEST(SlidingWindow, EqualTimestampsAreAllRetained) {
  ev::SlidingWindow window(1e-12);
  for(int i = 0; i < 50; i++) {
    window.push(ev::Event(i, i, 7.0, true));
  }
  EXPECT_EQ(window.size(), 50U);
}

TEST(SlidingWindow, SetWindowOnEmptyDoesNotCrash) {
  ev::SlidingWindow window(5.0);
  EXPECT_TRUE(window.empty());
  window.setWindow(1.0);
  EXPECT_TRUE(window.empty());
}

template <typename Grid>
Grid makeGrid(const cv::Size sensor = cv::Size(640, 480), const cv::Size cells = cv::Size(4, 3)) {
  if constexpr(std::is_same_v<Grid, ev::Grid_<ev::CircularBuffer>>) {
    return Grid(sensor, cells, 16);
  } else if constexpr(std::is_same_v<Grid, ev::Grid_<ev::SlidingWindow>>) {
    return Grid(sensor, cells, 100.0);
  } else {
    return Grid(sensor, cells);
  }
}

template <typename Grid>
class GridTestFixture : public ::testing::Test {
protected:
  Grid grid = makeGrid<Grid>();
};

using GridTypes = ::testing::Types<ev::Grid_<ev::Vector>, ev::Grid_<ev::Deque>, ev::Grid_<ev::Queue>, ev::Grid_<ev::CircularBuffer>, ev::Grid_<ev::SlidingWindow>>;
TYPED_TEST_SUITE(GridTestFixture, GridTypes);

TYPED_TEST(GridTestFixture, ShapeMatchesConstructor) {
  EXPECT_EQ(this->grid.rows(), 3);
  EXPECT_EQ(this->grid.cols(), 4);
  EXPECT_EQ(this->grid.size(), cv::Size(4, 3));
  EXPECT_EQ(this->grid.sensorSize(), cv::Size(640, 480));
  EXPECT_EQ(this->grid.cellSize(), cv::Size(160, 160));
}

TYPED_TEST(GridTestFixture, CellsStartEmpty) {
  for(const auto &cell : this->grid) {
    EXPECT_TRUE(cell.empty());
  }
}

TYPED_TEST(GridTestFixture, InsertGoesToItsCell) {
  EXPECT_TRUE(this->grid.insert(ev::Event(0, 0, 1.0, true)));
  EXPECT_TRUE(this->grid.insert(ev::Event(639, 479, 2.0, false)));
  EXPECT_TRUE(this->grid.insert(ev::Event(160, 159, 3.0, true)));
  EXPECT_EQ(this->grid(0, 0).size(), 1U);
  EXPECT_EQ(this->grid(2, 3).size(), 1U);
  EXPECT_EQ(this->grid(0, 1).size(), 1U);
  EXPECT_EQ(this->grid(1, 1).size(), 0U);
}

TYPED_TEST(GridTestFixture, EventOutsideIsRejected) {
  EXPECT_FALSE(this->grid.insert(ev::Event(640, 0, 1.0, true)));
  EXPECT_FALSE(this->grid.insert(ev::Event(0, 480, 1.0, true)));
  EXPECT_FALSE(this->grid.insert(ev::Event(-1, 0, 1.0, true)));
  EXPECT_FALSE(this->grid.insert(ev::Event(0, -1, 1.0, true)));
  for(const auto &cell : this->grid) {
    EXPECT_TRUE(cell.empty());
  }
}

TYPED_TEST(GridTestFixture, CellOfEvent) {
  EXPECT_EQ(this->grid.cell(ev::Event(0, 0)), cv::Point(0, 0));
  EXPECT_EQ(this->grid.cell(ev::Event(159, 159)), cv::Point(0, 0));
  EXPECT_EQ(this->grid.cell(ev::Event(160, 160)), cv::Point(1, 1));
  EXPECT_EQ(this->grid.cell(ev::Event(639, 479)), cv::Point(3, 2));
  EXPECT_EQ(this->grid.cell(ev::Event(640, 479)), cv::Point(-1, -1));
}

TYPED_TEST(GridTestFixture, AccessByPointMatchesAccessByIndex) {
  this->grid.insert(ev::Event(500, 300, 1.0, true));
  EXPECT_EQ(this->grid(cv::Point(3, 1)).size(), 1U);
  EXPECT_EQ(&this->grid(cv::Point(3, 1)), &this->grid(1, 3));
}


TYPED_TEST(GridTestFixture, ClearEmptiesEveryCell) {
  this->grid.insert(ev::Event(1, 1, 1.0, true));
  this->grid.insert(ev::Event(639, 479, 2.0, true));
  this->grid.clear();
  for(const auto &cell : this->grid) {
    EXPECT_TRUE(cell.empty());
  }
}

TYPED_TEST(GridTestFixture, IncompleteLastCellsCoverTheSensor) {
  TypeParam grid = makeGrid<TypeParam>(cv::Size(7, 5), cv::Size(3, 2));
  EXPECT_EQ(grid.cellSize(), cv::Size(3, 3));
  EXPECT_EQ(grid.cell(ev::Event(6, 4)), cv::Point(2, 1));
  EXPECT_EQ(grid.cell(ev::Event(5, 2)), cv::Point(1, 0));
  EXPECT_TRUE(grid.insert(ev::Event(6, 4, 1.0, true)));
  EXPECT_EQ(grid(1, 2).size(), 1U);
}

TYPED_TEST(GridTestFixture, SingleCellHoldsEverything) {
  TypeParam grid = makeGrid<TypeParam>(cv::Size(640, 480), cv::Size(1, 1));
  EXPECT_EQ(grid.cellSize(), cv::Size(640, 480));
  EXPECT_TRUE(grid.insert(ev::Event(0, 0, 1.0, true)));
  EXPECT_TRUE(grid.insert(ev::Event(639, 479, 2.0, true)));
  EXPECT_EQ(grid(0, 0).size(), 2U);
}

TYPED_TEST(GridTestFixture, InvalidGeometryThrows) {
  EXPECT_THROW(makeGrid<TypeParam>(cv::Size(0, 480), cv::Size(4, 3)), cv::Exception);
  EXPECT_THROW(makeGrid<TypeParam>(cv::Size(640, 480), cv::Size(0, 3)), cv::Exception);
  EXPECT_THROW(makeGrid<TypeParam>(cv::Size(640, 480), cv::Size(641, 3)), cv::Exception);
  EXPECT_THROW(makeGrid<TypeParam>(cv::Size(640, 480), cv::Size(4, 481)), cv::Exception);
}

TEST(Grid, CircularCellsKeepTheirCapacity) {
  ev::Grid_<ev::CircularBuffer> grid(cv::Size(640, 480), cv::Size(4, 3), 2);
  EXPECT_TRUE(grid.insert(ev::Event(1, 1, 1.0, true)));
  EXPECT_TRUE(grid.insert(ev::Event(1, 1, 2.0, true)));
  EXPECT_TRUE(grid.insert(ev::Event(1, 1, 3.0, true)));
  EXPECT_EQ(grid(0, 0).size(), 2U);
  EXPECT_EQ(grid(0, 0).front(), ev::Event(1, 1, 2.0, true));
  grid.clear();
  EXPECT_EQ(grid(0, 0).capacity(), 2U);
}

TEST(Grid, SlidingWindowCellsEvictExpiredEvents) {
  ev::Grid_<ev::SlidingWindow> grid(cv::Size(640, 480), cv::Size(4, 3), 1.5);
  EXPECT_TRUE(grid.insert(ev::Event(1, 1, 1.0, true)));
  EXPECT_TRUE(grid.insert(ev::Event(1, 1, 3.0, true)));
  EXPECT_EQ(grid(0, 0).size(), 1U);
  EXPECT_EQ(grid(0, 0).back(), ev::Event(1, 1, 3.0, true));
  grid.clear();
  EXPECT_DOUBLE_EQ(grid(0, 0).window(), 1.5);
}

TEST(Grid, FloatCoordinatesAreRounded) {
  ev::Grid_<ev::Vectorf> grid(cv::Size(640, 480), cv::Size(4, 3));
  EXPECT_EQ(grid.cell(ev::Eventf(159.4f, 0.0f)), cv::Point(0, 0));
  EXPECT_EQ(grid.cell(ev::Eventf(159.6f, 0.0f)), cv::Point(1, 0));
  EXPECT_TRUE(grid.insert(ev::Eventf(-0.4f, 479.4f, 1.0, true)));
  EXPECT_FALSE(grid.insert(ev::Eventf(-0.6f, 0.0f, 1.0, true)));
  EXPECT_FALSE(grid.insert(ev::Eventf(639.6f, 0.0f, 1.0, true)));
  EXPECT_EQ(grid(2, 0).size(), 1U);
}

TEST(Grid, CellStatisticsAreAvailable) {
  ev::Grid_<ev::Vector> grid(cv::Size(640, 480), cv::Size(4, 3));
  grid.insert(ev::Event(10, 10, 1.0, true));
  grid.insert(ev::Event(20, 20, 3.0, true));
  grid.insert(ev::Event(630, 470, 5.0, false));
  EXPECT_DOUBLE_EQ(grid(0, 0).duration(), 2.0);
  EXPECT_DOUBLE_EQ(grid(0, 0).meanPoint().x, 15.0);
  EXPECT_THROW((void)grid(2, 3).rate(), cv::Exception);
  EXPECT_THROW((void)grid(1, 1).duration(), cv::Exception);
}
