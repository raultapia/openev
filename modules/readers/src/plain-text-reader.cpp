/*!
\file plain-text-reader.cpp
\brief Implementation of plain-text-reader.
\author Raul Tapia
*/
#include "openev/readers/plain-text-reader.hpp"
#include "openev/utils/logger.hpp"
#include <sstream>

ev::PlainTextReader::PlainTextReader(const std::string &filename, const PlainTextReaderColumns columns /*= PlainTextReaderColumns::TXYP*/, const std::string &separator /*= " "*/, const std::size_t buffer_size /*= 0*/, const bool use_threading /*=false*/) : file_{filename, std::ios::in}, separator_{separator}, AbstractReader_{buffer_size, use_threading} {
  switch(columns) {
  case ev::PlainTextReaderColumns::TXYP:
    parser_ = [](std::stringstream &iss, ev::Event &e) { iss >> e.t >> e.x >> e.y >> e.p; };
    break;
  case ev::PlainTextReaderColumns::XYTP:
    parser_ = [](std::stringstream &iss, ev::Event &e) { iss >> e.x >> e.y >> e.t >> e.p; };
    break;
  case ev::PlainTextReaderColumns::PTXY:
    parser_ = [](std::stringstream &iss, ev::Event &e) { iss >> e.p >> e.t >> e.x >> e.y; };
    break;
  case ev::PlainTextReaderColumns::PXYT:
    parser_ = [](std::stringstream &iss, ev::Event &e) { iss >> e.p >> e.x >> e.y >> e.t; };
    break;
  default:
    ev::logger::error("ev::PlainTextReader: No column order selected.");
  }
  replace_ = (separator != " ");
  ev::logger::error("ev::PlainTextReader: Could not open file.", file_.is_open());
}

ev::PlainTextReader::~PlainTextReader() {
  if(file_.is_open()) {
    file_.close();
  }
}

std::size_t ev::PlainTextReader::count() {
  std::streampos original_pos = file_.tellg();
  auto original_state = file_.rdstate();

  file_.clear(std::ios::goodbit);
  file_.seekg(0, std::ios::beg);

  std::size_t line_count = 0;
  std::string line;
  while(std::getline(file_, line)) {
    line_count++;
  }

  file_.clear(std::ios::goodbit);
  file_.seekg(original_pos);
  file_.setstate(original_state);

  return line_count;
}

bool ev::PlainTextReader::read_(ev::Event &e) {
  std::string line;
  if(std::getline(file_, line)) {
    if(replace_) {
      line = std::regex_replace(line, separator_, " ");
    }
    std::stringstream iss(line, std::ios_base::in);
    parser_(iss, e);
    return true;
  }
  return false;
}

void ev::PlainTextReader::reset_() {
  file_.clear(std::ios::goodbit);
  file_.seekg(0, std::ios::beg);
}
