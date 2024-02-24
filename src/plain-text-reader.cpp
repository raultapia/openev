/*!
\file plain-text-reader.cpp
\brief Implementation of PlainTextReader class.
\author Raul Tapia
*/
#include "openev/logger.hpp"
#include "openev/reader.hpp"

ev::PlainTextReader::PlainTextReader(const std::string &filename) : file_{filename, std::ios_base::in} {
  ev::logger::error("ev::PlainTextReader: Could not open file.", file_.is_open());
}

ev::PlainTextReader::~PlainTextReader() {
  if(file_.is_open()) {
    file_.close();
  }
}

void ev::PlainTextReader::reset_() {
  file_.clear(std::ios::goodbit);
  file_.seekg(0, std::ios::beg);
}

bool ev::PlainTextReader::next_(ev::Event &e) {
  ev::logger::error("ev::PlainTextReader::next: File is not opened.", file_.is_open());

  std::string line;
  if(std::getline(file_, line)) {
    std::stringstream iss(line, std::ios_base::in);
    ev::logger::error("ev::PlainTextReader: Cannot parse line.", static_cast<bool>(iss >> e.t >> e.x >> e.y >> e.p));
    return true;
  }
  return false;
}
