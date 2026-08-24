/*!
\file reader.hpp
\brief Dataset reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_ABSTRACT_READER_HPP
#define OPENEV_READERS_ABSTRACT_READER_HPP

#include "openev/containers/queue.hpp"
#include <atomic>
#include <cstddef>
#include <mutex>
#include <thread>

namespace ev {

/*!
\brief This is an auxiliary class. This class cannot be instanced.
*/
class AbstractReader_ {
public:
  /*! \cond INTERNAL */
  virtual ~AbstractReader_();
  AbstractReader_(const AbstractReader_ &) = delete;
  AbstractReader_(AbstractReader_ &&) noexcept = delete;
  AbstractReader_ &operator=(const AbstractReader_ &) = delete;
  AbstractReader_ &operator=(AbstractReader_ &&) noexcept = delete;
  /*! \endcond */

  /*!
  \brief Constructor for AbstractReader_.
  \param buffer_size The size of the buffer to be used by the reader.
  */
  AbstractReader_(const std::size_t buffer_size, const bool use_threading);

  /*!
  \brief Returns a reference to the internal buffer (Queue) containing the data.
  \return Reference to the internal Queue buffer.
  */
  inline Queue &data() {
    if(!eof_ && buffer_.size() < bufferSize_) {
      if(!updateBuffer_()) {
        eof_.store(true);
      }
    }
    return buffer_;
  }

protected:
  const std::size_t bufferSize_;
  std::thread thread_;
  Queue buffer_;
  std::mutex bufferMutex_;
  std::atomic<bool> threadRunning_{};
  std::atomic<bool> eof_{false};

  virtual bool updateBuffer_() = 0;

private:
  void threadFunction();
};

} // namespace ev

#endif // OPENEV_READERS_ABSTRACT_READER_HPP
