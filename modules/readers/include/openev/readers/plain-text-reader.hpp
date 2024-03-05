/*!
\file reader.hpp
\brief Dataset reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_PLAIN_TEXT_READER_HPP
#define OPENEV_READERS_PLAIN_TEXT_READER_HPP

#include "openev/containers.hpp"
#include "openev/core/types.hpp"
#include "openev/readers/abstract-reader.hpp"
#include <fstream>
#include <sstream>

namespace ev {

/*!
\brief This class extends AbstractReader_ to read dataset in plain text format.
*/
class PlainTextReader : public AbstractReader_ {
public:
  /*!
  Contructor using filename.
  \param filename Filename of the dataset
  */
  explicit PlainTextReader(const std::string &filename);

  /*! \cond INTERNAL */
  ~PlainTextReader() override;
  PlainTextReader(const PlainTextReader &) = delete;
  PlainTextReader(PlainTextReader &&) noexcept = delete;
  PlainTextReader &operator=(const PlainTextReader &) = delete;
  PlainTextReader &operator=(PlainTextReader &&) noexcept = delete;
  /*! \endcond */

private:
  std::fstream file_;
  void reset_() override;
  bool next_(Event &e) override;
};

} // namespace ev

#endif // OPENEV_READERS_PLAIN_TEXT_READER_HPP
