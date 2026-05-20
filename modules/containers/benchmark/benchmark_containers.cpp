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
  } else {
    for(std::size_t i = 0; i < events.size(); ++i) {
      container[i] = events[i];
    }
  }
}

template <typename Container, typename Fn>
void benchmarkReadOnlyMetric(benchmark::State &state, const char *label, Fn fn) {
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

template <typename Container, typename Fn>
void benchmarkDestructiveMetric(benchmark::State &state, const char *label, Fn fn) {
  const auto events = makeEvents(static_cast<std::size_t>(state.range(0)));
  Container container;

  for(auto _ : state) {
    state.PauseTiming();
    fillSequential(container, events);
    state.ResumeTiming();

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
  if constexpr(std::is_same_v<Container, ev::Queue>) {
    benchmarkDestructiveMetric<Container>(state, label, [](auto &container) { return container.mean(); });
  } else {
    benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.mean(); });
  }
}

template <typename Container>
void benchmarkMeanPoint(benchmark::State &state, const char *label) {
  if constexpr(std::is_same_v<Container, ev::Queue>) {
    benchmarkDestructiveMetric<Container>(state, label, [](auto &container) { return container.meanPoint(); });
  } else {
    benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.meanPoint(); });
  }
}

template <typename Container>
void benchmarkMeanTime(benchmark::State &state, const char *label) {
  if constexpr(std::is_same_v<Container, ev::Queue>) {
    benchmarkDestructiveMetric<Container>(state, label, [](auto &container) { return container.meanTime(); });
  } else {
    benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.meanTime(); });
  }
}

template <typename Container>
void benchmarkMidTime(benchmark::State &state, const char *label) {
  benchmarkReadOnlyMetric<Container>(state, label, [](const auto &container) { return container.midTime(); });
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

static void BM_VectorMidTime(benchmark::State &state) { benchmarkMidTime<ev::Vector>(state, "vector"); }
static void BM_DequeMidTime(benchmark::State &state) { benchmarkMidTime<ev::Deque>(state, "deque"); }
static void BM_CircularMidTime(benchmark::State &state) { benchmarkMidTime<ev::CircularBuffer>(state, "circular"); }
static void BM_QueueMidTime(benchmark::State &state) { benchmarkMidTime<ev::Queue>(state, "queue"); }

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

static void BM_CircularEmplaceBack(benchmark::State &state) { benchmarkCircularEmplaceBack(state, "circular"); }
static void BM_CircularEmplaceFront(benchmark::State &state) { benchmarkCircularEmplaceFront(state, "circular"); }

BENCHMARK(BM_VectorDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueDuration)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueRate)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMean)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMeanPoint)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMeanTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

BENCHMARK(BM_VectorMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_DequeMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_QueueMidTime)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);

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

BENCHMARK(BM_CircularEmplaceBack)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
BENCHMARK(BM_CircularEmplaceFront)->Arg(1 << 10)->Arg(1 << 14)->Arg(1 << 18);
} // namespace
