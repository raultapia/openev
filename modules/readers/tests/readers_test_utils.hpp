#pragma once

#include "openev/readers/abstract-reader.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"

static bool tryPull(ev::AbstractReader_ &reader, ev::Event &e) {
  ev::Queue &q = reader.data();
  if(q.empty()) {
    return false;
  }
  e = q.front();
  q.pop();
  return true;
}

static ev::Vector drainAll(ev::AbstractReader_ &reader) {
  ev::Vector v;
  ev::Event e;
  while(tryPull(reader, e)) {
    v.push_back(e);
  }
  return v;
}
