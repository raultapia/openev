#include "openev/core.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace {
constexpr int kWidth = 640;
constexpr int kHeight = 480;

std::vector<ev::Eventf> makeFloatEvents(const std::size_t count) {
  std::vector<ev::Eventf> events;
  events.reserve(count);
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> x_dist(0.0f, static_cast<float>(kWidth - 1));
  std::uniform_real_distribution<float> y_dist(0.0f, static_cast<float>(kHeight - 1));
  std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
  std::bernoulli_distribution p_dist(0.5);
  for(std::size_t i = 0; i < count; ++i) {
    events.emplace_back(x_dist(rng), y_dist(rng), t_dist(rng), p_dist(rng) ? ev::POSITIVE : ev::NEGATIVE);
  }
  return events;
}

std::vector<ev::Event> makeEvents(const std::size_t count) {
  std::vector<ev::Event> events;
  events.reserve(count);

  std::mt19937 rng(42);
  std::uniform_int_distribution<int> x_dist(0, kWidth - 1);
  std::uniform_int_distribution<int> y_dist(0, kHeight - 1);
  std::uniform_real_distribution<double> t_dist(0.0, 1.0);
  std::bernoulli_distribution p_dist(0.5);

  for(std::size_t i = 0; i < count; ++i) {
    const auto x = x_dist(rng);
    const auto y = y_dist(rng);
    const auto t = t_dist(rng);
    const auto p = p_dist(rng) ? ev::POSITIVE : ev::NEGATIVE;
    events.emplace_back(x, y, t, p);
  }

  return events;
}

template <typename Matrix>
void benchmarkInsert(benchmark::State &state, const char *label) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  Matrix matrix(kHeight, kWidth);

  for(auto _ : state) {
    for(const auto &event : events) {
      benchmark::DoNotOptimize(matrix.insert(event));
    }
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

template <typename Matrix>
void benchmarkInsertFloat(benchmark::State &state, const char *label) {
  const auto events = makeFloatEvents(static_cast<std::size_t>(state.range(0)));
  Matrix matrix(kHeight, kWidth);
  for(auto _ : state) {
    for(const auto &event : events) {
      benchmark::DoNotOptimize(matrix.insert(event));
    }
    benchmark::ClobberMemory();
  }
  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

template <typename Matrix>
void benchmarkUpdateStats(benchmark::State &state, const char *label) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  Matrix matrix(kHeight, kWidth);
  for(auto _ : state) {
    for(const auto &event : events) {
      matrix.updateStats(event);
    }
    benchmark::ClobberMemory();
  }
  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

template <typename Matrix>
void benchmarkClear(benchmark::State &state, const char *label) {
  Matrix matrix(kHeight, kWidth);

  for(auto _ : state) {
    benchmark::DoNotOptimize(matrix.data);
    matrix.clear();
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(matrix.total() * matrix.elemSize()));
}

static void BM_BinaryInsert(benchmark::State &state) {
  benchmarkInsert<ev::Mat::Binary>(state, "binary");
}

static void BM_TernaryInsert(benchmark::State &state) {
  benchmarkInsert<ev::Mat::Ternary>(state, "ternary");
}

static void BM_TimeInsert(benchmark::State &state) {
  benchmarkInsert<ev::Mat::Time>(state, "time");
}

static void BM_PolarityInsert(benchmark::State &state) {
  benchmarkInsert<ev::Mat::Polarity>(state, "polarity");
}

static void BM_CounterInsert(benchmark::State &state) {
  benchmarkInsert<ev::Mat::Counter>(state, "counter");
}

static void BM_BinaryInsertFloat(benchmark::State &state) { benchmarkInsertFloat<ev::Mat::Binary>(state, "binary_float"); }
static void BM_TernaryInsertFloat(benchmark::State &state) { benchmarkInsertFloat<ev::Mat::Ternary>(state, "ternary_float"); }
static void BM_TimeInsertFloat(benchmark::State &state) { benchmarkInsertFloat<ev::Mat::Time>(state, "time_float"); }
static void BM_PolarityInsertFloat(benchmark::State &state) { benchmarkInsertFloat<ev::Mat::Polarity>(state, "polarity_float"); }
static void BM_CounterInsertFloat(benchmark::State &state) { benchmarkInsertFloat<ev::Mat::Counter>(state, "counter_float"); }

static void BM_BinaryUpdateStats(benchmark::State &state) { benchmarkUpdateStats<ev::Mat::Binary>(state, "binary"); }
static void BM_TernaryUpdateStats(benchmark::State &state) { benchmarkUpdateStats<ev::Mat::Ternary>(state, "ternary"); }
static void BM_TimeUpdateStats(benchmark::State &state) { benchmarkUpdateStats<ev::Mat::Time>(state, "time"); }
static void BM_PolarityUpdateStats(benchmark::State &state) { benchmarkUpdateStats<ev::Mat::Polarity>(state, "polarity"); }
static void BM_CounterUpdateStats(benchmark::State &state) { benchmarkUpdateStats<ev::Mat::Counter>(state, "counter"); }

static void BM_BinaryClear(benchmark::State &state) {
  benchmarkClear<ev::Mat::Binary>(state, "binary");
}

static void BM_TernaryClear(benchmark::State &state) {
  benchmarkClear<ev::Mat::Ternary>(state, "ternary");
}

static void BM_TimeClear(benchmark::State &state) {
  benchmarkClear<ev::Mat::Time>(state, "time");
}

static void BM_PolarityClear(benchmark::State &state) {
  benchmarkClear<ev::Mat::Polarity>(state, "polarity");
}

static void BM_CounterClear(benchmark::State &state) {
  benchmarkClear<ev::Mat::Counter>(state, "counter");
}

BENCHMARK(BM_BinaryInsert)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_TernaryInsert)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_TimeInsert)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_PolarityInsert)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CounterInsert)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_BinaryInsertFloat)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_TernaryInsertFloat)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_TimeInsertFloat)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_PolarityInsertFloat)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CounterInsertFloat)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_BinaryUpdateStats)->Arg(1 << 14);
BENCHMARK(BM_TernaryUpdateStats)->Arg(1 << 14);
BENCHMARK(BM_TimeUpdateStats)->Arg(1 << 14);
BENCHMARK(BM_PolarityUpdateStats)->Arg(1 << 14);
BENCHMARK(BM_CounterUpdateStats)->Arg(1 << 14);

BENCHMARK(BM_BinaryClear);
BENCHMARK(BM_TernaryClear);
BENCHMARK(BM_TimeClear);
BENCHMARK(BM_PolarityClear);
BENCHMARK(BM_CounterClear);

} // namespace
