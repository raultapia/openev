#include "readers_test_utils.hpp"
#include "openev/readers/plain-text-reader.hpp"
#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
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
    f_txyp_  = writeTempFile(kTXYP);
    f_xytp_  = writeTempFile(kXYTP);
    f_ptxy_  = writeTempFile(kPTXY);
    f_pxyt_  = writeTempFile(kPXYT);
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

TEST_F(PlainTextReaderTest, DataReturnsQueueReference) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  ev::Queue &q1 = reader.data();
  ev::Queue &q2 = reader.data();
  EXPECT_EQ(&q1, &q2);
}

TEST_F(PlainTextReaderTest, DataFillsOneEventPerCall) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  ev::Queue &q = reader.data();
  EXPECT_EQ(q.size(), 1U);
}

TEST_F(PlainTextReaderTest, DataFillsUpToBufferSize) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 3);
  ev::Queue &q = reader.data();
  reader.data();
  reader.data();
  EXPECT_EQ(q.size(), 3U);
}

TEST_F(PlainTextReaderTest, DataDoesNotExceedBufferSize) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 2);
  ev::Queue &q = reader.data();
  reader.data();
  reader.data();
  reader.data();
  EXPECT_EQ(q.size(), 2U);
}

TEST_F(PlainTextReaderTest, DataEmptyAfterEOF) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  drainAll(reader);
  ev::Queue &q = reader.data();
  EXPECT_TRUE(q.empty());
}

TEST_F(PlainTextReaderTest, FormatTXYP) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_TRUE(e.p);
}

TEST_F(PlainTextReaderTest, FormatXYTP) {
  ev::PlainTextReader reader(f_xytp_, ev::PlainTextReaderColumns::XYTP, " ", 1);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_TRUE(e.p);
}

TEST_F(PlainTextReaderTest, FormatPTXY) {
  ev::PlainTextReader reader(f_ptxy_, ev::PlainTextReaderColumns::PTXY, " ", 1);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_TRUE(e.p);
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
}

TEST_F(PlainTextReaderTest, FormatPXYT) {
  ev::PlainTextReader reader(f_pxyt_, ev::PlainTextReaderColumns::PXYT, " ", 1);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_TRUE(e.p);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_FLOAT_EQ(e.t, 1.0f);
}

TEST_F(PlainTextReaderTest, AllEventsOrdered) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  const float expected_t[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  ev::Event e;
  for(int i = 0; i < 5; i++) {
    ASSERT_TRUE(tryPull(reader, e)) << "failed at event " << i;
    EXPECT_FLOAT_EQ(e.t, expected_t[i]);
  }
  EXPECT_FALSE(tryPull(reader, e));
}

TEST_F(PlainTextReaderTest, PolarityAlternates) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  const bool expected_p[] = {true, false, true, false, true};
  ev::Event e;
  for(int i = 0; i < 5; i++) {
    ASSERT_TRUE(tryPull(reader, e));
    EXPECT_EQ(e.p, expected_p[i]);
  }
}

TEST_F(PlainTextReaderTest, CommaSeparator) {
  ev::PlainTextReader reader(f_comma_, ev::PlainTextReaderColumns::TXYP, ",", 1);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_TRUE(e.p);
}

TEST_F(PlainTextReaderTest, CommaSeparatorAllEvents) {
  ev::PlainTextReader reader(f_comma_, ev::PlainTextReaderColumns::TXYP, ",", 1);
  ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_FLOAT_EQ(v[1].t, 2.0f);
  EXPECT_FALSE(v[1].p);
}

TEST_F(PlainTextReaderTest, DrainAllEvents) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  ev::Vector v = drainAll(reader);
  EXPECT_EQ(v.size(), 5U);
}

TEST_F(PlainTextReaderTest, DrainMatchesFileContent) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 1);
  ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  EXPECT_FLOAT_EQ(v[0].t, 1.0f); EXPECT_EQ(v[0].x, 10);
  EXPECT_FLOAT_EQ(v[1].t, 2.0f); EXPECT_EQ(v[1].x, 30);
  EXPECT_FLOAT_EQ(v[2].t, 3.0f); EXPECT_EQ(v[2].x, 50);
  EXPECT_FLOAT_EQ(v[3].t, 4.0f); EXPECT_EQ(v[3].x, 70);
  EXPECT_FLOAT_EQ(v[4].t, 5.0f); EXPECT_EQ(v[4].x, 90);
}

TEST_F(PlainTextReaderTest, BufferSizeMatchesEventCount) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 5);
  ev::Queue &q = reader.data();
  for(int i = 0; i < 4; i++) { reader.data(); }
  EXPECT_EQ(q.size(), 5U);
}

TEST_F(PlainTextReaderTest, BufferRefillAfterPop) {
  ev::PlainTextReader reader(f_txyp_, ev::PlainTextReaderColumns::TXYP, " ", 2);
  ev::Queue &q = reader.data();
  reader.data();
  ASSERT_EQ(q.size(), 2U);
  q.pop();
  reader.data();
  EXPECT_EQ(q.size(), 2U);
}
