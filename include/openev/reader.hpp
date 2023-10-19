/*!
\file reader.hpp
\brief Dataset reader.
\author Raul Tapia
*/
#ifndef OPENEV_READER_HPP
#define OPENEV_READER_HPP

#include "openev/containers.hpp"
#include "openev/types.hpp"
#include <fstream>

namespace ev {

/*!
\brief This is an auxiliary class. This class cannot be instanced.
*/
class AbstractReader_ {
public:
  /*! \cond INTERNAL */
  AbstractReader_() = default;
  virtual ~AbstractReader_() = default;
  AbstractReader_(const AbstractReader_ &) = delete;
  AbstractReader_(AbstractReader_ &&) noexcept = delete;
  AbstractReader_ &operator=(const AbstractReader_ &) = delete;
  AbstractReader_ &operator=(AbstractReader_ &&) noexcept = delete;
  /*! \endcond */

  /*!
  \brief Start reading from the first event in the dataset.
  */
  virtual void reset() = 0;

  /*!
  \brief Get next event from the dataset.
  \param e Event
  \return True if new event available
  */
  virtual bool next(Event &e) = 0;

  /*!
  \brief Get next n event from the dataset.
  \param n Number of events to get
  \param vector Event vector
  \return True if vector populated with n new events
  */
  bool next(std::size_t n, EventVector &vector);

  /*!
  \brief Get next n event from the dataset.
  \param n Number of events to get
  \param queue Event queue
  \return True if queue populated with n new events
  */
  bool next(std::size_t n, EventBuffer &eb);
};

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

  /*!
  \brief Start reading from the first event in the dataset.
  */
  void reset() override;

  /*!
  \brief Get next event from the dataset.
  \param e Event
  \return True if new event available
  */
  bool next(Event &e) override;

private:
  std::fstream file_;
};

} // namespace ev

#endif // OPENEV_READER_HPP
