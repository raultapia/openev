#include "openev/core.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace {
constexpr int kWidth = 640;
constexpr int kHeight = 480;

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
void benchmarkClear(benchmark::State &state, const char *label) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  Matrix matrix(kHeight, kWidth);
  Matrix backup(kHeight, kWidth);

  for(const auto &event : events) {
    backup.insert(event);
  }

  for(auto _ : state) {
    state.PauseTiming();
    backup.copyTo(matrix);
    state.ResumeTiming();

    benchmark::DoNotOptimize(matrix.data);
    matrix.clear();
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations());
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

BENCHMARK(BM_BinaryClear)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_TernaryClear)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_TimeClear)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_PolarityClear)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CounterClear)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

} // namespace
