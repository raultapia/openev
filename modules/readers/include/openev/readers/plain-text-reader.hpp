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
#include <vector>

namespace ev {
[[maybe_unused]] constexpr bool USING_PLAIN_TEXT_READER_HPP = true;

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
  explicit PlainTextReader(const std::string &filename, const PlainTextReaderColumns columns = PlainTextReaderColumns::TXYP, const std::string &separator = " ");
  ~PlainTextReader() override;

  /*! \cond INTERNAL */
  PlainTextReader(const PlainTextReader &) = delete;
  PlainTextReader(PlainTextReader &&) noexcept = delete;
  PlainTextReader &operator=(const PlainTextReader &) = delete;
  PlainTextReader &operator=(PlainTextReader &&) noexcept = delete;
  /*! \endcond */

protected:
  std::size_t read_(Event *events, const std::size_t max) override;
  void reset_() override;

private:
  std::fstream file_;
  PlainTextReaderColumns columns_;
  char sep_char_;       // non-zero when separator is a single non-space char
  std::string sep_str_; // non-empty when separator is multi-char
  std::string line_;    // reused across calls
  std::vector<char> chunk_;
  std::size_t pos_{0};
  std::size_t end_{0};
  bool exhausted_{false};
  bool failed_{false};

  bool nextLine_(const char *&line, const char *&lineEnd);
};

} // namespace ev

#endif // OPENEV_READERS_PLAIN_TEXT_READER_HPP
