/*!
\file plain-text-reader.cpp
\brief Implementation of PlainTextReader class.
\author Raul Tapia
*/
#include "openev/logger.hpp"
#include "openev/reader.hpp"

ev::PlainTextReader::PlainTextReader(const std::string &filename) : file_{filename, std::ios_base::in} {
  if(!file_.is_open()) {
    ev::logger::error("Could not open file.");
  }
}

ev::PlainTextReader::~PlainTextReader() {
  if(file_.is_open()) {
    file_.close();
  }
}

void ev::PlainTextReader::reset() {
  file_.clear(std::ios::goodbit);
  file_.seekg(0, std::ios::beg);
}

bool ev::PlainTextReader::next(ev::Event &e) {
  if(!file_.is_open()) {
    return false;
  }

  std::string line;
  if(std::getline(file_, line)) {
    std::stringstream iss(line, std::ios_base::in);
    if(!(iss >> e.t >> e.x >> e.y >> e.p)) {
      ev::logger::error("Could not parse line.");
      return false;
    }
    return true;
  }
  return false;
}
