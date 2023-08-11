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

ev::Event ev::AbstractReader_::next() {
  ev::Event e;
  if(!next(e)) {
    ev::logger::warning("End of the dataset reached. Restarting.");
    reset();
  }
  return e;
}

ev::EventPacket ev::AbstractReader_::next(const std::size_t n) {
  ev::EventPacket ep;
  if(!next(n, ep)) {
    ev::logger::warning("End of the dataset reached. Restarting.");
    reset();
  }
  return ep;
}
