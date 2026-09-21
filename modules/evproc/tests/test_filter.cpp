#include "openev/core/types.hpp"
#include "openev/evproc/filtering.hpp"
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
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

TEST(BackgroundActivityFilterTest, WithoutResetEarlierEventsPass) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 100.0f, true));
  EXPECT_TRUE(filter(ev::Event(6, 5, 0.0f, true)));
}

TEST(BackgroundActivityFilterTest, ResetForgetsNeighbors) {
  ev::BackgroundActivityFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 100.0f, true));
  filter.reset();
  EXPECT_FALSE(filter(ev::Event(6, 5, 0.0f, true)));
  EXPECT_TRUE(filter(ev::Event(5, 5, 0.5f, true)));
}

TEST(RefractoryPeriodFilterTest, FirstEventPasses) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  EXPECT_TRUE(filter(ev::Event(5, 5, 0.0f, true)));
}

TEST(RefractoryPeriodFilterTest, EventWithinDtIsRejected) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_FALSE(filter(ev::Event(5, 5, 0.5f, true)));
}

TEST(RefractoryPeriodFilterTest, EventAfterDtPasses) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(5, 5, 2.0f, true)));
}

TEST(RefractoryPeriodFilterTest, ExactDtBoundaryPasses) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(RefractoryPeriodFilterTest, NeighborIsNotInhibited) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_TRUE(filter(ev::Event(6, 5, 0.5f, true)));
}

TEST(RefractoryPeriodFilterTest, RejectedEventDoesNotExtendInhibition) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  EXPECT_FALSE(filter(ev::Event(5, 5, 0.5f, true)));
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(RefractoryPeriodFilterTest, RateIsLimitedToOneEventPerDt) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  int passed = 0;
  for(int i = 0; i < 40; i++) {
    if(filter(ev::Event(5, 5, 0.25f * static_cast<float>(i), true))) {
      passed++;
    }
  }
  EXPECT_EQ(passed, 10);
}

TEST(RefractoryPeriodFilterTest, SetDtChangesInhibition) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 0.0f, true));
  filter.setDt(0.25f);
  EXPECT_TRUE(filter(ev::Event(5, 5, 0.5f, true)));
}

TEST(RefractoryPeriodFilterTest, WithoutResetEarlierEventsAreRejected) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 100.0f, true));
  EXPECT_FALSE(filter(ev::Event(5, 5, 0.0f, true)));
}

TEST(RefractoryPeriodFilterTest, ResetLiftsInhibition) {
  ev::RefractoryPeriodFilter filter({10, 10}, 1.0f);
  (void)filter(ev::Event(5, 5, 100.0f, true));
  filter.reset();
  EXPECT_TRUE(filter(ev::Event(5, 5, 0.0f, true)));
  EXPECT_FALSE(filter(ev::Event(5, 5, 0.5f, true)));
}

namespace {
void fire(ev::HotPixelFilter &filter, const int x, const int y, const float t, const int times) {
  for(int i = 0; i < times; i++) {
    (void)filter(ev::Event(x, y, t, true));
  }
}

void window(ev::HotPixelFilter &filter, const float t, const int hot) {
  fire(filter, 1, 1, t, 1);
  fire(filter, 2, 2, t, 1);
  fire(filter, 3, 3, t, 1);
  fire(filter, 5, 5, t, hot);
}
} // namespace

TEST(HotPixelFilterTest, FirstWindowPassesEverything) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  for(int i = 0; i < 100; i++) {
    EXPECT_TRUE(filter(ev::Event(5, 5, 0.5f, true)));
  }
}

TEST(HotPixelFilterTest, PixelAboveFactorIsRejectedInNextWindow) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  window(filter, 0.0f, 97);
  EXPECT_FALSE(filter(ev::Event(5, 5, 1.0f, true)));
  EXPECT_TRUE(filter(ev::Event(1, 1, 1.0f, true)));
}

TEST(HotPixelFilterTest, PixelBelowFactorPasses) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  window(filter, 0.0f, 2);
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, ExactFactorBoundaryPasses) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  fire(filter, 1, 1, 0.0f, 1);
  fire(filter, 2, 2, 0.0f, 1);
  fire(filter, 5, 5, 0.0f, 4);
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, PixelFiringAloneIsNotAboveTheMean) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  fire(filter, 5, 5, 0.0f, 100);
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, LimitRejectsPixelFiringAlone) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 0.0, 50);
  fire(filter, 5, 5, 0.0f, 100);
  EXPECT_FALSE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, ExactLimitBoundaryPasses) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 0.0, 50);
  fire(filter, 5, 5, 0.0f, 50);
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, EitherCriterionIsEnough) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 1000.0, 50);
  window(filter, 0.0f, 97);
  EXPECT_FALSE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, BothCriteriaDisabledPassesEverything) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 0.0, ev::HotPixelFilter::DISABLED);
  window(filter, 0.0f, 97);
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, PixelStaysHotWhileItKeepsFiring) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  window(filter, 0.0f, 97);
  window(filter, 1.0f, 97);
  EXPECT_FALSE(filter(ev::Event(5, 5, 2.0f, true)));
}

TEST(HotPixelFilterTest, PixelRecoversWhenItCoolsDown) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  window(filter, 0.0f, 97);
  window(filter, 1.0f, 1);
  EXPECT_TRUE(filter(ev::Event(5, 5, 2.0f, true)));
}

TEST(HotPixelFilterTest, MaskHoldsTheHotPixels) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  EXPECT_EQ(cv::countNonZero(filter.mask()), 0);
  window(filter, 0.0f, 97);
  (void)filter(ev::Event(1, 1, 1.0f, true));
  EXPECT_EQ(filter.mask().size(), cv::Size(10, 10));
  EXPECT_EQ(cv::countNonZero(filter.mask()), 1);
  EXPECT_NE(filter.mask()(5, 5), 0);
}

TEST(HotPixelFilterTest, ResetForgetsHotPixels) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 2.0);
  window(filter, 100.0f, 97);
  (void)filter(ev::Event(1, 1, 101.0f, true));
  filter.reset();
  EXPECT_EQ(cv::countNonZero(filter.mask()), 0);
  EXPECT_TRUE(filter(ev::Event(5, 5, 0.0f, true)));
}

TEST(HotPixelFilterTest, ResetForgetsCounts) {
  ev::HotPixelFilter filter({10, 10}, 1.0f, 0.0, 50);
  fire(filter, 5, 5, 100.0f, 100);
  filter.reset();
  fire(filter, 5, 5, 0.0f, 10);
  EXPECT_TRUE(filter(ev::Event(5, 5, 1.0f, true)));
}

TEST(HotPixelFilterTest, SettersChangeTheCriteria) {
  ev::HotPixelFilter filter({10, 10}, 100.0f, 1000.0);
  filter.setWindow(1.0f);
  filter.setFactor(2.0);
  window(filter, 0.0f, 97);
  EXPECT_FALSE(filter(ev::Event(5, 5, 1.0f, true)));
  filter.setFactor(0.0);
  filter.setLimit(50);
  window(filter, 1.0f, 40);
  EXPECT_TRUE(filter(ev::Event(5, 5, 2.0f, true)));
}
