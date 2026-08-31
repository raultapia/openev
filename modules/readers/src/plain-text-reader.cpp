/*!
\file plain-text-reader.cpp
\brief Implementation of plain-text-reader.
\author Raul Tapia
*/
#include "openev/readers/plain-text-reader.hpp"
#include <charconv>
#include <opencv2/core/utils/logger.hpp>
#include <system_error>

namespace {
template <typename T>
bool field(const char *&it, const char *const end, T &value, const char separator) {
  while(it != end && (*it == ' ' || *it == '\t' || *it == '\r' || *it == separator)) {
    it++;
  }
  if(it != end && *it == '+') {
    it++;
  }
  const std::from_chars_result result = std::from_chars(it, end, value);
  if(result.ec != std::errc{}) {
    return false;
  }
  it = result.ptr;
  return true;
}
} // namespace

ev::PlainTextReader::PlainTextReader(const std::string &filename, const PlainTextReaderColumns columns /*= PlainTextReaderColumns::TXYP*/, const std::string &separator /*= " "*/, const std::size_t buffer_size /*= 0*/, const bool use_threading /*=false*/) : AbstractReader_{buffer_size, use_threading}, file_{filename, std::ios::in}, columns_{columns}, sep_char_{0} {
  if(separator != " ") {
    if(separator.size() == 1) {
      sep_char_ = separator[0];
    } else {
      sep_str_ = separator;
    }
  }
  if(!file_.is_open()) {
    CV_Error(cv::Error::StsError, "ev::PlainTextReader: Could not open file.");
  }
}

ev::PlainTextReader::~PlainTextReader() {
  if(file_.is_open()) {
    file_.close();
  }
}

bool ev::PlainTextReader::updateBuffer_() {
  if(!std::getline(file_, line_)) {
    return false;
  }

  if(!sep_str_.empty()) {
    std::size_t pos = 0;
    while((pos = line_.find(sep_str_, pos)) != std::string::npos) {
      line_.replace(pos++, sep_str_.size(), " ");
    }
  }

  Event e;
  int pi = 0;
  bool parsed = false;
  const char *it = line_.data();
  const char *const end = it + line_.size();
  switch(columns_) {
  case PlainTextReaderColumns::TXYP:
    parsed = field(it, end, e.t, sep_char_) && field(it, end, e.x, sep_char_) && field(it, end, e.y, sep_char_) && field(it, end, pi, sep_char_);
    break;
  case PlainTextReaderColumns::XYTP:
    parsed = field(it, end, e.x, sep_char_) && field(it, end, e.y, sep_char_) && field(it, end, e.t, sep_char_) && field(it, end, pi, sep_char_);
    break;
  case PlainTextReaderColumns::PTXY:
    parsed = field(it, end, pi, sep_char_) && field(it, end, e.t, sep_char_) && field(it, end, e.x, sep_char_) && field(it, end, e.y, sep_char_);
    break;
  case PlainTextReaderColumns::PXYT:
    parsed = field(it, end, pi, sep_char_) && field(it, end, e.x, sep_char_) && field(it, end, e.y, sep_char_) && field(it, end, e.t, sep_char_);
    break;
  default:
    CV_Error(cv::Error::StsBadArg, "ev::PlainTextReader: No column order selected.");
  }
  if(!parsed) {
    CV_LOG_WARNING(nullptr, "ev::PlainTextReader: could not parse a line, stopping there: \"" << line_ << "\"");
    return false;
  }
  e.p = static_cast<bool>(pi > 0);
  buffer_.push(e);
  return true;
}
