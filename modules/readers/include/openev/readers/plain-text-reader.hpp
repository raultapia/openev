/*!
\file plain-text-reader.hpp
\brief Plain text reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_PLAIN_TEXT_READER_HPP
#define OPENEV_READERS_PLAIN_TEXT_READER_HPP

#include "openev/core/types.hpp"
#include "openev/readers/abstract-reader.hpp"
#include <fstream>
#include <regex>
#include <string>

namespace ev {

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
  /*!
  Contructor using filename.
  \param filename Filename of the dataset
  TODO
  */
  explicit PlainTextReader(const std::string &filename, const PlainTextReaderColumns columns = PlainTextReaderColumns::TXYP, const std::string &separator = " ");

  /*! \cond INTERNAL */
  ~PlainTextReader() override;
  PlainTextReader(const PlainTextReader &) = delete;
  PlainTextReader(PlainTextReader &&) noexcept = delete;
  PlainTextReader &operator=(const PlainTextReader &) = delete;
  PlainTextReader &operator=(PlainTextReader &&) noexcept = delete;
  /*! \endcond */

private:
  std::fstream file_;
  std::regex separator_;
  std::function<void(std::stringstream &, ev::Event &)> parser_;
  bool replace_;
  void reset_() override;
  bool read_(Event &e) override;
};

} // namespace ev

#endif // OPENEV_READERS_PLAIN_TEXT_READER_HPP
