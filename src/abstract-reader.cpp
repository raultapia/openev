/*!
\file abstract-reader.cpp
\brief Implementation of AbstractReader_ class.
\author Raul Tapia
*/
#include "openev/logger.hpp"
#include "openev/reader.hpp"

bool ev::AbstractReader_::next(std::size_t n, ev::EventPacket &ep) {
  ev::Event e;
  while(n-- > 0 && next(e)) {
    ep.push_back(e);
  }
  return (n == 0);
}

bool ev::AbstractReader_::next(std::size_t n, ev::EventBuffer &eb) {
  ev::Event e;
  while(n-- > 0 && next(e)) {
    eb.push(e);
  }
  return (n == 0);
}
