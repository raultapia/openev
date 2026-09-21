/*!
\file abstract-reader.cpp
\brief Implementation of abstract-reader.
\author Raul Tapia
*/
#include "openev/readers/abstract-reader.hpp"
#include <algorithm>
#include <chrono>
#include <opencv2/core/base.hpp>
#include <string>
#include <cstddef>
#include <utility>

namespace {
constexpr std::chrono::microseconds IDLE{50};
constexpr std::size_t SPINS = 64;
constexpr std::size_t MIN_CAPACITY = 4096;
} // namespace

ev::AbstractReader_::~AbstractReader_() {
  stop_();
}

ev::ConcurrentQueue &ev::AbstractReader_::fill_(const std::size_t n) {
  if(!queue_) {
    resize_(std::max(n, MIN_CAPACITY));
  }
  if(running_.load()) {
    if(n > level_) {
      CV_Error(cv::Error::StsOutOfRange, "ev::AbstractReader::events: " + std::to_string(n) + " events asked, but prefetch keeps " + std::to_string(level_) + ".");
    }
    while(queue_->size() < n && !finished_.load()) {
      std::this_thread::yield();
    }
    return *queue_;
  }

  const std::size_t wanted = queue_->size() + n;
  if(wanted > queue_->capacity()) {
    resize_(std::max(wanted, 2 * queue_->capacity()));
  }
  while(queue_->size() < wanted && !finished_.load()) {
    readBlock_(wanted - queue_->size());
  }
  return *queue_;
}

void ev::AbstractReader_::prefetch(const std::size_t n) {
  stop_();
  level_ = n;
  if(n > 0) {
    if(!queue_ || n > queue_->capacity()) {
      resize_(std::max(n, MIN_CAPACITY));
    }
    start_();
  }
}

void ev::AbstractReader_::reset() {
  const bool threaded = stop_();
  reset_();
  blockPos_ = 0;
  blockCount_ = 0;
  if(queue_) {
    queue_->clear();
  }
  finished_.store(false);
  if(threaded) {
    start_();
  }
}

void ev::AbstractReader_::setTimeTransform(const TimeType offset, const TimeType scale /*= 1*/) {
  const bool threaded = stop_();
  timeOffset_ = offset;
  timeScale_ = scale;
  if(threaded) {
    start_();
  }
}

void ev::AbstractReader_::setRoi(const cv::Rect &roi) {
  const bool threaded = stop_();
  roi_ = roi;
  if(threaded) {
    start_();
  }
}

void ev::AbstractReader_::readBlock_(const std::size_t max) {
  while(blockPos_ == blockCount_) {
    blockPos_ = 0;
    blockCount_ = read_(block_.data(), BLOCK);
    if(blockCount_ == 0) {
      finished_.store(true);
      return;
    }
    if(timeOffset_ != 0 || timeScale_ != 1) {
      for(std::size_t i = 0; i < blockCount_; i++) {
        block_[i].t = (block_[i].t - timeOffset_) * timeScale_;
      }
    }
    if(!roi_.empty()) {
      blockCount_ = static_cast<std::size_t>(std::remove_if(block_.begin(), block_.begin() + static_cast<std::ptrdiff_t>(blockCount_), [this](const Event &e) { return !roi_.contains(e); }) - block_.begin());
    }
  }
  const std::size_t count = std::min(max, blockCount_ - blockPos_);
  queue_->push(block_.data() + blockPos_, count);
  blockPos_ += count;
}

void ev::AbstractReader_::resize_(const std::size_t capacity) {
  const bool threaded = stop_();
  auto queue = std::make_unique<ConcurrentQueue>(capacity);
  while(queue_ && !queue_->empty()) {
    queue->push(queue_->front());
    queue_->pop();
  }
  queue_ = std::move(queue);
  if(threaded) {
    start_();
  }
}

void ev::AbstractReader_::start_() {
  running_.store(true);
  thread_ = std::thread([this]() {
    std::size_t idle = 0;
    while(running_.load()) {
      const std::size_t held = queue_->size();
      const std::size_t room = held < level_ ? std::min(level_ - held, queue_->write_available()) : 0;
      if(finished_.load() || room == 0) {
        if(++idle < SPINS) {
          std::this_thread::yield();
        } else {
          std::this_thread::sleep_for(IDLE);
        }
      } else {
        idle = 0;
        readBlock_(room);
      }
    }
  });
}

bool ev::AbstractReader_::stop_() {
  const bool threaded = running_.exchange(false);
  if(thread_.joinable()) {
    thread_.join();
  }
  return threaded;
}
