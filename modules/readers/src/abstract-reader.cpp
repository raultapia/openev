/*!
\file abstract-reader.cpp
\brief Implementation of abstract-reader.
\author Raul Tapia
*/
#include "openev/readers/abstract-reader.hpp"
#include <vector>

ev::AbstractReader_::AbstractReader_(const std::size_t buffer_size, const bool use_threading) : bufferSize_{buffer_size} {
  if(buffer_size > NO_BUFFER) {
    loadBuffer();
    if(use_threading) {
      threadRunning_.store(true);
      thread_ = std::thread(&AbstractReader_::threadFunction, this);
    }
  }
}

ev::AbstractReader_::~AbstractReader_() {
  if(thread_.joinable()) {
    threadRunning_.store(false);
    thread_.join();
  }
}

bool ev::AbstractReader_::read(ev::Event &e) {
  if(bufferSize_ == NO_BUFFER) {
    return read_(e);
  }

  std::lock_guard<std::mutex> lock(bufferMutex_);
  if(buffer_.empty()) {
    if(!loadBuffer()) {
      return false;
    }
  }

  e = buffer_.front();
  buffer_.pop();
  return true;
}

bool ev::AbstractReader_::read(ev::Vector &vector, const int n) {
  const std::size_t current_size = vector.size();
  vector.resize(current_size + n);
  for(int i = 0; i < n; i++) {
    if(!read(vector[current_size + i])) {
      vector.resize(current_size + i);
      return false;
    }
  }
  return true;
}

bool ev::AbstractReader_::read(ev::Queue &queue, const int n, const bool keep_size /*= false*/) {
  const std::size_t size = queue.size() + n;
  ev::Event e;
  while(queue.size() < size && read(e)) {
    queue.emplace(e);
    if(keep_size) {
      queue.pop();
    }
  }
  return n < 0;
}

bool ev::AbstractReader_::read_t(ev::Vector &vector, const double t) {
  ev::Event e;
  if(vector.empty()) {
    if(read(e)) {
      vector.emplace_back(e);
    } else {
      return false;
    }
  }

  const double t_ref = vector.back().t;
  while(read(e)) {
    if(e.t - t_ref >= t) {
      return true;
    }
    vector.emplace_back(e);
  }
  return false;
}

bool ev::AbstractReader_::read_t(ev::Queue &queue, const double t, const bool keep_size /*= false*/) {
  ev::Event e;
  if(queue.empty()) {
    if(read(e)) {
      queue.emplace(e);
    } else {
      return false;
    }
  }

  const double t_ref = queue.back().t;
  while(read(e)) {
    if(e.t - t_ref >= t) {
      return true;
    }
    queue.emplace(e);
    if(keep_size) {
      queue.pop();
    }
  }
  return false;
}

bool ev::AbstractReader_::skip(int n) {
  ev::Event e;
  while(n-- > 0 && read(e));
  return n < 0;
}

bool ev::AbstractReader_::skip_t(const double t) {
  ev::Event e0;
  ev::Event e1;
  if(!read(e0)) {
    return false;
  }
  while(read(e1)) {
    if(e1.t - e0.t >= t) {
      return true;
    }
  }
  return false;
}

void ev::AbstractReader_::reset() {
  bool reset_thread = false;
  if(thread_.joinable()) {
    threadRunning_.store(false);
    thread_.join();
    reset_thread = true;
  }

  reset_();

  if(bufferSize_ > NO_BUFFER) {
    while(!buffer_.empty()) {
      buffer_.pop();
    }
  }

  if(reset_thread) {
    threadRunning_.store(true);
    thread_ = std::thread(&AbstractReader_::threadFunction, this);
  }
}

std::size_t ev::AbstractReader_::count() {
  std::size_t cnt = 0;
  reset();
  while(skip(1)) {
    cnt++;
  }
  reset();
  return cnt;
}

void ev::AbstractReader_::threadFunction() {
  while(threadRunning_.load() && loadBuffer()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

bool ev::AbstractReader_::loadBuffer() {
  ev::Event e;
  std::lock_guard<std::mutex> lock(bufferMutex_);
  while(buffer_.size() < bufferSize_) {
    if(read_(e)) {
      buffer_.push(e);
    } else {
      return false;
    }
  }
  return true;
}
