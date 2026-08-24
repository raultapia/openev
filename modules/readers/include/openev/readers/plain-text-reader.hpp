/*!
\file plain-text-reader.hpp
\brief Plain text reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_PLAIN_TEXT_READER_HPP
#define OPENEV_READERS_PLAIN_TEXT_READER_HPP

#include "openev/readers/abstract-reader.hpp"
#include <cstddef>
#include <fstream>
#include <stdint.h>
#include <string>

namespace ev {
constexpr bool USING_PLAIN_TEXT_READER_HPP = true;

enum PlainTextReaderColumns : uint8_t {
  TXYP,
  XYTP,
  PTXY,
  PXYT
};

/*!
\brief This class extends AbstractReader_ to read dataset in plain text format.
*/
class PlainTextReader : public AbstractReader_ {
public:
  explicit PlainTextReader(const std::string &filename, const PlainTextReaderColumns columns = PlainTextReaderColumns::TXYP, const std::string &separator = " ", const std::size_t buffer_size = 0, const bool use_threading = false);
  ~PlainTextReader();

  /*! \cond INTERNAL */
  PlainTextReader(const PlainTextReader &) = delete;
  PlainTextReader(PlainTextReader &&) noexcept = delete;
  PlainTextReader &operator=(const PlainTextReader &) = delete;
  PlainTextReader &operator=(PlainTextReader &&) noexcept = delete;
  /*! \endcond */

private:
  std::fstream file_;
  PlainTextReaderColumns columns_;
  char sep_char_;       // non-zero when separator is a single non-space char
  std::string sep_str_; // non-empty when separator is multi-char

  bool updateBuffer_() override;
};

} // namespace ev

#endif // OPENEV_READERS_PLAIN_TEXT_READER_HPP
