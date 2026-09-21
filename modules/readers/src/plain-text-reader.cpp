/*!
\file plain-text-reader.cpp
\brief Implementation of plain-text-reader.
\author Raul Tapia
*/
#include "openev/readers/plain-text-reader.hpp"
#include <charconv>
#include <cstddef>
#include <cstring>
#include <ios>
#include <string>
#include <opencv2/core/utils/logger.hpp>
#include <system_error>

namespace {
constexpr std::size_t CHUNK = 1U << 16U;

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

ev::PlainTextReader::PlainTextReader(const std::string &filename, const PlainTextReaderColumns columns /*= PlainTextReaderColumns::TXYP*/, const std::string &separator /*= " "*/) : file_{filename, std::ios::in | std::ios::binary}, columns_{columns}, sep_char_{0}, chunk_(CHUNK) {
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
  stop_();
  if(file_.is_open()) {
    file_.close();
  }
}

void ev::PlainTextReader::reset_() {
  file_.clear();
  file_.seekg(0, std::ios::beg);
  pos_ = 0;
  end_ = 0;
  exhausted_ = false;
  failed_ = false;
}

bool ev::PlainTextReader::nextLine_(const char *&line, const char *&lineEnd) {
  while(true) {
    const char *const begin = chunk_.data() + pos_;
    const auto *const newline = static_cast<const char *>(std::memchr(begin, '\n', end_ - pos_));
    if(newline != nullptr) {
      line = begin;
      lineEnd = newline;
      pos_ = static_cast<std::size_t>(newline - chunk_.data()) + 1;
      return true;
    }
    if(exhausted_) {
      if(pos_ == end_) {
        return false;
      }
      line = begin;
      lineEnd = chunk_.data() + end_;
      pos_ = end_;
      return true;
    }

    const std::size_t pending = end_ - pos_;
    std::memmove(chunk_.data(), begin, pending);
    pos_ = 0;
    end_ = pending;
    if(end_ == chunk_.size()) {
      chunk_.resize(2 * chunk_.size());
    }
    file_.read(chunk_.data() + end_, static_cast<std::streamsize>(chunk_.size() - end_));
    const auto got = static_cast<std::size_t>(file_.gcount());
    end_ += got;
    exhausted_ = got == 0;
  }
}

std::size_t ev::PlainTextReader::read_(Event *events, const std::size_t max) {
  std::size_t count = 0;
  while(count < max && !failed_) {
    const char *it = nullptr;
    const char *end = nullptr;
    if(!nextLine_(it, end)) {
      break;
    }
    const char *const line = it;
    const char *const lineEnd = end;

    if(!sep_str_.empty()) {
      line_.assign(it, end);
      std::size_t pos = 0;
      while((pos = line_.find(sep_str_, pos)) != std::string::npos) {
        line_.replace(pos++, sep_str_.size(), " ");
      }
      it = line_.data();
      end = it + line_.size();
    }

    Event &e = events[count];
    int pi = 0;
    bool parsed = false;
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
      CV_LOG_WARNING(nullptr, "ev::PlainTextReader: could not parse a line, stopping there: \"" << std::string(line, lineEnd) << "\"");
      failed_ = true;
      break;
    }
    e.p = static_cast<bool>(pi > 0);
    count++;
  }
  return count;
}
