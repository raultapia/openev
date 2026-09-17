#include "openev/containers/array.hpp"
#include "openev/containers/circular.hpp"
#include "openev/containers/deque.hpp"
#include "openev/containers/grid.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/sliding_window.hpp"
#include "openev/containers/stats_container.hpp"
#include "openev/containers/vector.hpp"
#include <array>
#include <cmath>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <type_traits>

template <typename Container>
class ContainerTestFixture : public ::testing::Test {
protected:
  Container container;

  void SetUp() override {
    if constexpr(std::is_same_v<Container, ev::Vector> || std::is_same_v<Container, ev::CircularBuffer> || std::is_same_v<Container, ev::Deque> || std::is_same_v<Container, ev::SlidingWindow>) {
      container.resize(3);
    }
    container[0] = ev::Event(34, 10, 1.2143, true);
    container[1] = ev::Event(45, 14, 3.2342, false);
    container[2] = ev::Event(87, 23, 5.3432, true);
  }
};

using ContainerTypes = ::testing::Types<ev::Array<3>, ev::Vector, ev::CircularBuffer, ev::Deque, ev::SlidingWindow>;
TYPED_TEST_SUITE(ContainerTestFixture, ContainerTypes);

TYPED_TEST(ContainerTestFixture, Duration) {
  const double duration = this->container.duration();
  EXPECT_DOUBLE_EQ(duration, 5.3432 - 1.2143);
}

TYPED_TEST(ContainerTestFixture, Rate) {
  const double rate = this->container.rate();
  EXPECT_DOUBLE_EQ(rate, 3.0 / (5.3432 - 1.2143));
}

TYPED_TEST(ContainerTestFixture, Density) {
  EXPECT_DOUBLE_EQ(this->container.density(cv::Size(8, 6)), 3.0 / 48.0);
  EXPECT_DOUBLE_EQ(this->container.density(cv::Size(640, 480)), 3.0 / 307200.0);
}

TYPED_TEST(ContainerTestFixture, BoundingBox) {
  EXPECT_EQ(this->container.boundingBox(), cv::Rect(34, 10, 54, 14));
}

TYPED_TEST(ContainerTestFixture, ActivePixels) {
  EXPECT_EQ(this->container.activePixels(), 3U);
}

TYPED_TEST(ContainerTestFixture, Peak) {
  EXPECT_EQ(this->container.peak(), 1U);
}

TYPED_TEST(ContainerTestFixture, FillRatio) {
  EXPECT_DOUBLE_EQ(this->container.fillRatio(cv::Size(100, 30)), 3.0 / 3000.0);
}

TYPED_TEST(ContainerTestFixture, Covariance) {
  const double mx = (34 + 45 + 87) / 3.0;
  const double my = (10 + 14 + 23) / 3.0;
  const double xx = ((34 - mx) * (34 - mx) + (45 - mx) * (45 - mx) + (87 - mx) * (87 - mx)) / 3.0;
  const double yy = ((10 - my) * (10 - my) + (14 - my) * (14 - my) + (23 - my) * (23 - my)) / 3.0;
  const double xy = ((34 - mx) * (10 - my) + (45 - mx) * (14 - my) + (87 - mx) * (23 - my)) / 3.0;
  const cv::Matx22d c = this->container.covariance();
  EXPECT_DOUBLE_EQ(c(0, 0), xx);
  EXPECT_DOUBLE_EQ(c(1, 1), yy);
  EXPECT_DOUBLE_EQ(c(0, 1), xy);
  EXPECT_DOUBLE_EQ(c(1, 0), xy);
}

TYPED_TEST(ContainerTestFixture, PolarityRatio) {
  EXPECT_DOUBLE_EQ(this->container.polarityRatio(ev::POSITIVE), 2.0 / 3.0);
  EXPECT_DOUBLE_EQ(this->container.polarityRatio(ev::NEGATIVE), 1.0 / 3.0);
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

TEST(Queue, IsAPlainFifo) {
  ev::Queue queue;
  queue.push(ev::Event(10, 20, 1.0, true));
  queue.emplace(30, 40, 2.0, false);
  ASSERT_EQ(queue.size(), 2U);
  EXPECT_EQ(queue.front(), ev::Event(10, 20, 1.0, true));
  EXPECT_EQ(queue.back(), ev::Event(30, 40, 2.0, false));
  queue.pop();
  EXPECT_EQ(queue.front(), ev::Event(30, 40, 2.0, false));
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

TEST(ZeroSpan, SingleEventRateIsInfinite) {
  ev::Vector v;
  v.emplace_back(1, 1, 5.0, true);
  EXPECT_DOUBLE_EQ(v.duration(), 0.0);
  EXPECT_TRUE(std::isinf(v.rate()));
}

TEST(ZeroSpan, SimultaneousEventsRateIsInfinite) {
  ev::Vector v;
  for(int i = 0; i < 100; i++) {
    v.emplace_back(i, i, 7.0, true);
  }
  EXPECT_DOUBLE_EQ(v.duration(), 0.0);
  EXPECT_TRUE(std::isinf(v.rate()));
}

TEST(ZeroSpan, DefaultArrayRateIsInfinite) {
  const ev::Array<3> a;
  EXPECT_DOUBLE_EQ(a.duration(), 0.0);
  EXPECT_TRUE(std::isinf(a.rate()));
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

TEST(Entropy, SparseTwoPixelsIsOneBit) {
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

using GridTypes = ::testing::Types<ev::Grid_<ev::Vector>, ev::Grid_<ev::Deque>, ev::Grid_<ev::Queue>, ev::Grid_<ev::CircularBuffer>, ev::Grid_<ev::SlidingWindow>, ev::Grid_<ev::StatsContainer>>;
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
}

TEST(Grid, CellsHoldSensorCoordinates) {
  ev::Grid_<ev::Vector> grid(cv::Size(640, 480), cv::Size(4, 3));
  EXPECT_TRUE(grid.insert(ev::Event(500, 330, 1.0, true)));
  EXPECT_EQ(grid(2, 3).front(), ev::Event(500, 330, 1.0, true));
  EXPECT_TRUE(grid.insert(ev::Event(0, 0, 2.0, false)));
  EXPECT_EQ(grid(0, 0).front(), ev::Event(0, 0, 2.0, false));
}

TEST(Grid, FloatSensorCoordinates) {
  ev::Grid_<ev::Vectorf> grid(cv::Size(640, 480), cv::Size(4, 3));
  EXPECT_TRUE(grid.insert(ev::Eventf(500.5f, 330.25f, 1.0, true)));
  EXPECT_FLOAT_EQ(grid(2, 3).front().x, 500.5f);
  EXPECT_FLOAT_EQ(grid(2, 3).front().y, 330.25f);
}

TEST(Grid, ClearKeepsTheCellsInPlace) {
  ev::Grid_<ev::SlidingWindow> grid(cv::Size(640, 480), cv::Size(4, 3), 100.0);
  grid.insert(ev::Event(1, 1, 1.0, true));
  const ev::SlidingWindow *before = &grid(0, 0);
  grid.clear();
  EXPECT_EQ(&grid(0, 0), before);
  EXPECT_TRUE(grid(0, 0).empty());
  EXPECT_DOUBLE_EQ(grid(0, 0).window(), 100.0);
}

TEST(Grid, ClearRebuildsCellsWithoutClear) {
  struct Counter {
    using value_type = ev::Event;
    int n = 0;
    void push(const ev::Event &) {
      n++;
    }
  };
  ev::Grid_<Counter> grid(cv::Size(640, 480), cv::Size(4, 3));
  grid.insert(ev::Event(1, 1, 1.0, true));
  EXPECT_EQ(grid(0, 0).n, 1);
  grid.clear();
  EXPECT_EQ(grid(0, 0).n, 0);
}

TEST(Grid, PrototypeIsCopiedToEveryCell) {
  ev::Grid_<ev::CircularBuffer> grid(cv::Size(640, 480), cv::Size(4, 3), 16);
  for(const auto &cell : grid) {
    EXPECT_EQ(cell.capacity(), 16U);
  }
  grid.clear();
  for(const auto &cell : grid) {
    EXPECT_EQ(cell.capacity(), 16U);
  }
}

class StatsContainerTest : public ::testing::Test {
protected:
  ev::StatsContainer stats;
  ev::Vector vector;

  void SetUp() override {
    std::mt19937 rng(7);
    std::uniform_int_distribution<int> x(0, 639);
    std::uniform_int_distribution<int> y(0, 479);
    std::bernoulli_distribution p(0.6);
    for(int i = 0; i < 20000; i++) {
      const ev::Event e(x(rng), y(rng), 1.0 + i * 1e-3, p(rng));
      vector.push_back(e);
      stats.push(e);
    }
  }
};

TEST_F(StatsContainerTest, SizeMatchesInsertions) {
  EXPECT_EQ(stats.size(), 20000U);
  EXPECT_FALSE(stats.empty());
}

TEST_F(StatsContainerTest, DurationRateDensityAndMidTimeMatchVector) {
  EXPECT_DOUBLE_EQ(stats.duration(), vector.duration());
  EXPECT_DOUBLE_EQ(stats.rate(), vector.rate());
  EXPECT_DOUBLE_EQ(stats.density(cv::Size(640, 480)), vector.density(cv::Size(640, 480)));
  EXPECT_DOUBLE_EQ(stats.density(cv::Size(640, 480)), 20000.0 / 307200.0);
  EXPECT_DOUBLE_EQ(stats.midTime(), vector.midTime());
}

TEST_F(StatsContainerTest, MeansAndPolarityRatioMatchVector) {
  const ev::Eventd a = stats.mean();
  const ev::Eventd b = vector.mean();
  EXPECT_NEAR(a.x, b.x, 1e-9);
  EXPECT_NEAR(a.y, b.y, 1e-9);
  EXPECT_NEAR(a.t, b.t, 1e-9);
  EXPECT_EQ(a.p, b.p);
  EXPECT_NEAR(stats.meanPoint().x, vector.meanPoint().x, 1e-9);
  EXPECT_NEAR(stats.meanPoint().y, vector.meanPoint().y, 1e-9);
  EXPECT_NEAR(stats.meanTime(), vector.meanTime(), 1e-9);
  EXPECT_NEAR(stats.polarityRatio(ev::POSITIVE), vector.polarityRatio(ev::POSITIVE), 1e-12);
  EXPECT_NEAR(stats.polarityRatio(ev::NEGATIVE), vector.polarityRatio(ev::NEGATIVE), 1e-12);
  EXPECT_NEAR(stats.polarityRatio(ev::POSITIVE) + stats.polarityRatio(ev::NEGATIVE), 1.0, 1e-12);
  EXPECT_GT(stats.polarityRatio(ev::POSITIVE), 0.55);
  EXPECT_LT(stats.polarityRatio(ev::POSITIVE), 0.65);
}

TEST_F(StatsContainerTest, EntropyMatchesVector) {
  EXPECT_NEAR(stats.entropy(), vector.entropy(), 1e-9);
}

TEST_F(StatsContainerTest, SpatialStatisticsMatchVector) {
  EXPECT_EQ(stats.boundingBox(), vector.boundingBox());
  EXPECT_EQ(stats.boundingBox() & cv::Rect(0, 0, 640, 480), stats.boundingBox());
  EXPECT_EQ(stats.activePixels(), vector.activePixels());
  EXPECT_EQ(stats.peak(), vector.peak());
  EXPECT_GE(stats.peak(), 2U);
  EXPECT_GT(stats.activePixels(), 19000U);
  EXPECT_LT(stats.activePixels(), 20000U);
  EXPECT_DOUBLE_EQ(stats.fillRatio(cv::Size(640, 480)), vector.fillRatio(cv::Size(640, 480)));
  EXPECT_DOUBLE_EQ(stats.fillRatio(cv::Size(640, 480)), static_cast<double>(stats.activePixels()) / 307200.0);
}

TEST_F(StatsContainerTest, CovarianceMatchesVector) {
  const cv::Matx22d a = stats.covariance();
  const cv::Matx22d b = vector.covariance();
  EXPECT_NEAR(a(0, 0), b(0, 0), 1e-6);
  EXPECT_NEAR(a(1, 1), b(1, 1), 1e-6);
  EXPECT_NEAR(a(0, 1), b(0, 1), 1e-6);
}

TEST_F(StatsContainerTest, ClearForgetsEverything) {
  stats.clear();
  EXPECT_TRUE(stats.empty());
  stats.push(ev::Event(5, 5, 1.0, true));
  stats.push(ev::Event(5, 5, 2.0, true));
  EXPECT_DOUBLE_EQ(stats.entropy(), 0.0);
  EXPECT_DOUBLE_EQ(stats.duration(), 1.0);
}

TEST(StatsContainer, ZeroSpanRateIsInfinite) {
  ev::StatsContainer stats;
  stats.push(ev::Event(1, 1, 5.0, true));
  EXPECT_DOUBLE_EQ(stats.duration(), 0.0);
  EXPECT_TRUE(std::isinf(stats.rate()));
}

TEST(StatsContainer, EntropyOfKnownDistributions) {
  ev::StatsContainer stats;
  for(int i = 0; i < 100; i++) {
    stats.push(ev::Event(i % 2, 0, i * 1e-3, true));
  }
  EXPECT_NEAR(stats.entropy(), 1.0, 1e-12);
  stats.clear();
  for(int i = 0; i < 1600; i++) {
    stats.push(ev::Event(i % 4, (i / 4) % 4, i * 1e-3, true));
  }
  EXPECT_NEAR(stats.entropy(), 4.0, 1e-12);
}

TEST(StatsContainer, FloatCoordinatesAreRounded) {
  ev::StatsContainerf stats;
  ev::Vectorf vector;
  vector.emplace_back(0.4f, 0.4f, 1.0, true);
  vector.emplace_back(0.6f, 0.6f, 2.0, true);
  vector.emplace_back(0.4f, 0.4f, 3.0, false);
  for(const ev::Eventf &e : vector) {
    stats.push(e);
  }
  EXPECT_NEAR(stats.entropy(), vector.entropy(), 1e-12);
  EXPECT_NEAR(stats.meanPoint().x, vector.meanPoint().x, 1e-6);
  EXPECT_EQ(stats.boundingBox(), cv::Rect(0, 0, 2, 2));
  EXPECT_EQ(stats.boundingBox(), vector.boundingBox());
  EXPECT_EQ(stats.activePixels(), 2U);
}

TEST(StatsContainer, WorksAsGridCell) {
  ev::Grid_<ev::StatsContainer> grid(cv::Size(640, 480), cv::Size(4, 3));
  EXPECT_TRUE(grid.insert(ev::Event(10, 10, 1.0, true)));
  EXPECT_TRUE(grid.insert(ev::Event(20, 20, 3.0, false)));
  EXPECT_TRUE(grid.insert(ev::Event(639, 479, 5.0, true)));
  EXPECT_EQ(grid(0, 0).size(), 2U);
  EXPECT_DOUBLE_EQ(grid(0, 0).duration(), 2.0);
  EXPECT_DOUBLE_EQ(grid(0, 0).entropy(), 1.0);
  EXPECT_EQ(grid(2, 3).size(), 1U);
  EXPECT_DOUBLE_EQ(grid(2, 3).meanPoint().x, 639.0);
  EXPECT_DOUBLE_EQ(grid(2, 3).meanPoint().y, 479.0);
  grid.clear();
  EXPECT_TRUE(grid(0, 0).empty());
}

TEST(StatsContainer, EntropyBeyondTheTabulatedCounts) {
  ev::StatsContainer stats;
  ev::Vector vector;
  for(int i = 0; i < 10000; i++) {
    const ev::Event e(i % 3 == 0 ? 1 : 0, 0, i * 1e-3, true);
    stats.push(e);
    vector.push_back(e);
  }
  EXPECT_NEAR(stats.entropy(), vector.entropy(), 1e-9);
}

TEST(StatsContainer, FirstEventBoundsTheBox) {
  ev::StatsContainer stats;
  stats.push(ev::Event(300, 200, 1.0, true));
  EXPECT_EQ(stats.boundingBox(), cv::Rect(300, 200, 1, 1));
  EXPECT_DOUBLE_EQ(stats.entropy(), 0.0);
}

TEST(StatsContainer, BoxGrowsInEveryDirection) {
  ev::StatsContainer stats;
  ev::Vector vector;
  const std::array<ev::Event, 6> events{ev::Event(100, 100, 1.0, true), ev::Event(130, 100, 2.0, true), ev::Event(100, 140, 3.0, false), ev::Event(20, 100, 4.0, true), ev::Event(100, 5, 5.0, false), ev::Event(-40, -30, 6.0, true)};
  for(const ev::Event &e : events) {
    stats.push(e);
    vector.push_back(e);
    EXPECT_TRUE(stats.boundingBox().contains(e));
  }
  EXPECT_EQ(stats.boundingBox(), cv::Rect(-40, -30, 171, 171));
  EXPECT_NEAR(stats.entropy(), vector.entropy(), 1e-12);
  EXPECT_NEAR(stats.meanPoint().x, vector.meanPoint().x, 1e-9);
}

TEST(StatsContainer, GrowthKeepsTheCounts) {
  ev::StatsContainer stats;
  ev::Vector vector;
  std::mt19937 rng(3);
  std::uniform_int_distribution<int> x(0, 639);
  std::uniform_int_distribution<int> y(0, 479);
  for(int i = 0; i < 5000; i++) {
    const ev::Event e(x(rng), y(rng), i * 1e-3, true);
    stats.push(e);
    vector.push_back(e);
  }
  for(int i = 0; i < 5000; i++) {
    const ev::Event e(5, 5, 5.0 + i * 1e-3, true);
    stats.push(e);
    vector.push_back(e);
  }
  EXPECT_NEAR(stats.entropy(), vector.entropy(), 1e-9);
}

TEST(StatsContainer, ClearThenPushStartsAgain) {
  ev::StatsContainer stats;
  stats.push(ev::Event(0, 0, 1.0, true));
  stats.push(ev::Event(639, 479, 2.0, true));
  stats.clear();
  EXPECT_TRUE(stats.empty());
  stats.push(ev::Event(10, 10, 3.0, true));
  EXPECT_EQ(stats.boundingBox(), cv::Rect(10, 10, 1, 1));
  EXPECT_EQ(stats.activePixels(), 1U);
  EXPECT_DOUBLE_EQ(stats.entropy(), 0.0);
}

TEST(StatsContainer, ActivePixelsCountsEachPixelOnce) {
  ev::StatsContainer stats;
  for(int i = 0; i < 100; i++) {
    stats.push(ev::Event(i % 4, (i / 4) % 4, i * 1e-3, true));
  }
  EXPECT_EQ(stats.activePixels(), 16U);
  EXPECT_EQ(stats.boundingBox(), cv::Rect(0, 0, 4, 4));
  EXPECT_EQ(stats.peak(), 7U);
  stats.clear();
  stats.push(ev::Event(7, 7, 1.0, true));
  EXPECT_EQ(stats.activePixels(), 1U);
  EXPECT_EQ(stats.boundingBox(), cv::Rect(7, 7, 1, 1));
  EXPECT_DOUBLE_EQ(stats.covariance()(0, 0), 0.0);
}

TEST(StatsContainer, HotPixelDominatesThePeak) {
  ev::Vector v;
  for(int i = 0; i < 50; i++) {
    v.emplace_back(i, i, i * 1e-3, true);
  }
  for(int i = 0; i < 200; i++) {
    v.emplace_back(7, 7, 1.0 + i * 1e-3, true);
  }
  EXPECT_EQ(v.peak(), 201U);
  ev::StatsContainer stats;
  for(const ev::Event &e : v) {
    stats.push(e);
  }
  EXPECT_EQ(stats.peak(), 201U);
  stats.clear();
  EXPECT_EQ(stats.peak(), 0U);
}
