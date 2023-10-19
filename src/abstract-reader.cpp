/*!
\file abstract-reader.cpp
\brief Implementation of AbstractReader_ class.
\author Raul Tapia
*/
#include "openev/logger.hpp"
#include "openev/reader.hpp"

bool ev::AbstractReader_::next(std::size_t n, ev::EventVector &vector) {
  ev::Event e;
  while(n-- > 0 && next(e)) {
    vector.push_back(e);
  }
  return (n == 0);
}

bool ev::AbstractReader_::next(std::size_t n, ev::EventQueue &queue) {
  ev::Event e;
  while(n-- > 0 && next(e)) {
    queue.push(e);
  }
  return (n == 0);
}
