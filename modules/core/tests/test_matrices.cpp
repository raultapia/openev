#include "openev/core/matrices.hpp"
#include "openev/core/types.hpp"
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

// Test Mat_ Base Class
TEST(MatBaseTest, UpdateStatsCountDuration) {
  ev::Mat::Binary b(10, 10);
  b.updateStats(ev::Event(3, 4, 1.0f, ev::POSITIVE));
  b.updateStats(ev::Event(3, 4, 3.0f, ev::POSITIVE));
  b.updateStats(ev::Event(3, 4, 5.0f, ev::POSITIVE));
  EXPECT_EQ(b.count(), 2);
  EXPECT_FLOAT_EQ(b.duration(), 4.0f);
}

TEST(MatBaseTest, CountZeroBeforeAnyUpdate) {
  ev::Mat::Counter c(10, 10);
  EXPECT_EQ(c.count(), 0);
  EXPECT_FLOAT_EQ(c.duration(), 0.0f);
}

TEST(MatBaseTest, UpdateStatsSingleEvent) {
  ev::Mat::Binary b(10, 10);
  b.updateStats(ev::Event(0, 0, 2.0f, ev::POSITIVE));
  EXPECT_EQ(b.count(), 0);
}

TEST(MatBaseTest, ResetStats) {
  ev::Mat::Binary b(10, 10);
  b.updateStats(ev::Event(3, 4, 1.0f, ev::POSITIVE));
  b.updateStats(ev::Event(3, 4, 5.0f, ev::POSITIVE));
  b.resetStats();
  EXPECT_EQ(b.count(), 0);
  EXPECT_FLOAT_EQ(b.duration(), 0.0f);
}

// Test Binary Class
TEST(BinaryTest, Insert) {
  ev::Mat::Binary binary(10, 10);
  const ev::Event event(3, 4, 1.5, ev::POSITIVE);
  binary.insert(event);
  EXPECT_EQ(binary(4, 3), ev::Mat::Binary::ON);
}

TEST(BinaryTest, Emplace) {
  ev::Mat::Binary binary(10, 10);
  binary.emplace(5, 6);
  EXPECT_EQ(binary(6, 5), ev::Mat::Binary::ON);
}

TEST(BinaryTest, Clear) {
  ev::Mat::Binary binary(10, 10);
  binary.emplace(5, 6);
  binary.clear();
  EXPECT_EQ(binary(6, 5), ev::Mat::Binary::OFF);
}

TEST(BinaryTest, LargeMatrixStressTest) {
  ev::Mat::Binary binary(10000, 10000);
  binary.emplace(9999, 9999);
  EXPECT_EQ(binary(9999, 9999), ev::Mat::Binary::ON);
}

TEST(BinaryTest, StreamOperator) {
  const ev::Mat::Binary binary(20, 15);
  std::ostringstream oss;
  [[maybe_unused]] const auto &result = oss << binary;
  EXPECT_EQ(oss.str(), std::string("Binary 15x20"));
}

TEST(BinaryTest, InsertReturnValue) {
  ev::Mat::Binary binary(10, 10);
  EXPECT_EQ(binary.insert(ev::Event(3, 4, 1.0f, ev::POSITIVE)), ev::Mat::Binary::ON);
}

TEST(BinaryTest, EmplaceReturnValue) {
  ev::Mat::Binary binary(10, 10);
  EXPECT_EQ(binary.emplace(3, 4), ev::Mat::Binary::ON);
}

TEST(BinaryTest, FloatCoordinates) {
  ev::Mat::Binary binary(10, 10);
  binary.emplace(3.6f, 4.4f);
  EXPECT_EQ(binary(4, 4), ev::Mat::Binary::ON);
}

TEST(BinaryTest, MultiplePixelsIndependent) {
  ev::Mat::Binary binary(10, 10);
  binary.emplace(1, 1);
  binary.emplace(8, 8);
  EXPECT_EQ(binary(1, 1), ev::Mat::Binary::ON);
  EXPECT_EQ(binary(8, 8), ev::Mat::Binary::ON);
  EXPECT_EQ(binary(1, 8), ev::Mat::Binary::OFF);
}

// Test Ternary Class
TEST(TernaryTest, Insert) {
  ev::Mat::Ternary ternary(10, 10);

  const ev::Event event1(3, 4, 1.5, ev::POSITIVE);
  ternary.insert(event1);
  EXPECT_EQ(ternary(4, 3), ev::Mat::Ternary::POSITIVE);

  const ev::Event event2(3, 4, 1.5, ev::NEGATIVE);
  ternary.insert(event2);
  EXPECT_EQ(ternary(4, 3), ev::Mat::Ternary::NEGATIVE);
}

TEST(TernaryTest, Emplace) {
  ev::Mat::Ternary ternary(10, 10);

  ternary.emplace(5, 6, ev::POSITIVE);
  EXPECT_EQ(ternary(6, 5), ev::Mat::Ternary::POSITIVE);

  ternary.emplace(5, 6, ev::NEGATIVE);
  EXPECT_EQ(ternary(6, 5), ev::Mat::Ternary::NEGATIVE);
}

TEST(TernaryTest, Clear) {
  ev::Mat::Ternary ternary(10, 10);

  ternary.emplace(5, 6, ev::POSITIVE);
  ternary.clear();
  EXPECT_EQ(ternary(6, 5), ev::Mat::Ternary::ZERO);

  ternary.emplace(5, 6, ev::NEGATIVE);
  ternary.clear();
  EXPECT_EQ(ternary(6, 5), ev::Mat::Ternary::ZERO);
}

TEST(TernaryTest, LargeMatrixStressTest) {
  ev::Mat::Ternary ternary(10000, 10000);
  ternary.emplace(9999, 9999, ev::POSITIVE);
  EXPECT_EQ(ternary(9999, 9999), ev::Mat::Ternary::POSITIVE);
}

TEST(TernaryTest, StreamOperator) {
  const ev::Mat::Ternary ternary(20, 15);
  std::ostringstream oss;
  [[maybe_unused]] const auto &result = oss << ternary;
  EXPECT_EQ(oss.str(), std::string("Ternary 15x20"));
}

TEST(TernaryTest, InsertReturnValuePositive) {
  ev::Mat::Ternary ternary(10, 10);
  EXPECT_EQ(ternary.insert(ev::Event(3, 4, 1.0f, ev::POSITIVE)), ev::Mat::Ternary::POSITIVE);
}

TEST(TernaryTest, InsertReturnValueNegative) {
  ev::Mat::Ternary ternary(10, 10);
  EXPECT_EQ(ternary.insert(ev::Event(3, 4, 1.0f, ev::NEGATIVE)), ev::Mat::Ternary::NEGATIVE);
}

TEST(TernaryTest, FloatCoordinates) {
  ev::Mat::Ternary ternary(10, 10);
  ternary.emplace(3.6f, 4.4f, ev::POSITIVE);
  EXPECT_EQ(ternary(4, 4), ev::Mat::Ternary::POSITIVE);
}

TEST(TernaryTest, OverwriteLastWins) {
  ev::Mat::Ternary ternary(10, 10);
  ternary.emplace(3, 4, ev::POSITIVE);
  ternary.emplace(3, 4, ev::NEGATIVE);
  EXPECT_EQ(ternary(4, 3), ev::Mat::Ternary::NEGATIVE);
}

// Test Time Class
TEST(TimeTest, Insert) {
  ev::Mat::Time time(10, 10);
  const ev::Event event(3, 4, 1.5, ev::POSITIVE);
  time.insert(event);
  EXPECT_FLOAT_EQ(time(4, 3), 1.5);
}

TEST(TimeTest, Emplace) {
  ev::Mat::Time time(10, 10);
  time.emplace(5, 6, 1.5);
  EXPECT_FLOAT_EQ(time(6, 5), 1.5);
}

TEST(TimeTest, Clear) {
  ev::Mat::Time time(10, 10);
  time.emplace(5, 6, 1.5);
  time.clear();
  EXPECT_FLOAT_EQ(time(6, 5), 0.0);
}

TEST(TimeTest, LargeMatrixStressTest) {
  ev::Mat::Time time(10000, 10000);
  time.emplace(9999, 9999, 3.14);
  EXPECT_FLOAT_EQ(time(9999, 9999), 3.14);
}

TEST(TimeTest, StreamOperator) {
  const ev::Mat::Time time(20, 15);
  std::ostringstream oss;
  [[maybe_unused]] const auto &result = oss << time;
  EXPECT_EQ(oss.str(), std::string("Time 15x20"));
}

TEST(TimeTest, InsertReturnValue) {
  ev::Mat::Time time(10, 10);
  EXPECT_FLOAT_EQ(time.insert(ev::Event(3, 4, 2.5f, ev::POSITIVE)), 2.5f);
}

TEST(TimeTest, FloatCoordinates) {
  ev::Mat::Time time(10, 10);
  time.emplace(3.6f, 4.4f, 1.5f);
  EXPECT_FLOAT_EQ(time(4, 4), 1.5f);
}

TEST(TimeTest, OverwriteLastWins) {
  ev::Mat::Time time(10, 10);
  time.insert(ev::Event(3, 4, 1.0f, ev::POSITIVE));
  time.insert(ev::Event(3, 4, 9.0f, ev::POSITIVE));
  EXPECT_FLOAT_EQ(time(4, 3), 9.0f);
}

// Test Polarity Class
TEST(PolarityTest, Insert) {
  ev::Mat::Polarity polarity(10, 10);
  const ev::Event event(3, 4, 1.5, ev::POSITIVE);
  polarity.insert(event);
  EXPECT_TRUE(polarity(4, 3));
}

TEST(PolarityTest, Emplace) {
  ev::Mat::Polarity polarity(10, 10);
  polarity.emplace(5, 6, ev::POSITIVE);
  EXPECT_TRUE(polarity(6, 5));
}

TEST(PolarityTest, Clear) {
  ev::Mat::Polarity polarity(10, 10);
  polarity.emplace(5, 6, ev::POSITIVE);
  polarity.clear();
  EXPECT_FALSE(polarity(6, 5));
}

TEST(PolarityTest, LargeMatrixStressTest) {
  ev::Mat::Polarity polarity(10000, 10000);
  polarity.emplace(9999, 9999, ev::POSITIVE);
  EXPECT_TRUE(polarity(9999, 9999));
}

TEST(PolarityTest, StreamOperator) {
  const ev::Mat::Polarity polarity(20, 15);
  std::ostringstream oss;
  [[maybe_unused]] const auto &result = oss << polarity;
  EXPECT_EQ(oss.str(), std::string("Polarity 15x20"));
}

TEST(PolarityTest, InsertReturnValue) {
  ev::Mat::Polarity polarity(10, 10);
  EXPECT_TRUE(polarity.insert(ev::Event(3, 4, 1.0f, ev::POSITIVE)));
  EXPECT_FALSE(polarity.insert(ev::Event(3, 4, 1.0f, ev::NEGATIVE)));
}

TEST(PolarityTest, FloatCoordinates) {
  ev::Mat::Polarity polarity(10, 10);
  polarity.emplace(3.6f, 4.4f, ev::POSITIVE);
  EXPECT_TRUE(polarity(4, 4));
}

TEST(PolarityTest, OverwriteLastWins) {
  ev::Mat::Polarity polarity(10, 10);
  polarity.insert(ev::Event(3, 4, 1.0f, ev::POSITIVE));
  polarity.insert(ev::Event(3, 4, 2.0f, ev::NEGATIVE));
  EXPECT_FALSE(polarity(4, 3));
}

// Test Counter Class
TEST(CounterTest, Insert) {
  ev::Mat::Counter counter(10, 10);
  const ev::Event event(3, 4, 1.5, ev::POSITIVE);
  counter.insert(event);
  counter.insert(event);
  counter.insert(event);
  EXPECT_EQ(counter(4, 3), 3);
}

TEST(CounterTest, Emplace) {
  ev::Mat::Counter counter(10, 10);
  counter.emplace(5, 6, ev::POSITIVE);
  counter.emplace(5, 6, ev::POSITIVE);
  counter.emplace(5, 6, ev::POSITIVE);
  EXPECT_EQ(counter(6, 5), 3);
}

TEST(CounterTest, Clear) {
  ev::Mat::Counter counter(10, 10);
  counter.emplace(5, 6, ev::POSITIVE);
  counter.emplace(5, 6, ev::POSITIVE);
  counter.emplace(5, 6, ev::POSITIVE);
  counter.clear();
  EXPECT_EQ(counter(6, 5), 0);
}

TEST(CounterTest, LargeMatrixStressTest) {
  ev::Mat::Counter counter(10000, 10000);
  counter.emplace(9999, 9999, ev::POSITIVE);
  counter.emplace(9999, 9999, ev::POSITIVE);
  EXPECT_EQ(counter(9999, 9999), 2);
}

TEST(CounterTest, StreamOperator) {
  const ev::Mat::Counter counter(20, 15);
  std::ostringstream oss;
  [[maybe_unused]] const auto &result = oss << counter;
  EXPECT_EQ(oss.str(), std::string("Counter 15x20"));
}

TEST(CounterTest, EmplaceNegative) {
  ev::Mat::Counter counter(10, 10);
  counter.emplace(5, 6, ev::NEGATIVE);
  EXPECT_EQ(counter(6, 5), -1);
}

TEST(CounterTest, MixedPolarity) {
  ev::Mat::Counter counter(10, 10);
  counter.emplace(5, 6, ev::POSITIVE);
  counter.emplace(5, 6, ev::POSITIVE);
  counter.emplace(5, 6, ev::NEGATIVE);
  EXPECT_EQ(counter(6, 5), 1);
}

TEST(CounterTest, InsertReturnValue) {
  ev::Mat::Counter counter(10, 10);
  EXPECT_EQ(counter.insert(ev::Event(3, 4, 1.0f, ev::POSITIVE)), 1);
  EXPECT_EQ(counter.insert(ev::Event(3, 4, 1.0f, ev::NEGATIVE)), 0);
}

TEST(CounterTest, FloatCoordinates) {
  ev::Mat::Counter counter(10, 10);
  counter.emplace(3.6f, 4.4f, ev::POSITIVE);
  counter.emplace(3.6f, 4.4f, ev::POSITIVE);
  EXPECT_EQ(counter(4, 4), 2);
}
