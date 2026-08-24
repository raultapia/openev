#include "openev/readers/hdf5-reader.hpp"
#include "readers_test_utils.hpp"
#include <H5Cpp.h>
#include <cstdio>
#include <gtest/gtest.h>
#include <string>
#include <vector>

static std::string writeHDF5File(const std::vector<double> &t,
                                 const std::vector<int> &x,
                                 const std::vector<int> &y,
                                 const std::vector<int> &p) {
  char path[] = "/tmp/openev_hdf5_test_XXXXXX";
  int fd = mkstemp(path);
  close(fd);

  H5::H5File f(path, H5F_ACC_TRUNC);
  H5::Group grp = f.createGroup("/events");
  hsize_t dims[1] = {t.size()};
  H5::DataSpace space(1, dims);

  grp.createDataSet("t", H5::PredType::NATIVE_DOUBLE, space).write(t.data(), H5::PredType::NATIVE_DOUBLE);
  grp.createDataSet("x", H5::PredType::NATIVE_INT, space).write(x.data(), H5::PredType::NATIVE_INT);
  grp.createDataSet("y", H5::PredType::NATIVE_INT, space).write(y.data(), H5::PredType::NATIVE_INT);
  grp.createDataSet("p", H5::PredType::NATIVE_INT, space).write(p.data(), H5::PredType::NATIVE_INT);

  return path;
}

class HDF5ReaderTest : public ::testing::Test {
protected:
  std::string f_;

  const std::vector<double> t_ = {1.0, 2.0, 3.0, 4.0, 5.0};
  const std::vector<int> x_ = {10, 30, 50, 70, 90};
  const std::vector<int> y_ = {20, 40, 60, 80, 100};
  const std::vector<int> p_ = {1, 0, 1, 0, 1};

  void SetUp() override { f_ = writeHDF5File(t_, x_, y_, p_); }
  void TearDown() override { std::remove(f_.c_str()); }
};

TEST_F(HDF5ReaderTest, DataReturnsQueueReference) {
  ev::HDF5Reader reader(f_, "/events/t", "/events/x", "/events/y", "/events/p", 1);
  ev::Queue &q1 = reader.data();
  ev::Queue &q2 = reader.data();
  EXPECT_EQ(&q1, &q2);
}

TEST_F(HDF5ReaderTest, FirstEventCorrect) {
  ev::HDF5Reader reader(f_, "/events/t", "/events/x", "/events/y", "/events/p", 1);
  ev::Event e;
  ASSERT_TRUE(tryPull(reader, e));
  EXPECT_FLOAT_EQ(e.t, 1.0f);
  EXPECT_EQ(e.x, 10);
  EXPECT_EQ(e.y, 20);
  EXPECT_TRUE(e.p);
}

TEST_F(HDF5ReaderTest, DrainAllEvents) {
  ev::HDF5Reader reader(f_, "/events/t", "/events/x", "/events/y", "/events/p", 1);
  ev::Vector v = drainAll(reader);
  EXPECT_EQ(v.size(), 5U);
}

TEST_F(HDF5ReaderTest, DrainMatchesFileContent) {
  ev::HDF5Reader reader(f_, "/events/t", "/events/x", "/events/y", "/events/p", 1);
  ev::Vector v = drainAll(reader);
  ASSERT_EQ(v.size(), 5U);
  for(int i = 0; i < 5; i++) {
    EXPECT_FLOAT_EQ(v[i].t, static_cast<float>(t_[i]));
    EXPECT_EQ(v[i].x, x_[i]);
    EXPECT_EQ(v[i].y, y_[i]);
    EXPECT_EQ(v[i].p, static_cast<bool>(p_[i]));
  }
}

TEST_F(HDF5ReaderTest, EmptyAfterEOF) {
  ev::HDF5Reader reader(f_, "/events/t", "/events/x", "/events/y", "/events/p", 1);
  drainAll(reader);
  EXPECT_TRUE(reader.data().empty());
}

TEST_F(HDF5ReaderTest, PolarityAlternates) {
  ev::HDF5Reader reader(f_, "/events/t", "/events/x", "/events/y", "/events/p", 1);
  const bool expected[] = {true, false, true, false, true};
  ev::Event e;
  for(int i = 0; i < 5; i++) {
    ASSERT_TRUE(tryPull(reader, e));
    EXPECT_EQ(e.p, expected[i]);
  }
}

TEST_F(HDF5ReaderTest, LargeDatasetChunkBoundary) {
  const std::size_t N = 5000;
  std::vector<double> t(N);
  std::vector<int> x(N), y(N), p(N);
  for(std::size_t i = 0; i < N; i++) {
    t[i] = static_cast<double>(i) * 1e-6;
    x[i] = static_cast<int>(i % 640);
    y[i] = static_cast<int>(i % 480);
    p[i] = static_cast<int>(i % 2);
  }
  const std::string big = writeHDF5File(t, x, y, p);
  ev::HDF5Reader reader(big, "/events/t", "/events/x", "/events/y", "/events/p", 1);
  ev::Vector v = drainAll(reader);
  std::remove(big.c_str());
  EXPECT_EQ(v.size(), N);
  EXPECT_FLOAT_EQ(v[4096].t, static_cast<float>(4096e-6));
}
