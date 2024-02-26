/*!
\file reader.hpp
\brief Dataset reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_ABSTRACT_READER_HPP
#define OPENEV_READERS_ABSTRACT_READER_HPP

#include "openev/containers.hpp"
#include "openev/core/types.hpp"
#include <fstream>
#include <sstream>

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
  \note The behaviour of reset should be implemented in the derived classes.
  */
  inline void reset() {
    return reset_();
  }

  /*!
  \brief Get next event from the dataset.
  \param e Event
  \return True if new event available
  \note The behaviour of next should be implemented in the derived classes.
  */
  inline bool next(Event &e) {
    return next_(e);
  }

  /*!
  \brief Get next events from the dataset.
  \param arrat Event array
  \return True if array fully populated events
  */
  template <std::size_t N>
  bool next(Array<N> &array) {
    if constexpr(N < 1) {
      return true;
    }

    ev::Event e;
    for(std::size_t i = 0; i < N; i++) {
      if(!next_(e)) {
        return false;
      }
      array[i] = e;
    }
    return true;
  }

  /*!
  \brief Get next n events from the dataset.
  \param n Number of events to get
  \param vector Event vector
  \return True if vector populated with n new events
  */
  bool next(int n, Vector &vector);

  /*!
  \brief Get next n events from the dataset.
  \param n Number of events to get
  \param queue Event queue
  \param keep_same_size Pop one event for each insertion
  \return True if queue populated with n new events
  */
  bool next(int n, Queue &queue, const bool keep_same_size = false);

protected:
  virtual void reset_() = 0;
  virtual bool next_(Event &e) = 0;
};

} // namespace ev

#endif // OPENEV_READERS_ABSTRACT_READER_HPP
