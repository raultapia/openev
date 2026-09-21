#include "openev/containers/queue.hpp"
#include "openev/core/frames.hpp"
#include "openev/core/imu.hpp"
#include "openev/core/types.hpp"
#include "openev/readers/plain-text-reader.hpp"
#include "openev/readers/player.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

class RecordedReader : public ev::PlainTextReader {
public:
  explicit RecordedReader(const std::string &directory) : ev::PlainTextReader(directory + "/events.txt"), frames_{{500, directory + "/images/a.png"}, {50000, directory + "/images/b.png"}}, imu_{{100, {1, 2, 3}, {4, 5, 6}}, {20000, {7, 8, 9}, {10, 11, 12}}} {}

  [[nodiscard]] const ev::FrameFileVector &frames() const {
    return frames_;
  }

  [[nodiscard]] const ev::ImuVector &imu() const {
    return imu_;
  }

private:
  ev::FrameFileVector frames_;
  ev::ImuVector imu_;
};

template <typename Reader>
class SteppedPlayer : public ev::Player_<Reader> {
public:
  explicit SteppedPlayer(Reader &reader) : ev::Player_<Reader>(reader) {}

  void advance(const ev::TimeType time, ev::Queue &events) {
    this->advance_(time, events, nullptr, nullptr);
  }

  void advance(const ev::TimeType time, ev::Queue &events, ev::StampedMatQueue &frames) {
    this->advance_(time, events, &frames, nullptr);
  }

  void advance(const ev::TimeType time, ev::Queue &events, ev::StampedMatQueue &frames, ev::ImuQueue &imus) {
    this->advance_(time, events, &frames, &imus);
  }
};

template <typename Reader>
SteppedPlayer(Reader &) -> SteppedPlayer<Reader>;

class PlayerTest : public ::testing::Test {
protected:
  std::string directory_;

  void SetUp() override {
    char path[] = "/tmp/openev_player_XXXXXX";
    directory_ = mkdtemp(path);
    std::filesystem::create_directories(directory_ + "/images");
    std::ofstream(directory_ + "/events.txt") << "0 10 10 1\n1000 20 20 0\n10000 30 30 1\n100000 40 40 0\n";
    cv::imwrite(directory_ + "/images/a.png", cv::Mat(180, 240, CV_8UC1, cv::Scalar::all(10)));
    cv::imwrite(directory_ + "/images/b.png", cv::Mat(180, 240, CV_8UC1, cv::Scalar::all(20)));
  }

  void TearDown() override {
    std::filesystem::remove_all(directory_);
  }
};

TEST_F(PlayerTest, NothingIsDeliveredBeforeTimeMoves) {
  RecordedReader reader(directory_);
  SteppedPlayer player(reader);
  ev::Queue events;
  player.advance(0, events);
  EXPECT_EQ(events.size(), 1U);
  EXPECT_DOUBLE_EQ(events.front().t, 0.0);
}

TEST_F(PlayerTest, EventsAreDeliveredUpToThePlayhead) {
  RecordedReader reader(directory_);
  SteppedPlayer player(reader);
  ev::Queue events;
  player.advance(1000, events);
  EXPECT_EQ(events.size(), 2U);
  player.advance(8000, events);
  EXPECT_EQ(events.size(), 2U);
  player.advance(2000, events);
  EXPECT_EQ(events.size(), 3U);
  EXPECT_DOUBLE_EQ(player.playhead(), 11000.0);
}

TEST_F(PlayerTest, FramesAndImuFollowThePlayhead) {
  RecordedReader reader(directory_);
  SteppedPlayer player(reader);
  ev::Queue events;
  ev::StampedMatQueue frames;
  ev::ImuQueue imus;
  player.advance(1000, events, frames, imus);
  ASSERT_EQ(frames.size(), 1U);
  EXPECT_NEAR(frames.front().t, 500.0, 1e-6);
  EXPECT_EQ(frames.front().size(), cv::Size(240, 180));
  EXPECT_EQ(frames.front().at<uchar>(0, 0), 10);
  ASSERT_EQ(imus.size(), 1U);
  EXPECT_NEAR(imus.front().t, 100.0, 1e-6);
  EXPECT_DOUBLE_EQ(imus.front().linear_acceleration.y, 2.0);
  EXPECT_DOUBLE_EQ(imus.front().angular_velocity.z, 6.0);
  player.advance(60000, events, frames, imus);
  EXPECT_EQ(frames.size(), 2U);
  EXPECT_EQ(imus.size(), 2U);
}

TEST_F(PlayerTest, LoopStartsAgainFromTheBeginning) {
  RecordedReader reader(directory_);
  SteppedPlayer player(reader);
  ev::Queue events;
  player.advance(50000, events);
  EXPECT_EQ(events.size(), 3U);
  EXPECT_EQ(player.loops(), 0U);
  player.advance(51000, events);
  EXPECT_EQ(events.size(), 4U);
  EXPECT_EQ(player.loops(), 1U);
  EXPECT_DOUBLE_EQ(player.playhead(), 0.0);
  EXPECT_FALSE(player.isFinished());
  ev::Queue again;
  player.advance(1000, again);
  ASSERT_EQ(again.size(), 2U);
  EXPECT_DOUBLE_EQ(again.front().t, 0.0);
}

TEST_F(PlayerTest, WithoutLoopThePlaybackFinishes) {
  RecordedReader reader(directory_);
  SteppedPlayer player(reader);
  player.setLoop(false);
  ev::Queue events;
  player.advance(1000000, events);
  EXPECT_EQ(events.size(), 4U);
  EXPECT_TRUE(player.isFinished());
  EXPECT_EQ(player.loops(), 0U);
  player.advance(1000000, events);
  EXPECT_EQ(events.size(), 4U);
}

TEST_F(PlayerTest, RestartGoesBackToTheBeginning) {
  RecordedReader reader(directory_);
  SteppedPlayer player(reader);
  ev::Queue events;
  ev::StampedMatQueue frames;
  player.advance(60000, events, frames);
  EXPECT_EQ(frames.size(), 2U);
  player.restart();
  EXPECT_EQ(player.loops(), 1U);
  EXPECT_DOUBLE_EQ(player.playhead(), 0.0);
  ev::Queue again;
  ev::StampedMatQueue framesAgain;
  player.advance(1000, again, framesAgain);
  EXPECT_EQ(again.size(), 2U);
  EXPECT_EQ(framesAgain.size(), 1U);
}

TEST_F(PlayerTest, NextFollowsTheClockAndTheSpeed) {
  RecordedReader reader(directory_);
  ev::Player_ player(reader);
  player.setSpeed(10.0);
  EXPECT_DOUBLE_EQ(player.speed(), 10.0);
  ev::Queue events;
  player.next(events);
  EXPECT_DOUBLE_EQ(player.playhead(), 0.0);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  player.next(events);
  EXPECT_GE(player.playhead(), 50000.0);
  EXPECT_LT(player.playhead(), 100000.0);
  EXPECT_EQ(events.size(), 3U);
}

TEST_F(PlayerTest, PauseHoldsThePlayhead) {
  RecordedReader reader(directory_);
  ev::Player_ player(reader);
  ev::Queue events;
  player.next(events);
  player.pause(true);
  EXPECT_TRUE(player.isPaused());
  std::this_thread::sleep_for(std::chrono::milliseconds(3));
  player.next(events);
  EXPECT_DOUBLE_EQ(player.playhead(), 0.0);
  player.pause(false);
  std::this_thread::sleep_for(std::chrono::milliseconds(3));
  player.next(events);
  EXPECT_GE(player.playhead(), 3000.0);
}

TEST_F(PlayerTest, ReadersWithoutFramesDeliverOnlyEvents) {
  std::ofstream(directory_ + "/plain.txt") << "0 1 1 1\n1000 2 2 0\n";
  ev::PlainTextReader reader(directory_ + "/plain.txt");
  SteppedPlayer player(reader);
  ev::Queue events;
  ev::StampedMatQueue frames;
  ev::ImuQueue imus;
  player.setLoop(false);
  player.advance(2000, events, frames, imus);
  EXPECT_EQ(events.size(), 2U);
  EXPECT_TRUE(frames.empty());
  EXPECT_TRUE(imus.empty());
}
