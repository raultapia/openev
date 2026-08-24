/*!
\file abstract-reader.cpp
\brief Implementation of abstract-reader.
\author Raul Tapia
*/
#include "openev/readers/abstract-reader.hpp"

ev::AbstractReader_::AbstractReader_(const std::size_t buffer_size, const bool use_threading) : bufferSize_{buffer_size} {
  if(use_threading) {
    threadRunning_.store(true);
    thread_ = std::thread(&AbstractReader_::threadFunction, this);
  }
}

ev::AbstractReader_::~AbstractReader_() {
  if(thread_.joinable()) {
    threadRunning_.store(false);
    thread_.join();
  }
}

void ev::AbstractReader_::threadFunction() {
  while(!eof_ && threadRunning_.load()) {
    {
      std::unique_lock<std::mutex> lock(bufferMutex_);
      if(buffer_.size() < bufferSize_) {
        if(!updateBuffer_()) {
          eof_.store(true);
        }
      }
    }
  }
}
