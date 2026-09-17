#include "openev/containers.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <random>
#include <type_traits>
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
    events.emplace_back(x_dist(rng), y_dist(rng), t_dist(rng), p_dist(rng) ? ev::POSITIVE : ev::NEGATIVE);
  }

  return events;
}

std::vector<ev::Event> makeTimelineEvents(const std::size_t count, const double step = 1e-3) {
  std::vector<ev::Event> events;
  events.reserve(count);

  std::mt19937 rng(42);
  std::uniform_int_distribution<int> x_dist(0, kWidth - 1);
  std::uniform_int_distribution<int> y_dist(0, kHeight - 1);
  std::bernoulli_distribution p_dist(0.5);

  for(std::size_t i = 0; i < count; ++i) {
    events.emplace_back(x_dist(rng), y_dist(rng), static_cast<double>(i) * step, p_dist(rng) ? ev::POSITIVE : ev::NEGATIVE);
  }

  return events;
}

template <typename Container>
void fillSequential(Container &container, const std::vector<ev::Event> &events) {
  if constexpr(std::is_same_v<Container, ev::Queue>) {
    while(!container.empty()) {
      container.pop();
    }
    for(const auto &event : events) {
      container.push(event);
    }
  } else if constexpr(std::is_same_v<Container, ev::Vector> || std::is_same_v<Container, ev::Deque>) {
    container.resize(events.size());
    for(std::size_t i = 0; i < events.size(); ++i) {
      container[i] = events[i];
    }
  } else if constexpr(std::is_same_v<Container, ev::CircularBuffer>) {
    container.set_capacity(events.size());
    container.resize(events.size());
    for(std::size_t i = 0; i < events.size(); ++i) {
      container[i] = events[i];
    }
  } else if constexpr(std::is_same_v<Container, ev::SlidingWindow>) {
    container.clear();
    for(const auto &event : events) {
      container.push(event);
    }
  } else {
    for(std::size_t i = 0; i < events.size(); ++i) {
      container[i] = events[i];
    }
  }
}

template <typename Container, typename Fn>
void benchmarkReadOnlyMetric(benchmark::State &state, const char *label, Fn fn) {
  const auto events = [](const std::size_t count) {
    if constexpr(std::is_same_v<Container, ev::SlidingWindow>) {
      return makeTimelineEvents(count);
    }
    return makeEvents(count);
  }(static_cast<std::size_t>(state.range(0)));
  Container container = []([[maybe_unused]] const std::vector<ev::Event> &e) {
    if constexpr(std::is_same_v<Container, ev::SlidingWindow>) {
      return ev::SlidingWindow(e.back().t - e.front().t);
    } else {
      return Container();
    }
  }(events);
  fillSequential(container, events);

  for(auto _ : state) {
    benchmark::DoNotOptimize(fn(container));
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

template <typename Container, typename Fn>
void benchmarkArrayMetric(benchmark::State &state, const char *label, Fn fn) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  Container container;
  fillSequential(container, events);

  for(auto _ : state) {
    benchmark::DoNotOptimize(fn(container));
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

void benchmarkCircularEmplaceBack(benchmark::State &state, const char *label) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  ev::CircularBuffer container(events.size());

  for(auto _ : state) {
    state.PauseTiming();
    container.clear();
    state.ResumeTiming();

    for(const auto &event : events) {
      container.emplace_back(event.x, event.y, event.t, event.p);
    }
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

void benchmarkCircularEmplaceFront(benchmark::State &state, const char *label) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  ev::CircularBuffer container(events.size());

  for(auto _ : state) {
    state.PauseTiming();
    container.clear();
    state.ResumeTiming();

    for(const auto &event : events) {
      container.emplace_front(event.x, event.y, event.t, event.p);
    }
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

void benchmarkSlidingWindowPush(benchmark::State &state, const char *label) {
  const auto events = makeTimelineEvents(static_cast<std::size_t>(state.range(0)));
  ev::SlidingWindow container(0.1);

  for(auto _ : state) {
    state.PauseTiming();
    container.clear();
    state.ResumeTiming();

    for(const auto &event : events) {
      container.push(event);
    }
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

template <typename Container>
ev::Grid_<Container> makeGrid(const int cells, const std::vector<ev::Event> &events) {
  if constexpr(std::is_same_v<Container, ev::CircularBuffer>) {
    return ev::Grid_<Container>(cv::Size(kWidth, kHeight), cv::Size(cells, cells), events.size() / static_cast<std::size_t>(cells * cells));
  } else if constexpr(std::is_same_v<Container, ev::SlidingWindow>) {
    return ev::Grid_<Container>(cv::Size(kWidth, kHeight), cv::Size(cells, cells), 0.1);
  } else {
    return ev::Grid_<Container>(cv::Size(kWidth, kHeight), cv::Size(cells, cells));
  }
}

template <typename Container>
void benchmarkGridInsert(benchmark::State &state, const char *label) {
  const auto events = [](const std::size_t count) {
    if constexpr(std::is_same_v<Container, ev::SlidingWindow>) {
      return makeTimelineEvents(count);
    }
    return makeEvents(count);
  }(static_cast<std::size_t>(state.range(0)));
  auto grid = makeGrid<Container>(static_cast<int>(state.range(1)), events);

  for(auto _ : state) {
    state.PauseTiming();
    grid.clear();
    state.ResumeTiming();

    for(const auto &event : events) {
      benchmark::DoNotOptimize(grid.insert(event));
    }
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

template <typename Container>
void benchmarkGridCell(benchmark::State &state, const char *label) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  const auto grid = makeGrid<Container>(static_cast<int>(state.range(1)), events);

  for(auto _ : state) {
    for(const auto &event : events) {
      benchmark::DoNotOptimize(grid.cell(event));
    }
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

void benchmarkStatsInsert(benchmark::State &state, const char *label) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  ev::Stats stats;

  for(auto _ : state) {
    state.PauseTiming();
    stats.clear();
    state.ResumeTiming();

    for(const auto &event : events) {
      stats.push(event);
    }
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

template <typename Fn>
void benchmarkStatsQuery(benchmark::State &state, const char *label, Fn fn) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  ev::Stats stats;
  for(const auto &event : events) {
    stats.push(event);
  }

  for(auto _ : state) {
    benchmark::DoNotOptimize(fn(stats));
    benchmark::ClobberMemory();
  }

  state.SetLabel(label);
  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(events.size()));
}

void benchmarkStatsEntropy(benchmark::State &state, const char *label) {
  benchmarkStatsQuery(state, label, [](const ev::Stats &stats) { return stats.entropy(); });
}

void benchmarkStatsMean(benchmark::State &state, const char *label) {
  benchmarkStatsQuery(state, label, [](const ev::Stats &stats) { return stats.mean(); });
}

template <typename Container>
void benchmarkDuration(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.duration(); });
}

template <typename Container>
void benchmarkRate(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.rate(); });
}

template <typename Container>
void benchmarkMean(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.mean(); });
}

template <typename Container>
void benchmarkMeanPoint(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.meanPoint(); });
}

template <typename Container>
void benchmarkMeanTime(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.meanTime(); });
}

template <typename Container>
void benchmarkPolarityRatio(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.polarityRatio(ev::POSITIVE); });
}

template <typename Container>
void benchmarkBoundingBox(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.boundingBox(); });
}

template <typename Container>
void benchmarkCovariance(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.covariance(); });
}

template <typename Container>
void benchmarkActivePixels(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.activePixels(); });
}

template <typename Container>
void benchmarkPeak(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.peak(); });
}

template <typename Container>
void benchmarkMidTime(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.midTime(); });
}

template <typename Container>
void benchmarkEntropy(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.entropy(); });
}

template <typename Container>
void benchmarkArrayDuration(benchmark::State &state, const char *label) {
  benchmarkArrayMetric<Container>(state, label, [](const auto &container) { return container.duration(); });
}

template <typename Container>
void benchmarkArrayRate(benchmark::State &state, const char *label) {
  benchmarkArrayMetric<Container>(state, label, [](const auto &container) { return container.rate(); });
}

template <typename Container>
void benchmarkArrayMean(benchmark::State &state, const char *label) {
  benchmarkArrayMetric<Container>(state, label, [](const auto &container) { return container.mean(); });
}

template <typename Container>
void benchmarkArrayMeanPoint(benchmark::State &state, const char *label) {
  benchmarkArrayMetric<Container>(state, label, [](const auto &container) { return container.meanPoint(); });
}

template <typename Container>
void benchmarkArrayMeanTime(benchmark::State &state, const char *label) {
  benchmarkArrayMetric<Container>(state, label, [](const auto &container) { return container.meanTime(); });
}

template <typename Container>
void benchmarkArrayMidTime(benchmark::State &state, const char *label) {
  benchmarkArrayMetric<Container>(state, label, [](const auto &container) { return container.midTime(); });
}

template <typename Container>
void benchmarkArrayEntropy(benchmark::State &state, const char *label) {
  benchmarkArrayMetric<Container>(state, label, [](const auto &container) { return container.entropy(); });
}

static void BM_VectorDuration(benchmark::State &state) { benchmarkDuration<ev::Vector>(state, "vector"); }
static void BM_DequeDuration(benchmark::State &state) { benchmarkDuration<ev::Deque>(state, "deque"); }
static void BM_CircularDuration(benchmark::State &state) { benchmarkDuration<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueDuration(benchmark::State &state) { benchmarkDuration<ev::Queue>(state, "queue"); }

static void BM_VectorRate(benchmark::State &state) { benchmarkRate<ev::Vector>(state, "vector"); }
static void BM_DequeRate(benchmark::State &state) { benchmarkRate<ev::Deque>(state, "deque"); }
static void BM_CircularRate(benchmark::State &state) { benchmarkRate<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueRate(benchmark::State &state) { benchmarkRate<ev::Queue>(state, "queue"); }

static void BM_VectorMean(benchmark::State &state) { benchmarkMean<ev::Vector>(state, "vector"); }
static void BM_DequeMean(benchmark::State &state) { benchmarkMean<ev::Deque>(state, "deque"); }
static void BM_CircularMean(benchmark::State &state) { benchmarkMean<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueMean(benchmark::State &state) { benchmarkMean<ev::Queue>(state, "queue"); }

static void BM_VectorMeanPoint(benchmark::State &state) { benchmarkMeanPoint<ev::Vector>(state, "vector"); }
static void BM_DequeMeanPoint(benchmark::State &state) { benchmarkMeanPoint<ev::Deque>(state, "deque"); }
static void BM_CircularMeanPoint(benchmark::State &state) { benchmarkMeanPoint<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueMeanPoint(benchmark::State &state) { benchmarkMeanPoint<ev::Queue>(state, "queue"); }

static void BM_VectorMeanTime(benchmark::State &state) { benchmarkMeanTime<ev::Vector>(state, "vector"); }
static void BM_DequeMeanTime(benchmark::State &state) { benchmarkMeanTime<ev::Deque>(state, "deque"); }
static void BM_CircularMeanTime(benchmark::State &state) { benchmarkMeanTime<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueMeanTime(benchmark::State &state) { benchmarkMeanTime<ev::Queue>(state, "queue"); }

static void BM_VectorPolarityRatio(benchmark::State &state) { benchmarkPolarityRatio<ev::Vector>(state, "vector"); }
static void BM_DequePolarityRatio(benchmark::State &state) { benchmarkPolarityRatio<ev::Deque>(state, "deque"); }
static void BM_CircularPolarityRatio(benchmark::State &state) { benchmarkPolarityRatio<ev::CircularBuffer>(state, "circular"); }
static void BM_QueuePolarityRatio(benchmark::State &state) { benchmarkPolarityRatio<ev::Queue>(state, "queue"); }
static void BM_SlidingWindowPolarityRatio(benchmark::State &state) { benchmarkPolarityRatio<ev::SlidingWindow>(state, "sliding_window"); }

static void BM_VectorBoundingBox(benchmark::State &state) { benchmarkBoundingBox<ev::Vector>(state, "vector"); }
static void BM_DequeBoundingBox(benchmark::State &state) { benchmarkBoundingBox<ev::Deque>(state, "deque"); }
static void BM_CircularBoundingBox(benchmark::State &state) { benchmarkBoundingBox<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueBoundingBox(benchmark::State &state) { benchmarkBoundingBox<ev::Queue>(state, "queue"); }
static void BM_SlidingWindowBoundingBox(benchmark::State &state) { benchmarkBoundingBox<ev::SlidingWindow>(state, "sliding_window"); }

static void BM_VectorCovariance(benchmark::State &state) { benchmarkCovariance<ev::Vector>(state, "vector"); }
static void BM_DequeCovariance(benchmark::State &state) { benchmarkCovariance<ev::Deque>(state, "deque"); }
static void BM_CircularCovariance(benchmark::State &state) { benchmarkCovariance<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueCovariance(benchmark::State &state) { benchmarkCovariance<ev::Queue>(state, "queue"); }
static void BM_SlidingWindowCovariance(benchmark::State &state) { benchmarkCovariance<ev::SlidingWindow>(state, "sliding_window"); }

static void BM_VectorActivePixels(benchmark::State &state) { benchmarkActivePixels<ev::Vector>(state, "vector"); }
static void BM_DequeActivePixels(benchmark::State &state) { benchmarkActivePixels<ev::Deque>(state, "deque"); }
static void BM_CircularActivePixels(benchmark::State &state) { benchmarkActivePixels<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueActivePixels(benchmark::State &state) { benchmarkActivePixels<ev::Queue>(state, "queue"); }
static void BM_SlidingWindowActivePixels(benchmark::State &state) { benchmarkActivePixels<ev::SlidingWindow>(state, "sliding_window"); }

static void BM_VectorPeak(benchmark::State &state) { benchmarkPeak<ev::Vector>(state, "vector"); }
static void BM_DequePeak(benchmark::State &state) { benchmarkPeak<ev::Deque>(state, "deque"); }
static void BM_CircularPeak(benchmark::State &state) { benchmarkPeak<ev::CircularBuffer>(state, "circular"); }
static void BM_QueuePeak(benchmark::State &state) { benchmarkPeak<ev::Queue>(state, "queue"); }
static void BM_SlidingWindowPeak(benchmark::State &state) { benchmarkPeak<ev::SlidingWindow>(state, "sliding_window"); }

static void BM_VectorMidTime(benchmark::State &state) { benchmarkMidTime<ev::Vector>(state, "vector"); }
static void BM_DequeMidTime(benchmark::State &state) { benchmarkMidTime<ev::Deque>(state, "deque"); }
static void BM_CircularMidTime(benchmark::State &state) { benchmarkMidTime<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueMidTime(benchmark::State &state) { benchmarkMidTime<ev::Queue>(state, "queue"); }
static void BM_SlidingWindowDuration(benchmark::State &state) { benchmarkDuration<ev::SlidingWindow>(state, "sliding_window"); }
static void BM_SlidingWindowRate(benchmark::State &state) { benchmarkRate<ev::SlidingWindow>(state, "sliding_window"); }
static void BM_SlidingWindowMean(benchmark::State &state) { benchmarkMean<ev::SlidingWindow>(state, "sliding_window"); }
static void BM_SlidingWindowMeanPoint(benchmark::State &state) { benchmarkMeanPoint<ev::SlidingWindow>(state, "sliding_window"); }
static void BM_SlidingWindowMeanTime(benchmark::State &state) { benchmarkMeanTime<ev::SlidingWindow>(state, "sliding_window"); }
static void BM_SlidingWindowMidTime(benchmark::State &state) { benchmarkMidTime<ev::SlidingWindow>(state, "sliding_window"); }

static void BM_VectorEntropy(benchmark::State &state) { benchmarkEntropy<ev::Vector>(state, "vector"); }
static void BM_DequeEntropy(benchmark::State &state) { benchmarkEntropy<ev::Deque>(state, "deque"); }
static void BM_CircularEntropy(benchmark::State &state) { benchmarkEntropy<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueEntropy(benchmark::State &state) { benchmarkEntropy<ev::Queue>(state, "queue"); }
static void BM_SlidingWindowEntropy(benchmark::State &state) { benchmarkEntropy<ev::SlidingWindow>(state, "sliding_window"); }

static void BM_Array1024Duration(benchmark::State &state) { benchmarkArrayDuration<ev::Array<1024>>(state, "array"); }
static void BM_Array16384Duration(benchmark::State &state) { benchmarkArrayDuration<ev::Array<16384>>(state, "array"); }
static void BM_Array1024Rate(benchmark::State &state) { benchmarkArrayRate<ev::Array<1024>>(state, "array"); }
static void BM_Array16384Rate(benchmark::State &state) { benchmarkArrayRate<ev::Array<16384>>(state, "array"); }
static void BM_Array1024Mean(benchmark::State &state) { benchmarkArrayMean<ev::Array<1024>>(state, "array"); }
static void BM_Array16384Mean(benchmark::State &state) { benchmarkArrayMean<ev::Array<16384>>(state, "array"); }
static void BM_Array1024MeanPoint(benchmark::State &state) { benchmarkArrayMeanPoint<ev::Array<1024>>(state, "array"); }
static void BM_Array16384MeanPoint(benchmark::State &state) { benchmarkArrayMeanPoint<ev::Array<16384>>(state, "array"); }
static void BM_Array1024MeanTime(benchmark::State &state) { benchmarkArrayMeanTime<ev::Array<1024>>(state, "array"); }
static void BM_Array16384MeanTime(benchmark::State &state) { benchmarkArrayMeanTime<ev::Array<16384>>(state, "array"); }
static void BM_Array1024MidTime(benchmark::State &state) { benchmarkArrayMidTime<ev::Array<1024>>(state, "array"); }
static void BM_Array16384MidTime(benchmark::State &state) { benchmarkArrayMidTime<ev::Array<16384>>(state, "array"); }
static void BM_Array1024Entropy(benchmark::State &state) { benchmarkArrayEntropy<ev::Array<1024>>(state, "array"); }
static void BM_Array16384Entropy(benchmark::State &state) { benchmarkArrayEntropy<ev::Array<16384>>(state, "array"); }

static void BM_CircularEmplaceBack(benchmark::State &state) { benchmarkCircularEmplaceBack(state, "circular"); }
static void BM_CircularEmplaceFront(benchmark::State &state) { benchmarkCircularEmplaceFront(state, "circular"); }
static void BM_SlidingWindowPush(benchmark::State &state) { benchmarkSlidingWindowPush(state, "sliding_window"); }

static void BM_GridVectorInsert(benchmark::State &state) { benchmarkGridInsert<ev::Vector>(state, "grid_vector"); }
static void BM_GridDequeInsert(benchmark::State &state) { benchmarkGridInsert<ev::Deque>(state, "grid_deque"); }
static void BM_GridCircularInsert(benchmark::State &state) { benchmarkGridInsert<ev::CircularBuffer>(state, "grid_circular"); }
static void BM_GridQueueInsert(benchmark::State &state) { benchmarkGridInsert<ev::Queue>(state, "grid_queue"); }
static void BM_GridSlidingWindowInsert(benchmark::State &state) { benchmarkGridInsert<ev::SlidingWindow>(state, "grid_sliding_window"); }
static void BM_GridVectorCell(benchmark::State &state) { benchmarkGridCell<ev::Vector>(state, "grid_vector"); }

static void BM_StatsInsert(benchmark::State &state) { benchmarkStatsInsert(state, "stats"); }
static void BM_StatsEntropy(benchmark::State &state) { benchmarkStatsEntropy(state, "stats"); }
static void BM_StatsMean(benchmark::State &state) { benchmarkStatsMean(state, "stats"); }

BENCHMARK(BM_VectorDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorPolarityRatio)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequePolarityRatio)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularPolarityRatio)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueuePolarityRatio)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowPolarityRatio)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorBoundingBox)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeBoundingBox)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularBoundingBox)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueBoundingBox)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowBoundingBox)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorCovariance)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeCovariance)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularCovariance)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueCovariance)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowCovariance)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorActivePixels)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeActivePixels)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularActivePixels)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueActivePixels)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowActivePixels)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorPeak)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequePeak)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularPeak)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueuePeak)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowPeak)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorEntropy)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeEntropy)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularEntropy)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueEntropy)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowEntropy)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_Array1024Duration)->Arg(1024);
BENCHMARK(BM_Array16384Duration)->Arg(16384);
BENCHMARK(BM_Array1024Rate)->Arg(1024);
BENCHMARK(BM_Array16384Rate)->Arg(16384);
BENCHMARK(BM_Array1024Mean)->Arg(1024);
BENCHMARK(BM_Array16384Mean)->Arg(16384);
BENCHMARK(BM_Array1024MeanPoint)->Arg(1024);
BENCHMARK(BM_Array16384MeanPoint)->Arg(16384);
BENCHMARK(BM_Array1024MeanTime)->Arg(1024);
BENCHMARK(BM_Array16384MeanTime)->Arg(16384);
BENCHMARK(BM_Array1024MidTime)->Arg(1024);
BENCHMARK(BM_Array16384MidTime)->Arg(16384);
BENCHMARK(BM_Array1024Entropy)->Arg(1024);
BENCHMARK(BM_Array16384Entropy)->Arg(16384);

BENCHMARK(BM_CircularEmplaceBack)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularEmplaceFront)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_SlidingWindowPush)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_GridVectorInsert)->Args({1 << 14, 4})->Args({1 << 14, 16})->Args({1 << 18, 4})->Args({1 << 18, 16});
BENCHMARK(BM_GridDequeInsert)->Args({1 << 14, 4})->Args({1 << 14, 16})->Args({1 << 18, 4})->Args({1 << 18, 16});
BENCHMARK(BM_GridCircularInsert)->Args({1 << 14, 4})->Args({1 << 14, 16})->Args({1 << 18, 4})->Args({1 << 18, 16});
BENCHMARK(BM_GridQueueInsert)->Args({1 << 14, 4})->Args({1 << 14, 16})->Args({1 << 18, 4})->Args({1 << 18, 16});
BENCHMARK(BM_GridSlidingWindowInsert)->Args({1 << 14, 4})->Args({1 << 14, 16})->Args({1 << 18, 4})->Args({1 << 18, 16});
BENCHMARK(BM_GridVectorCell)->Args({1 << 14, 4})->Args({1 << 18, 16});
BENCHMARK(BM_StatsInsert)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_StatsEntropy)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_StatsMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
} // namespace
