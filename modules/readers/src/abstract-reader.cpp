/*!
\file abstract-reader.cpp
\brief Implementation of abstract-reader.
\author Raul Tapia
*/
#include "openev/readers/abstract-reader.hpp"
#include <vector>

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
  if(vector.empty() && read(e)) {
    vector.emplace_back(e);
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
  if(queue.empty() && read(e)) {
    queue.emplace(e);
  }
  const ev::Event &ref = queue.back();
  while(read(e)) {
    if(e.distance(ref, DISTANCE_FLAG_TEMPORAL) >= t) {
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
  while(n-- > 0 && read(e)) {
    ;
  }
  return n < 0;
}

bool ev::AbstractReader_::skip_t(const double t) {
  ev::Event e0;
  ev::Event e1;
  if(!read(e0)) {
    return false;
  }
  while(read(e1)) {
    if(e1.distance(e0, DISTANCE_FLAG_TEMPORAL) >= t) {
      return true;
    }
  }
  return false;
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
