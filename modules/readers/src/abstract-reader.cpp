/*!
\file abstract-reader.cpp
\brief Implementation of abstract-reader.
\author Raul Tapia
*/
#include "openev/readers/abstract-reader.hpp"
#include "openev/utils/logger.hpp"

bool ev::AbstractReader_::next(int n, ev::Vector &vector) {
  if(n < 1) {
    return true;
  }

  ev::Event e;
  while(n-- > 0 && next_(e)) {
    vector.emplace_back(e);
    std::cout << "n = " << n << '\n';
  }
  std::cout << "final n = " << n << '\n';
  return n < 0;
}

bool ev::AbstractReader_::next(int n, ev::Queue &queue, const bool keep_same_size /*= false*/) {
  if(n < 1) {
    return true;
  }

  ev::Event e;
  while(n-- > 0 && next_(e)) {
    queue.emplace(e);
    if(keep_same_size) {
      queue.pop();
    }
  }
  return n < 0;
}
