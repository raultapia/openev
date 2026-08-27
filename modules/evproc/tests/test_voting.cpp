#include "openev/core/types.hpp"
#include "openev/evproc/voting.hpp"
#include <gtest/gtest.h>

TEST(BilinearVotingTest, IntegerCoordAllWeightOnPixel) {
  const auto w = ev::bilinearVoting(ev::Eventf(3.0f, 5.0f, 0.0f, true));
  EXPECT_FLOAT_EQ(w[0], 1.0f);
  EXPECT_FLOAT_EQ(w[1], 0.0f);
  EXPECT_FLOAT_EQ(w[2], 0.0f);
  EXPECT_FLOAT_EQ(w[3], 0.0f);
}

TEST(BilinearVotingTest, HalfXSplitHorizontally) {
  const auto w = ev::bilinearVoting(ev::Eventf(3.5f, 5.0f, 0.0f, true));
  EXPECT_FLOAT_EQ(w[0], 0.5f);
  EXPECT_FLOAT_EQ(w[1], 0.5f);
  EXPECT_FLOAT_EQ(w[2], 0.0f);
  EXPECT_FLOAT_EQ(w[3], 0.0f);
}

TEST(BilinearVotingTest, HalfYSplitVertically) {
  const auto w = ev::bilinearVoting(ev::Eventf(3.0f, 5.5f, 0.0f, true));
  EXPECT_FLOAT_EQ(w[0], 0.5f);
  EXPECT_FLOAT_EQ(w[1], 0.0f);
  EXPECT_FLOAT_EQ(w[2], 0.5f);
  EXPECT_FLOAT_EQ(w[3], 0.0f);
}

TEST(BilinearVotingTest, HalfXYEqualWeights) {
  const auto w = ev::bilinearVoting(ev::Eventf(3.5f, 5.5f, 0.0f, true));
  EXPECT_FLOAT_EQ(w[0], 0.25f);
  EXPECT_FLOAT_EQ(w[1], 0.25f);
  EXPECT_FLOAT_EQ(w[2], 0.25f);
  EXPECT_FLOAT_EQ(w[3], 0.25f);
}

TEST(BilinearVotingTest, WeightsSumToOne) {
  const auto w = ev::bilinearVoting(ev::Eventf(3.7f, 5.2f, 0.0f, true));
  EXPECT_FLOAT_EQ(w[0] + w[1] + w[2] + w[3], 1.0f);
}

TEST(BilinearVotingAugmentedTest, PositionsAreCorrect) {
  const auto e = ev::bilinearVoting(ev::AugmentedEventf(3.5f, 5.5f, 0.0f, true));
  EXPECT_EQ(e[0].x, 3);
  EXPECT_EQ(e[0].y, 5);
  EXPECT_EQ(e[1].x, 4);
  EXPECT_EQ(e[1].y, 5);
  EXPECT_EQ(e[2].x, 3);
  EXPECT_EQ(e[2].y, 6);
  EXPECT_EQ(e[3].x, 4);
  EXPECT_EQ(e[3].y, 6);
}

TEST(BilinearVotingAugmentedTest, WeightsAreCorrect) {
  const auto e = ev::bilinearVoting(ev::AugmentedEventf(3.5f, 5.5f, 0.0f, true));
  EXPECT_FLOAT_EQ(e[0].weight, 0.25f);
  EXPECT_FLOAT_EQ(e[1].weight, 0.25f);
  EXPECT_FLOAT_EQ(e[2].weight, 0.25f);
  EXPECT_FLOAT_EQ(e[3].weight, 0.25f);
}

TEST(BilinearVotingAugmentedTest, WeightsSumToOne) {
  const auto e = ev::bilinearVoting(ev::AugmentedEventf(3.7f, 5.2f, 0.0f, true));
  EXPECT_FLOAT_EQ(e[0].weight + e[1].weight + e[2].weight + e[3].weight, 1.0f);
}
