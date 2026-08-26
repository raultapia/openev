/*!
\file plain-text-reader.cpp
\brief Implementation of plain-text-reader.
\author Raul Tapia
*/
#include "openev/readers/plain-text-reader.hpp"
#include <algorithm>
#include <cstdio>
#include <opencv2/core/utils/logger.hpp>

ev::PlainTextReader::PlainTextReader(const std::string &filename, const PlainTextReaderColumns columns /*= PlainTextReaderColumns::TXYP*/, const std::string &separator /*= " "*/, const std::size_t buffer_size /*= 0*/, const bool use_threading /*=false*/) : AbstractReader_{buffer_size, use_threading}, file_{filename, std::ios::in}, columns_{columns}, sep_char_{0} {
  if(separator != " ") {
    if(separator.size() == 1) {
      sep_char_ = separator[0];
    } else {
      sep_str_ = separator;
    }
  }
  if(!file_.is_open()) {
    CV_LOG_ERROR(nullptr, "ev::PlainTextReader: Could not open file.");
  }
}

ev::PlainTextReader::~PlainTextReader() {
  if(file_.is_open()) {
    file_.close();
  }
}

bool ev::PlainTextReader::updateBuffer_() {
  std::string line;
  if(!std::getline(file_, line)) {
    return false;
  }

  if(sep_char_) {
    std::replace(line.begin(), line.end(), sep_char_, ' ');
  } else if(!sep_str_.empty()) {
    std::size_t pos = 0;
    while((pos = line.find(sep_str_, pos)) != std::string::npos) {
      line.replace(pos++, sep_str_.size(), " ");
    }
  }

  Event e;
  int pi;
  switch(columns_) {
  case PlainTextReaderColumns::TXYP:
    if(std::sscanf(line.c_str(), "%lf %d %d %d", &e.t, &e.x, &e.y, &pi) != 4) return false;
    break;
  case PlainTextReaderColumns::XYTP:
    if(std::sscanf(line.c_str(), "%d %d %lf %d", &e.x, &e.y, &e.t, &pi) != 4) return false;
    break;
  case PlainTextReaderColumns::PTXY:
    if(std::sscanf(line.c_str(), "%d %lf %d %d", &pi, &e.t, &e.x, &e.y) != 4) return false;
    break;
  case PlainTextReaderColumns::PXYT:
    if(std::sscanf(line.c_str(), "%d %d %d %lf", &pi, &e.x, &e.y, &e.t) != 4) return false;
    break;
  default:
    CV_LOG_ERROR(nullptr, "ev::PlainTextReader: No column order selected.");
    return false;
  }
  e.p = static_cast<bool>(pi > 0);
  buffer_.push(e);
  return true;
}
