#include "openev/evproc/filtering.hpp"
#include "openev/core/types.hpp"
#include <gtest/gtest.h>
#include <opencv2/core/types.hpp>

TEST(BackgroundActivityFilterTest, IsolatedEventIsRejected) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  EXPECT_FALSE(filter(ev::Event(5, 5, 0.0f, true)));
}

TEST(BackgroundActivityFilterTest, CorrelatedNeighborPasses) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(6, 5, 0.5f, true)));
}

TEST(BackgroundActivityFilterTest, NeighborOutsideDtIsRejected) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_FALSE(filter(ev::Event(6, 5, 2.0f, true)));
}

TEST(BackgroundActivityFilterTest, ExactDtBoundaryPasses) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(6, 5, 1.0f, true)));
}

TEST(BackgroundActivityFilterTest, DiagonalNeighborPasses) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(4, 4, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(5, 5, 0.5f, true)));
}

TEST(BackgroundActivityFilterTest, RadiusTwoReachesDistantNeighbor) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f, 2);
  (void)filter(ev::Event(3, 5, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(5, 5, 0.5f, true)));
}

TEST(BackgroundActivityFilterTest, RadiusOneDoesNotReachDistantNeighbor) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f, 1);
  (void)filter(ev::Event(3, 5, 0.0f, true));
  EXPECT_FALSE(filter(ev::Event(5, 5, 0.5f, true)));
}

TEST(BackgroundActivityFilterTest, MapUpdatedAfterRejection) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(6, 5, 0.5f, true)));
}

TEST(BackgroundActivityFilterTest, BorderPixelChecksOnlyValidNeighbors) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(0, 0, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(1, 0, 0.5f, true)));
}
