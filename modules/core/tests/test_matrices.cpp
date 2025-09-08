#include "openev/core/matrices.hpp"
#include "openev/core/types.hpp"
#include <gtest/gtest.h>
#include <opencv2/core/mat.hpp>

// Test Binary Class
TEST(BinaryTest, Insert) {
  ev::Mat::Binary binary(10, 10);
  ev::Event event(3, 4, 1.5, ev::POSITIVE);
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

// Test Time Class
TEST(TimeTest, Insert) {
  ev::Mat::Time time(10, 10);
  ev::Event event(3, 4, 1.5, ev::POSITIVE);
  time.insert(event);
  EXPECT_DOUBLE_EQ(time(4, 3), 1.5);
}

TEST(TimeTest, Emplace) {
  ev::Mat::Time time(10, 10);
  time.emplace(5, 6, 1.5);
  EXPECT_DOUBLE_EQ(time(6, 5), 1.5);
}

TEST(TimeTest, Clear) {
  ev::Mat::Time time(10, 10);
  time.emplace(5, 6, 1.5);
  time.clear();
  EXPECT_DOUBLE_EQ(time(6, 5), 0.0);
}

TEST(TimeTest, LargeMatrixStressTest) {
  ev::Mat::Time time(10000, 10000);
  time.emplace(9999, 9999, 3.14);
  EXPECT_DOUBLE_EQ(time(9999, 9999), 3.14);
}

// Test Polarity Class
TEST(PolarityTest, Insert) {
  ev::Mat::Polarity polarity(10, 10);
  ev::Event event(3, 4, 1.5, ev::POSITIVE);
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

// Test Counter Class
TEST(CounterTest, Insert) {
  ev::Mat::Counter counter(10, 10);
  ev::Event event(3, 4, 1.5, ev::POSITIVE);
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
