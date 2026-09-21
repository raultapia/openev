#include "openev/containers/concurrent_queue.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/datasets/abstract-dataset.hpp"
#include "openev/datasets/event-camera-dataset.hpp"
#include "openev/readers/player.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>
#include <zip.h>

namespace {
const std::string EVENTS = "0.5 1 2 1\n0.6 3 4 0\n";
const std::string RECORDED_EVENTS = "0.003811000 96 133 0\n0.003820001 127 171 1\n1.500000000 4 160 0\n";
const std::string RECORDED_IMAGES = "0.038766001 images/frame_00000001.png\n0.000000000 images/frame_00000000.png\n";
const std::string RECORDED_IMU = "0.005811000 0.3 -9.7 0.1 0.04 0.05 -0.06\n0.001811000 0.1 -9.8 0.2 0.01 0.02 -0.03\n";

constexpr std::size_t BATCH = 2;
constexpr std::size_t LONG_EVENTS = 20000;

std::string longEvents() {
  std::string content;
  for(std::size_t i = 0; i < LONG_EVENTS; i++) {
    content += std::to_string(1.0 + 1e-5 * static_cast<double>(i)) + " " + std::to_string(i % 200) + " 5 1\n";
  }
  return content;
}

ev::Vector drain(ev::AbstractDataset &sequence) {
  ev::Vector v;
  while(true) {
    ev::ConcurrentQueue *events = &sequence.events();
    if(events->empty()) {
      events = &sequence.events(BATCH);
    }
    if(events->empty()) {
      break;
    }
    while(!events->empty()) {
      v.push_back(events->front());
      events->pop();
    }
  }
  return v;
}

void zipFile(const std::string &file, const std::vector<std::pair<std::string, std::string>> &entries) {
  int code = 0;
  zip_t *archive = zip_open(file.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &code);
  ASSERT_NE(archive, nullptr);
  for(const auto &[name, content] : entries) {
    zip_source_t *source = zip_source_buffer(archive, content.data(), content.size(), 0);
    ASSERT_GE(zip_file_add(archive, name.c_str(), source, ZIP_FL_OVERWRITE), 0);
  }
  ASSERT_EQ(zip_close(archive), 0);
}

class LocalDataset : public ev::EventCameraDataset {
public:
  explicit LocalDataset(std::string server) : server_{std::move(server)} {}

  [[nodiscard]] std::vector<std::string> sequences() const override {
    return {"good", "nested", "recorded", "long", "missing", "broken", "empty", "unsafe"};
  }

  [[nodiscard]] std::string url(const std::string &sequence) const override {
    return "file://" + server_ + "/" + sequence + ".zip";
  }

private:
  std::string server_;
};
} // namespace

namespace {
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

} // namespace

class DatasetTest : public ::testing::Test {
protected:
  std::string root_;
  std::string server_;
  std::string cache_;

  void SetUp() override {
    char path[] = "/tmp/openev_datasets_XXXXXX";
    root_ = mkdtemp(path);
    server_ = root_ + "/server";
    cache_ = root_ + "/cache";
    std::filesystem::create_directories(server_);
    zipFile(server_ + "/good.zip", {{"events.txt", EVENTS}, {"images.txt", ""}});
    zipFile(server_ + "/nested.zip", {{"events.txt", EVENTS}, {"images/", ""}, {"images/a.txt", "a"}});
    zipFile(server_ + "/recorded.zip", {{"events.txt", RECORDED_EVENTS}, {"images.txt", RECORDED_IMAGES}, {"imu.txt", RECORDED_IMU}});
    zipFile(server_ + "/long.zip", {{"events.txt", longEvents()}});
    zipFile(server_ + "/empty.zip", {{"readme.txt", "nothing"}});
    zipFile(server_ + "/unsafe.zip", {{"events.txt", EVENTS}, {"../escaped.txt", "bad"}});
    std::ofstream(server_ + "/broken.zip") << "this is not a zip file";
  }

  void TearDown() override {
    std::filesystem::remove_all(root_);
  }
};

TEST(EventCameraDataset, Catalogue) {
  const ev::EventCameraDataset dataset;
  EXPECT_EQ(dataset.name(), "Event Camera Dataset");
  EXPECT_EQ(dataset.shortname(), "ECD");
  EXPECT_EQ(dataset.sequences().size(), 27U);
  EXPECT_EQ(dataset.sequences().front(), "shapes_rotation");
  EXPECT_EQ(dataset.sequences().back(), "simulation_3walls");
  EXPECT_EQ(dataset.url("shapes_rotation"), "https://rpg.ifi.uzh.ch/datasets/davis/shapes_rotation.zip");
  EXPECT_EQ(dataset.sensorSize("shapes_rotation"), cv::Size(240, 180));
  EXPECT_EQ(dataset.cameraName("shapes_rotation"), "DAVIS240C");
}

TEST(EventCameraDataset, DefaultDirectoryFollowsTheCache) {
  const char *previous = std::getenv("XDG_CACHE_HOME");
  const std::string saved = previous != nullptr ? previous : "";
  setenv("XDG_CACHE_HOME", "/tmp/openev_xdg", 1);
  const ev::EventCameraDataset dataset;
  EXPECT_EQ(ev::AbstractDataset::cacheDirectory(), "/tmp/openev_xdg/openev/datasets");
  EXPECT_EQ(dataset.directory(), "/tmp/openev_xdg/openev/datasets/ECD");
  EXPECT_EQ(dataset.path("urban"), "/tmp/openev_xdg/openev/datasets/ECD/urban");
  if(previous != nullptr) {
    setenv("XDG_CACHE_HOME", saved.c_str(), 1);
  } else {
    unsetenv("XDG_CACHE_HOME");
  }
}

TEST_F(DatasetTest, SetDirectory) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_EQ(dataset.directory(), cache_);
  EXPECT_EQ(dataset.path("good"), cache_ + "/good");
  dataset.setDirectory("");
  EXPECT_EQ(dataset.directory(), ev::AbstractDataset::cacheDirectory() + "/ECD");
}

TEST_F(DatasetTest, DownloadExtractsTheSequence) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.isAvailable("good"));
  EXPECT_TRUE(dataset.download("good"));
  EXPECT_TRUE(dataset.error().empty());
  EXPECT_TRUE(dataset.isAvailable("good"));
  EXPECT_TRUE(std::filesystem::exists(cache_ + "/good/events.txt"));
  EXPECT_TRUE(std::filesystem::exists(cache_ + "/good/images.txt"));
  EXPECT_FALSE(std::filesystem::exists(cache_ + "/good.zip"));
}

TEST_F(DatasetTest, DownloadKeepsSubdirectories) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_TRUE(dataset.download("nested"));
  EXPECT_TRUE(std::filesystem::exists(cache_ + "/nested/images/a.txt"));
}

TEST_F(DatasetTest, DownloadReportsProgress) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  double received = 0;
  EXPECT_TRUE(dataset.download("good", [&received](const double now, const double /*total*/) { received = now; }));
  EXPECT_GT(received, 0.0);
}

TEST_F(DatasetTest, AvailableSequenceIsNotDownloadedAgain) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_TRUE(dataset.download("good"));
  std::filesystem::remove(server_ + "/good.zip");
  EXPECT_TRUE(dataset.download("good"));
}

TEST_F(DatasetTest, UnknownSequenceFails) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.download("other"));
  EXPECT_FALSE(dataset.error().empty());
  EXPECT_FALSE(dataset.isAvailable("other"));
}

TEST_F(DatasetTest, MissingFileFailsAndLeavesNothingBehind) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.download("missing"));
  EXPECT_FALSE(dataset.error().empty());
  EXPECT_FALSE(std::filesystem::exists(cache_ + "/missing"));
  EXPECT_FALSE(std::filesystem::exists(cache_ + "/missing.zip"));
}

TEST_F(DatasetTest, BrokenZipFails) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.download("broken"));
  EXPECT_FALSE(dataset.error().empty());
  EXPECT_FALSE(std::filesystem::exists(cache_ + "/broken"));
}

TEST_F(DatasetTest, ZipWithoutEventsFails) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.download("empty"));
  EXPECT_FALSE(dataset.isAvailable("empty"));
  EXPECT_FALSE(std::filesystem::exists(cache_ + "/empty"));
}

TEST_F(DatasetTest, ZipEscapingTheDirectoryFails) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.download("unsafe"));
  EXPECT_FALSE(std::filesystem::exists(cache_ + "/escaped.txt"));
  EXPECT_FALSE(std::filesystem::exists(cache_ + "/unsafe"));
}

TEST_F(DatasetTest, ErrorIsClearedByTheNextDownload) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.download("missing"));
  EXPECT_TRUE(dataset.download("good"));
  EXPECT_TRUE(dataset.error().empty());
}

TEST_F(DatasetTest, OpenDownloadsAndReads) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("good"));
  EXPECT_TRUE(dataset.isAvailable("good"));
  EXPECT_EQ(dataset.sensorSize(dataset.sequence()), cv::Size(240, 180));
  ASSERT_FALSE(dataset.events(1).empty());
  EXPECT_EQ(dataset.events().front().x, 1);
}

TEST_F(DatasetTest, TimestampsAreMicroseconds) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  const ev::Vector v = drain(dataset);
  ASSERT_EQ(v.size(), 3U);
  EXPECT_NEAR(v[0].t, 3811.0, 1e-6);
  EXPECT_NEAR(v[1].t, 3820.001, 1e-6);
  EXPECT_NEAR(v[2].t, 1500000.0, 1e-6);
  EXPECT_EQ(v[0].x, 96);
  EXPECT_EQ(v[0].y, 133);
  EXPECT_FALSE(v[0].p);
  EXPECT_TRUE(v[1].p);
}

TEST_F(DatasetTest, EventsAddsTheGivenNumber) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_TRUE(dataset.events().empty());
  EXPECT_EQ(dataset.events(1).size(), 1U);
  EXPECT_EQ(dataset.events(1).size(), 2U);
  EXPECT_EQ(dataset.events().size(), 2U);
}

TEST_F(DatasetTest, EventsAddsLessOnlyAtTheEnd) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_EQ(dataset.events(100).size(), 3U);
  EXPECT_EQ(dataset.events(100).size(), 3U);
}

TEST_F(DatasetTest, BatchCanBeConsumedThroughTheReference) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  std::size_t total = 0;
  while(true) {
    ev::ConcurrentQueue &batch = dataset.events(2);
    if(batch.empty()) {
      break;
    }
    while(!batch.empty()) {
      batch.pop();
      total++;
    }
  }
  EXPECT_EQ(total, 3U);
}

TEST_F(DatasetTest, FramesAreInMicrosecondsAndSorted) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  ASSERT_EQ(dataset.frames().size(), 2U);
  EXPECT_NEAR(dataset.frames()[0].t, 0.0, 1e-6);
  EXPECT_NEAR(dataset.frames()[1].t, 38766.001, 1e-6);
  EXPECT_EQ(dataset.frames()[0].path, cache_ + "/recorded/images/frame_00000000.png");
}

TEST_F(DatasetTest, ImuIsInMicrosecondsAndSorted) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  ASSERT_EQ(dataset.imu().size(), 2U);
  EXPECT_NEAR(dataset.imu()[0].t, 1811.0, 1e-6);
  EXPECT_NEAR(dataset.imu()[1].t, 5811.0, 1e-6);
  EXPECT_DOUBLE_EQ(dataset.imu()[0].linear_acceleration.y, -9.8);
  EXPECT_DOUBLE_EQ(dataset.imu()[1].angular_velocity.z, -0.06);
}

TEST_F(DatasetTest, NoFramesNorImuWithoutFiles) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("nested"));
  EXPECT_TRUE(dataset.frames().empty());
  EXPECT_TRUE(dataset.imu().empty());
}

TEST_F(DatasetTest, ResetReadsTheSequenceAgain) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_EQ(drain(dataset).size(), 3U);
  dataset.reset();
  const ev::Vector v = drain(dataset);
  ASSERT_EQ(v.size(), 3U);
  EXPECT_NEAR(v[0].t, 3811.0, 1e-6);
  EXPECT_EQ(dataset.frames().size(), 2U);
  EXPECT_EQ(dataset.imu().size(), 2U);
}

TEST_F(DatasetTest, OpenFailsWhenTheDownloadFails) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.open("missing"));
  EXPECT_FALSE(dataset.isOpen());
  EXPECT_FALSE(dataset.error().empty());
}

TEST_F(DatasetTest, OpenAndClose) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  EXPECT_FALSE(dataset.isOpen());
  EXPECT_TRUE(dataset.sequence().empty());
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_TRUE(dataset.isOpen());
  EXPECT_EQ(dataset.sequence(), "recorded");
  EXPECT_EQ(dataset.sensorSize(dataset.sequence()), cv::Size(240, 180));
  EXPECT_EQ(dataset.cameraName(dataset.sequence()), "DAVIS240C");
  dataset.close();
  EXPECT_FALSE(dataset.isOpen());
  EXPECT_TRUE(dataset.sequence().empty());
  EXPECT_TRUE(dataset.frames().empty());
  EXPECT_TRUE(dataset.imu().empty());
}

TEST_F(DatasetTest, OpeningAnotherSequenceReplacesTheFirst) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  ASSERT_TRUE(dataset.open("good"));
  EXPECT_EQ(dataset.sequence(), "good");
  EXPECT_TRUE(dataset.imu().empty());
  EXPECT_EQ(drain(dataset).size(), 2U);
}

TEST_F(DatasetTest, FailedOpenClosesTheSequence) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_FALSE(dataset.open("missing"));
  EXPECT_FALSE(dataset.isOpen());
}

TEST_F(DatasetTest, PlayerPlaysTheOpenSequence) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  SteppedPlayer player(dataset);
  ev::Queue events;
  ev::StampedMatQueue frames;
  ev::ImuQueue imus;
  player.advance(5000, events, frames, imus);
  EXPECT_EQ(events.size(), 2U);
  EXPECT_EQ(imus.size(), 2U);
  player.advance(2000000, events, frames, imus);
  EXPECT_EQ(events.size(), 3U);
  EXPECT_EQ(player.loops(), 1U);
}

TEST_F(DatasetTest, PrefetchReadsTheSameEvents) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  dataset.prefetch(2);
  const ev::Vector v = drain(dataset);
  ASSERT_EQ(v.size(), 3U);
  EXPECT_NEAR(v[0].t, 3811.0, 1e-6);
  EXPECT_NEAR(v[2].t, 1500000.0, 1e-6);
}

TEST_F(DatasetTest, EverySequenceStartsWithoutPrefetch) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_EQ(dataset.prefetchCapacity(), 0U);
  dataset.prefetch(8);
  EXPECT_EQ(dataset.prefetchCapacity(), 8U);
  EXPECT_EQ(dataset.events(3).size(), 3U);
  ASSERT_TRUE(dataset.open("good"));
  EXPECT_EQ(dataset.prefetchCapacity(), 0U);
  EXPECT_EQ(drain(dataset).size(), 2U);
}

TEST_F(DatasetTest, PrefetchFillsTheQueueWithoutBeingAsked) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.prefetch(500);
  ev::ConcurrentQueue &events = dataset.events();
  for(int i = 0; i < 2000 && events.size() < 500; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  EXPECT_EQ(events.size(), 500U);
}

TEST_F(DatasetTest, PrefetchKeepsTheOrderOverManyEvents) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.prefetch(64);
  std::size_t total = 0;
  bool ordered = true;
  while(true) {
    ev::ConcurrentQueue &events = dataset.events(50);
    if(events.empty()) {
      break;
    }
    while(!events.empty()) {
      ordered = ordered && events.front().x == static_cast<int>(total % 200);
      events.pop();
      total++;
    }
  }
  EXPECT_EQ(total, LONG_EVENTS);
  EXPECT_TRUE(ordered);
}

TEST_F(DatasetTest, ResetWithPrefetchStartsAgain) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.prefetch(64);
  EXPECT_EQ(dataset.events(50).front().x, 0);
  for(int i = 0; i < 30; i++) {
    dataset.events().pop();
  }
  dataset.reset();
  EXPECT_EQ(dataset.events(10).front().x, 0);
  EXPECT_EQ(drain(dataset).size(), LONG_EVENTS);
}

TEST_F(DatasetTest, PrefetchCanBeStopped) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.prefetch(64);
  EXPECT_EQ(dataset.events(10).size() >= 10U, true);
  dataset.prefetch(0);
  EXPECT_EQ(drain(dataset).size(), LONG_EVENTS);
}

TEST_F(DatasetTest, QueueGrowsKeepingThePendingEvents) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  const std::size_t capacity = dataset.events(10).capacity();
  dataset.events().pop();
  ev::ConcurrentQueue &events = dataset.events(capacity);
  EXPECT_GT(events.capacity(), capacity);
  EXPECT_EQ(events.size(), capacity + 9U);
  EXPECT_EQ(events.front().x, 1);
}

TEST_F(DatasetTest, CloseWithPrefetchStopsTheThread) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.prefetch(64);
  dataset.close();
  EXPECT_FALSE(dataset.isOpen());
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_EQ(drain(dataset).size(), 3U);
}

TEST_F(DatasetTest, AskingForMoreThanPrefetchKeepsThrows) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.prefetch(64);
  EXPECT_EQ(dataset.prefetchCapacity(), 64U);
  EXPECT_THROW((void)dataset.events(65), cv::Exception);
  EXPECT_NO_THROW((void)dataset.events(64));
}

TEST_F(DatasetTest, PlayerAdaptsToASmallPrefetch) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.prefetch(16);
  SteppedPlayer player(dataset);
  player.setLoop(false);
  ev::Queue events;
  player.advance(10000000, events);
  EXPECT_EQ(events.size(), LONG_EVENTS);
}

TEST_F(DatasetTest, RoiDefaultsToTheWholeSensor) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  EXPECT_EQ(dataset.getRoi(), cv::Rect(0, 0, 240, 180));
  EXPECT_EQ(drain(dataset).size(), LONG_EVENTS);
}

TEST_F(DatasetTest, RoiKeepsOnlyTheEventsInside) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.setRoi(cv::Rect(0, 0, 20, 20));
  EXPECT_EQ(dataset.getRoi(), cv::Rect(0, 0, 20, 20));
  const ev::Vector v = drain(dataset);
  EXPECT_EQ(v.size(), LONG_EVENTS / 10);
  bool inside = true;
  for(const ev::Event &e : v) {
    inside = inside && e.x < 20;
  }
  EXPECT_TRUE(inside);
}

TEST_F(DatasetTest, EverySequenceStartsWithoutRoi) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.setRoi(cv::Rect(0, 0, 20, 20));
  EXPECT_EQ(drain(dataset).size(), LONG_EVENTS / 10);
  ASSERT_TRUE(dataset.open("recorded"));
  EXPECT_EQ(dataset.getRoi(), cv::Rect(0, 0, 240, 180));
  EXPECT_EQ(drain(dataset).size(), 3U);
}

TEST_F(DatasetTest, RoiWithPrefetchAndPlayer) {
  LocalDataset dataset(server_);
  dataset.setDirectory(cache_);
  ASSERT_TRUE(dataset.open("long"));
  dataset.setRoi(cv::Rect(0, 0, 20, 20));
  dataset.prefetch(64);
  SteppedPlayer player(dataset);
  player.setLoop(false);
  ev::Queue events;
  player.advance(10000000, events);
  EXPECT_EQ(events.size(), LONG_EVENTS / 10);
}
