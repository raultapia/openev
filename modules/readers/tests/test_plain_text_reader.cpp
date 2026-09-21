#include "openev/readers/plain-text-reader.hpp"
#include "readers_test_utils.hpp"
#include <cstdio>
#include <fstream>
#include <chrono>
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <thread>
#include <string>

static std::string writeTempFile(const std::string &content) {
  char path[] = "/tmp/openev_test_XXXXXX";
  int fd = mkstemp(path);
  close(fd);
  std::ofstream f(path);
  f << content;
  return path;
}

static const std::string kTXYP =
    "1.0 10 20 1\n"
    "2.0 30 40 0\n"
    "3.0 50 60 1\n"
    "4.0 70 80 0\n"
    "5.0 90 100 1\n";

static const std::string kXYTP =
    "10 20 1.0 1\n"
    "30 40 2.0 0\n"
    "50 60 3.0 1\n"
    "70 80 4.0 0\n"
    "90 100 5.0 1\n";

static const std::string kPTXY =
    "1 1.0 10 20\n"
    "0 2.0 30 40\n"
    "1 3.0 50 60\n"
    "0 4.0 70 80\n"
    "1 5.0 90 100\n";

static const std::string kPXYT =
    "1 10 20 1.0\n"
    "0 30 40 2.0\n"
    "1 50 60 3.0\n"
    "0 70 80 4.0\n"
    "1 90 100 5.0\n";

static const std::string kComma =
    "1.0,10,20,1\n"
    "2.0,30,40,0\n"
    "3.0,50,60,1\n"
    "4.0,70,80,0\n"
    "5.0,90,100,1\n";

class PlainTextReaderTest : public ::testing::Test {
protected:
  std::string f_txyp_, f_xytp_, f_ptxy_, f_pxyt_, f_comma_;

  void SetUp() override {
    f_txyp_ = writeTempFile(kTXYP);
    f_xytp_ = writeTempFile(kXYTP);
    f_ptxy_ = writeTempFile(kPTXY);
    f_pxyt_ = writeTempFile(kPXYT);
    f_comma_ = writeTempFile(kComma);
  }

  void TearDown() override {
    std::remove(f_txyp_.c_str());
    std::remove(f_xytp_.c_str());
    std::remove(f_ptxy_.c_str());
    std::remove(f_pxyt_.c_str());
    std::remove(f_comma_.c_str());
  }
};

TEST_F(PlainTextReaderTest, EventsReturnsTheSameQueue) {
  ev::PlainTextReader reader(f_txyp_);
  ev::ConcurrentQueue &q1 = reader.events(2);
  ev::ConcurrentQueue &q2 = reader.events();
  EXPECT_EQ(&q1, &q2);
}

TEST_F(PlainTextReaderTest, EventsAddsTheGivenNumber) {
  ev::PlainTextReader reader(f_txyp_);
  EXPECT_EQ(reader.events(2).size(), 2U);
  EXPECT_EQ(reader.events(2).size(), 4U);
  EXPECT_DOUBLE_EQ(reader.events().front().t, 1.0);
}

TEST_F(PlainTextReaderTest, EventsAddsLessOnlyAtTheEnd) {
  ev::PlainTextReader reader(f_txyp_);
  EXPECT_EQ(reader.events(3).size(), 3U);
  EXPECT_EQ(reader.events(100).size(), 5U);
  EXPECT_EQ(reader.events(100).size(), 5U);
}

TEST_F(PlainTextReaderTest, EventsWithoutNumberAddsNothing) {
  ev::PlainTextReader reader(f_txyp_);
  EXPECT_TRUE(reader.events().empty());
  EXPECT_EQ(reader.events(2).size(), 2U);
  EXPECT_EQ(reader.events().size(), 2U);
  reader.events().pop();
  EXPECT_EQ(reader.events().size(), 1U);
  EXPECT_DOUBLE_EQ(reader.events().front().t, 2.0);
}

TEST(PlainTextReader, QueueGrowsKeepingThePendingEvents) {
  constexpr int N = 20000;
  std::string content;
  for(int i = 0; i < N; i++) {
    content += std::to_string(i) + " 1 2 1\n";
  }
  const std::string file = writeTempFile(content);
  ev::PlainTextReader reader(file);
  const std::size_t capacity = reader.events(10).capacity();
  reader.events().pop();
  ev::ConcurrentQueue &q = reader.events(capacity);
  std::remove(file.c_str());
  EXPECT_GT(q.capacity(), capacity);
  EXPECT_EQ(q.size(), capacity + 9U);
  EXPECT_DOUBLE_EQ(q.front().t, 1.0);
}

TEST_F(PlainTextReaderTest, BatchCanBeConsumedThroughTheReference) {
  ev::PlainTextReader reader(f_txyp_);
  std::size_t count = 0;
  while(true) {
    ev::ConcurrentQueue &batch = reader.events(2);
    if(batch.empty()) {
      break;
    }
    while(!batch.empty()) {
      batch.pop();
      count++;
    }
  }
  EXPECT_EQ(count, 5U);
}

TEST_F(PlainTextReaderTest, EventsEmptyAfterEOF) {
  ev::PlainTextReader reader(f_txyp_);
  drainAll(reader);
  EXPECT_TRUE(reader.events(1).empty());
}

TEST_F(PlainTextReaderTest, PrefetchReadsTheSameEvents) {
  ev::PlainTextReader reader(f_txyp_);
  reader.prefetch(2);
  const ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_DOUBLE_EQ(v[0].t, 1.0);
  EXPECT_DOUBLE_EQ(v[4].t, 5.0);
}

TEST_F(PlainTextReaderTest, PrefetchFillsTheQueueWithoutBeingAsked) {
  ev::PlainTextReader reader(f_txyp_);
  reader.prefetch(4);
  ev::ConcurrentQueue &q = reader.events();
  for(int i = 0; i < 2000 && q.size() < 4; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  EXPECT_EQ(q.size(), 4U);
}

TEST_F(PlainTextReaderTest, ResetWithPrefetchStartsAgain) {
  ev::PlainTextReader reader(f_txyp_);
  reader.prefetch(3);
  reader.events(2).pop();
  reader.reset();
  const ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_DOUBLE_EQ(v[0].t, 1.0);
}

TEST_F(PlainTextReaderTest, PrefetchCanBeStopped) {
  ev::PlainTextReader reader(f_txyp_);
  reader.prefetch(3);
  EXPECT_EQ(reader.events(2).size() >= 2U, true);
  reader.prefetch(0);
  EXPECT_EQ(drainAll(reader).size(), 5U);
}

TEST_F(PlainTextReaderTest, DestroyedWhilePrefetching) {
  for(int i = 0; i < 50; i++) {
    ev::PlainTextReader reader(f_txyp_);
    reader.prefetch(2);
  }
  SUCCEED();
}

TEST_F(PlainTextReaderTest, FormatTXYP) {
  ev::PlainTextReader reader(f_txyp_);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_TRUE(e.p);
}

TEST_F(PlainTextReaderTest, FormatXYTP) {
  ev::PlainTextReader reader(f_xytp_, ev::PlainTextReaderColumns::XYTP);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_TRUE(e.p);
}

TEST_F(PlainTextReaderTest, FormatPTXY) {
  ev::PlainTextReader reader(f_ptxy_, ev::PlainTextReaderColumns::PTXY);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_TRUE(e.p);
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
}

TEST_F(PlainTextReaderTest, FormatPXYT) {
  ev::PlainTextReader reader(f_pxyt_, ev::PlainTextReaderColumns::PXYT);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_TRUE(e.p);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_FLOAT_EQ(e.t, 1.0f);
}

TEST_F(PlainTextReaderTest, AllEventsOrdered) {
  ev::PlainTextReader reader(f_txyp_);
  const float expected_t[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  ev::Event e;
  for(int i = 0; i < 5; i++) {
    ASSERT_TRUE(tryPull(reader, e)) << "failed at event " << i;
    EXPECT_FLOAT_EQ(e.t, expected_t[i]);
  }
  EXPECT_FALSE(tryPull(reader, e));
}

TEST_F(PlainTextReaderTest, PolarityAlternates) {
  ev::PlainTextReader reader(f_txyp_);
  const bool expected_p[] = {true, false, true, false, true};
  ev::Event e;
  for(int i = 0; i < 5; i++) {
    ASSERT_TRUE(tryPull(reader, e));
    EXPECT_EQ(e.p, expected_p[i]);
  }
}

TEST_F(PlainTextReaderTest, CommaSeparator) {
  ev::PlainTextReader reader(f_comma_, ev::PlainTextReaderColumns::TXYP, ",");
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_TRUE(e.p);
}

TEST_F(PlainTextReaderTest, CommaSeparatorAllEvents) {
  ev::PlainTextReader reader(f_comma_, ev::PlainTextReaderColumns::TXYP, ",");
  ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_FLOAT_EQ(v[1].t, 2.0f);
  EXPECT_FALSE(v[1].p);
}

TEST_F(PlainTextReaderTest, DrainAllEvents) {
  ev::PlainTextReader reader(f_txyp_);
  ev::Vector v = drainAll(reader);
  EXPECT_EQ(v.size(), 5U);
}

TEST_F(PlainTextReaderTest, DrainMatchesFileContent) {
  ev::PlainTextReader reader(f_txyp_);
  ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_FLOAT_EQ(v[0].t, 1.0f);
  EXPECT_EQ(v[0].x, 10);
  EXPECT_FLOAT_EQ(v[1].t, 2.0f);
  EXPECT_EQ(v[1].x, 30);
  EXPECT_FLOAT_EQ(v[2].t, 3.0f);
  EXPECT_EQ(v[2].x, 50);
  EXPECT_FLOAT_EQ(v[3].t, 4.0f);
  EXPECT_EQ(v[3].x, 70);
  EXPECT_FLOAT_EQ(v[4].t, 5.0f);
  EXPECT_EQ(v[4].x, 90);
}

TEST_F(PlainTextReaderTest, ResetRestartsFromTheFirstLine) {
  ev::PlainTextReader reader(f_txyp_);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_DOUBLE_EQ(e.t, 2.0);
  reader.reset();
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_DOUBLE_EQ(e.t, 1.0);
  EXPECT_EQ(drainAll(reader).size(), 4U);
}

TEST_F(PlainTextReaderTest, TimeTransformSubtractsTheOffsetAndScales) {
  ev::PlainTextReader reader(f_txyp_);
  reader.setTimeTransform(1.0, 1e6);
  const ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_DOUBLE_EQ(v[0].t, 0.0);
  EXPECT_DOUBLE_EQ(v[1].t, 1e6);
  EXPECT_DOUBLE_EQ(v[4].t, 4e6);
  EXPECT_EQ(v[1].x, 30);
}

TEST_F(PlainTextReaderTest, TimeTransformDefaultsToIdentity) {
  ev::PlainTextReader reader(f_txyp_);
  const ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_DOUBLE_EQ(v[0].t, 1.0);
  EXPECT_DOUBLE_EQ(v[4].t, 5.0);
}

TEST_F(PlainTextReaderTest, TimeTransformSurvivesReset) {
  ev::PlainTextReader reader(f_txyp_);
  reader.setTimeTransform(1.0, 1e6);
  EXPECT_EQ(drainAll(reader).size(), 5U);
  reader.reset();
  const ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_DOUBLE_EQ(v[0].t, 0.0);
  EXPECT_DOUBLE_EQ(v[4].t, 4e6);
}

TEST_F(PlainTextReaderTest, PrefetchedEventsAreWaitedFor) {
  ev::PlainTextReader reader(f_txyp_);
  reader.prefetch(3);
  EXPECT_EQ(reader.prefetchCapacity(), 3U);
  EXPECT_GE(reader.events(3).size(), 3U);
  EXPECT_EQ(drainAll(reader).size(), 5U);
}

TEST_F(PlainTextReaderTest, AskingForMoreThanPrefetchKeepsThrows) {
  ev::PlainTextReader reader(f_txyp_);
  reader.prefetch(2);
  EXPECT_THROW((void)reader.events(3), cv::Exception);
  EXPECT_NO_THROW((void)reader.events(2));
  reader.prefetch(0);
  EXPECT_EQ(reader.prefetchCapacity(), 0U);
  EXPECT_NO_THROW((void)reader.events(3));
}

TEST(PlainTextReader, FileLargerThanTheReadingChunk) {
  constexpr int N = 20000;
  std::string content;
  for(int i = 0; i < N; i++) {
    content += std::to_string(1.5 + 0.25 * i) + " " + std::to_string(i % 640) + " " + std::to_string(i % 480) + " " + std::to_string(i % 2) + "\n";
  }
  ASSERT_GT(content.size(), 3U * 65536U);
  const std::string file = writeTempFile(content);
  ev::PlainTextReader reader(file);
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  ASSERT_EQ(v.size(), static_cast<std::size_t>(N));
  bool same = true;
  for(int i = 0; i < N; i++) {
    same = same && v[i].t == 1.5 + 0.25 * i && v[i].x == i % 640 && v[i].y == i % 480 && v[i].p == (i % 2 == 1);
  }
  EXPECT_TRUE(same);
}

TEST(PlainTextReader, LastLineWithoutNewline) {
  const std::string file = writeTempFile("1.0 10 20 1\n2.0 30 40 0");
  ev::PlainTextReader reader(file);
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  ASSERT_EQ(v.size(), 2U);
  EXPECT_DOUBLE_EQ(v[1].t, 2.0);
  EXPECT_EQ(v[1].y, 40);
  EXPECT_FALSE(v[1].p);
}

TEST(PlainTextReader, WindowsLineEndings) {
  const std::string file = writeTempFile("1.0 10 20 1\r\n2.0 30 40 0\r\n");
  ev::PlainTextReader reader(file);
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  ASSERT_EQ(v.size(), 2U);
  EXPECT_EQ(v[0].x, 10);
  EXPECT_TRUE(v[0].p);
  EXPECT_DOUBLE_EQ(v[1].t, 2.0);
}

TEST(PlainTextReader, LineLongerThanTheReadingChunk) {
  const std::string file = writeTempFile("1.0 10 20 1\n2.0" + std::string(100000, ' ') + "30 40 0\n3.0 50 60 1\n");
  ev::PlainTextReader reader(file);
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  ASSERT_EQ(v.size(), 3U);
  EXPECT_EQ(v[1].x, 30);
  EXPECT_DOUBLE_EQ(v[2].t, 3.0);
}

TEST(PlainTextReader, ScientificNotationAndSigns) {
  const std::string file = writeTempFile("1.5e3 10 20 1\n+2.25 30 40 -1\n");
  ev::PlainTextReader reader(file);
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  ASSERT_EQ(v.size(), 2U);
  EXPECT_DOUBLE_EQ(v[0].t, 1500.0);
  EXPECT_DOUBLE_EQ(v[1].t, 2.25);
  EXPECT_FALSE(v[1].p);
}

TEST(PlainTextReader, StopsAtTheFirstLineItCannotParse) {
  const std::string file = writeTempFile("1.0 10 20 1\nnot an event\n3.0 50 60 1\n");
  ev::PlainTextReader reader(file);
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  ASSERT_EQ(v.size(), 1U);
  EXPECT_DOUBLE_EQ(v[0].t, 1.0);
}

TEST_F(PlainTextReaderTest, RoiIsEmptyByDefaultAndDeliversEverything) {
  ev::PlainTextReader reader(f_txyp_);
  EXPECT_TRUE(reader.getRoi().empty());
  EXPECT_EQ(drainAll(reader).size(), 5U);
}

TEST_F(PlainTextReaderTest, RoiKeepsOnlyTheEventsInside) {
  ev::PlainTextReader reader(f_txyp_);
  reader.setRoi(cv::Rect(25, 35, 30, 30));
  EXPECT_EQ(reader.getRoi(), cv::Rect(25, 35, 30, 30));
  const ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 2U);
  EXPECT_EQ(v[0].x, 30);
  EXPECT_EQ(v[0].y, 40);
  EXPECT_EQ(v[1].x, 50);
}

TEST_F(PlainTextReaderTest, EventsCountsTheEventsInsideTheRoi) {
  ev::PlainTextReader reader(f_txyp_);
  reader.setRoi(cv::Rect(25, 35, 100, 100));
  EXPECT_EQ(reader.events(2).size(), 2U);
  EXPECT_EQ(reader.events().front().x, 30);
  EXPECT_EQ(reader.events(100).size(), 4U);
}

TEST(PlainTextReader, RoiDoesNotAffectTheEventsAlreadyRead) {
  constexpr int N = 20000;
  std::string content;
  for(int i = 0; i < N; i++) {
    content += std::to_string(i) + " " + std::to_string(i % 100) + " 5 1\n";
  }
  const std::string file = writeTempFile(content);
  ev::PlainTextReader reader(file);
  EXPECT_EQ(reader.events(50).size(), 50U);
  reader.setRoi(cv::Rect(0, 0, 10, 10));
  ASSERT_EQ(reader.events().size(), 50U);
  EXPECT_EQ(reader.events().front().x, 0);
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  EXPECT_EQ(v[49].x, 49);
  EXPECT_GT(v.size(), static_cast<std::size_t>(N / 10));
  EXPECT_LT(v.size(), static_cast<std::size_t>(N / 10 + 1000));
  EXPECT_LT(v.back().x, 10);
}

TEST_F(PlainTextReaderTest, EmptyRoiRemovesTheFilter) {
  ev::PlainTextReader reader(f_txyp_);
  reader.setRoi(cv::Rect(0, 0, 1, 1));
  EXPECT_TRUE(reader.events(5).empty());
  reader.reset();
  reader.setRoi(cv::Rect());
  EXPECT_EQ(drainAll(reader).size(), 5U);
}

TEST_F(PlainTextReaderTest, RoiSurvivesReset) {
  ev::PlainTextReader reader(f_txyp_);
  reader.setRoi(cv::Rect(25, 35, 30, 30));
  EXPECT_EQ(drainAll(reader).size(), 2U);
  reader.reset();
  EXPECT_EQ(drainAll(reader).size(), 2U);
}

TEST(PlainTextReader, RoiWithPrefetchOverManyEvents) {
  constexpr int N = 20000;
  std::string content;
  for(int i = 0; i < N; i++) {
    content += std::to_string(i) + " " + std::to_string(i % 100) + " 5 1\n";
  }
  const std::string file = writeTempFile(content);
  ev::PlainTextReader reader(file);
  reader.setRoi(cv::Rect(0, 0, 10, 10));
  reader.prefetch(64);
  std::size_t total = 0;
  bool inside = true;
  bool ordered = true;
  double last = -1;
  while(!reader.events(32).empty()) {
    ev::ConcurrentQueue &events = reader.events();
    while(!events.empty()) {
      inside = inside && events.front().x < 10;
      ordered = ordered && events.front().t > last;
      last = events.front().t;
      events.pop();
      total++;
    }
  }
  std::remove(file.c_str());
  EXPECT_EQ(total, static_cast<std::size_t>(N / 10));
  EXPECT_TRUE(inside);
  EXPECT_TRUE(ordered);
}

TEST(PlainTextReader, RoiCanChangeWhilePrefetching) {
  constexpr int N = 20000;
  std::string content;
  for(int i = 0; i < N; i++) {
    content += std::to_string(i) + " " + std::to_string(i % 100) + " 5 1\n";
  }
  const std::string file = writeTempFile(content);
  ev::PlainTextReader reader(file);
  reader.prefetch(64);
  EXPECT_GE(reader.events(32).size(), 32U);
  reader.setRoi(cv::Rect(0, 0, 10, 10));
  const ev::Vector v = drainAll(reader);
  std::remove(file.c_str());
  EXPECT_GT(v.size(), static_cast<std::size_t>(N / 10));
  EXPECT_LT(v.size(), static_cast<std::size_t>(N / 10 + 1000));
  EXPECT_LT(v.back().x, 10);
}
