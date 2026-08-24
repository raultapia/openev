#include "openev/containers/queue.hpp"
#include "openev/readers/hdf5-reader.hpp"
#include "openev/readers/plain-text-reader.hpp"

#include <H5Cpp.h>
#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

constexpr std::size_t kEventCount = 1 << 18; // 262144 events

static std::string makeTempFile(const std::size_t count, const ev::PlainTextReaderColumns fmt, const char sep = ' ') {
  char path[] = "/tmp/openev_bench_XXXXXX";
  int fd = mkstemp(path);
  close(fd);
  std::ofstream f(path);
  for(std::size_t i = 0; i < count; ++i) {
    const double t = static_cast<double>(i) * 1e-6;
    const int x = static_cast<int>(i % 640);
    const int y = static_cast<int>(i % 480);
    const int p = static_cast<int>(i % 2);
    switch(fmt) {
    case ev::PlainTextReaderColumns::TXYP:
      f << t << sep << x << sep << y << sep << p << '\n';
      break;
    case ev::PlainTextReaderColumns::XYTP:
      f << x << sep << y << sep << t << sep << p << '\n';
      break;
    case ev::PlainTextReaderColumns::PTXY:
      f << p << sep << t << sep << x << sep << y << '\n';
      break;
    case ev::PlainTextReaderColumns::PXYT:
      f << p << sep << x << sep << y << sep << t << '\n';
      break;
    }
  }
  return path;
}

static const std::string kFileTXYP = makeTempFile(kEventCount, ev::PlainTextReaderColumns::TXYP);
static const std::string kFileXYTP = makeTempFile(kEventCount, ev::PlainTextReaderColumns::XYTP);
static const std::string kFilePTXY = makeTempFile(kEventCount, ev::PlainTextReaderColumns::PTXY);
static const std::string kFilePXYT = makeTempFile(kEventCount, ev::PlainTextReaderColumns::PXYT);
static const std::string kFileComma = makeTempFile(kEventCount, ev::PlainTextReaderColumns::TXYP, ',');

static bool pullOne(ev::PlainTextReader &reader, ev::Event &e) {
  ev::Queue &q = reader.data();
  if(q.empty()) {
    return false;
  }
  e = q.front();
  q.pop();
  return true;
}

static void BM_PullSingle(benchmark::State &state) {
  const std::size_t buf = static_cast<std::size_t>(state.range(0));
  ev::Event e;
  int64_t count = 0;

  for(auto _ : state) {
    state.PauseTiming();
    ev::PlainTextReader reader(kFileTXYP, ev::PlainTextReaderColumns::TXYP, " ", buf);
    state.ResumeTiming();

    while(pullOne(reader, e)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
  }

  state.SetItemsProcessed(count);
  state.SetLabel("pull_single");
}

static void BM_PullBatch(benchmark::State &state) {
  const std::size_t buf = static_cast<std::size_t>(state.range(0));
  ev::Event e;
  int64_t count = 0;

  for(auto _ : state) {
    state.PauseTiming();
    ev::PlainTextReader reader(kFileTXYP, ev::PlainTextReaderColumns::TXYP, " ", buf);
    for(std::size_t i = 0; i < buf; ++i) {
      reader.data();
    }
    state.ResumeTiming();

    while(pullOne(reader, e)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
  }

  state.SetItemsProcessed(count);
  state.SetLabel("pull_batch");
}

template <ev::PlainTextReaderColumns Fmt>
static void BM_Format(benchmark::State &state, const std::string &file, const char *label) {
  ev::Event e;
  int64_t count = 0;

  for(auto _ : state) {
    state.PauseTiming();
    ev::PlainTextReader reader(file, Fmt, " ", 1);
    state.ResumeTiming();

    while(pullOne(reader, e)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
  }

  state.SetItemsProcessed(count);
  state.SetLabel(label);
}

static void BM_FormatTXYP(benchmark::State &state) { BM_Format<ev::PlainTextReaderColumns::TXYP>(state, kFileTXYP, "TXYP"); }
static void BM_FormatXYTP(benchmark::State &state) { BM_Format<ev::PlainTextReaderColumns::XYTP>(state, kFileXYTP, "XYTP"); }
static void BM_FormatPTXY(benchmark::State &state) { BM_Format<ev::PlainTextReaderColumns::PTXY>(state, kFilePTXY, "PTXY"); }
static void BM_FormatPXYT(benchmark::State &state) { BM_Format<ev::PlainTextReaderColumns::PXYT>(state, kFilePXYT, "PXYT"); }

static void BM_SepSpace(benchmark::State &state) {
  ev::Event e;
  int64_t count = 0;
  for(auto _ : state) {
    state.PauseTiming();
    ev::PlainTextReader reader(kFileTXYP, ev::PlainTextReaderColumns::TXYP, " ", 1);
    state.ResumeTiming();
    while(pullOne(reader, e)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
  }
  state.SetItemsProcessed(count);
  state.SetLabel("sep_space");
}

static void BM_SepComma(benchmark::State &state) {
  ev::Event e;
  int64_t count = 0;
  for(auto _ : state) {
    state.PauseTiming();
    ev::PlainTextReader reader(kFileComma, ev::PlainTextReaderColumns::TXYP, ",", 1);
    state.ResumeTiming();
    while(pullOne(reader, e)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
  }
  state.SetItemsProcessed(count);
  state.SetLabel("sep_comma");
}

BENCHMARK(BM_PullSingle)->Arg(1)->Arg(1 << 10)->Arg(1 << 14)->Iterations(3);
BENCHMARK(BM_PullBatch)->Arg(1 << 10)->Arg(1 << 14)->Iterations(3);
BENCHMARK(BM_FormatTXYP)->Iterations(3);
BENCHMARK(BM_FormatXYTP)->Iterations(3);
BENCHMARK(BM_FormatPTXY)->Iterations(3);
BENCHMARK(BM_FormatPXYT)->Iterations(3);
BENCHMARK(BM_SepSpace)->Iterations(3);
BENCHMARK(BM_SepComma)->Iterations(3);

static std::string makeHDF5File(const std::size_t count) {
  char path[] = "/tmp/openev_bench_hdf5_XXXXXX";
  int fd = mkstemp(path);
  close(fd);

  std::vector<double> t(count);
  std::vector<int> x(count), y(count), p(count);
  for(std::size_t i = 0; i < count; ++i) {
    t[i] = static_cast<double>(i) * 1e-6;
    x[i] = static_cast<int>(i % 640);
    y[i] = static_cast<int>(i % 480);
    p[i] = static_cast<int>(i % 2);
  }

  H5::H5File f(path, H5F_ACC_TRUNC);
  H5::Group grp = f.createGroup("/events");
  hsize_t dims[1] = {count};
  H5::DataSpace space(1, dims);
  grp.createDataSet("t", H5::PredType::NATIVE_DOUBLE, space).write(t.data(), H5::PredType::NATIVE_DOUBLE);
  grp.createDataSet("x", H5::PredType::NATIVE_INT, space).write(x.data(), H5::PredType::NATIVE_INT);
  grp.createDataSet("y", H5::PredType::NATIVE_INT, space).write(y.data(), H5::PredType::NATIVE_INT);
  grp.createDataSet("p", H5::PredType::NATIVE_INT, space).write(p.data(), H5::PredType::NATIVE_INT);

  return path;
}

static const std::string kFileHDF5 = makeHDF5File(kEventCount);

static bool pullOneHDF5(ev::HDF5Reader &reader, ev::Event &e) {
  ev::Queue &q = reader.data();
  if(q.empty()) return false;
  e = q.front();
  q.pop();
  return true;
}

static void BM_HDF5(benchmark::State &state) {
  const std::size_t buf = static_cast<std::size_t>(state.range(0));
  ev::Event e;
  int64_t count = 0;

  for(auto _ : state) {
    state.PauseTiming();
    ev::HDF5Reader reader(kFileHDF5, "/events/t", "/events/x", "/events/y", "/events/p", buf);
    state.ResumeTiming();

    while(pullOneHDF5(reader, e)) {
      benchmark::DoNotOptimize(e);
      ++count;
    }
  }

  state.SetItemsProcessed(count);
  state.SetLabel("hdf5");
}

BENCHMARK(BM_HDF5)->Arg(1)->Arg(1 << 10)->Arg(1 << 14)->Iterations(3);

} // namespace
