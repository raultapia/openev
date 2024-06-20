/*!
\file reader.hpp
\brief Dataset reader.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_ABSTRACT_READER_HPP
#define OPENEV_READERS_ABSTRACT_READER_HPP

#include "openev/containers/array.hpp"
#include "openev/containers/queue.hpp"
#include "openev/containers/vector.hpp"
#include "openev/core/types.hpp"
#include <cstddef>

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
  \brief Get next event from the dataset.
  \param e Event
  \return True if new event available
  \note The behaviour of next should be implemented in the derived classes.
  */
  inline bool next(Event &e) {
    return read_(e);
  }

  /*!
  \brief Get next events from the dataset.
  \param arrat Event array
  \return True if array fully populated events
  */
  template <std::size_t N>
  bool next(Array<N> &array) {
    for(Event &a : array) {
      if(!read_(a)) {
        return false;
      }
    }
    return true;
  }

  /*!
  \brief Get next n events from the dataset.
  \param vector Event vector
  \param n Number of events to get
  \return True if vector populated with n new events
  */
  bool next_n(Vector &vector, const int n);

  /*!
  \brief Get next n events from the dataset.
  \param n Number of events to get
  \param queue Event queue
  \param keep_size If true, pop one event for each insertion to maintain queue size.
  \return True if queue populated with n new events
  */
  bool next_n(Queue &queue, const int n, const bool keep_size = false);

  /*!
  \brief Get the next events until the specified duration is reached.
  \param vector Event vector to store the events.
  \param t Duration to get events for.
  \return True if the vector is populated with events for the specified duration.
  */
  bool next_t(Vector &vector, const double t);

  /*!
  \brief Get the next events until the specified duration is reached and store them in a queue.
  \param t Duration to get events for.
  \param queue Event queue to store the events.
  \param keep_size If true, pop one event for each insertion to maintain queue size.
  \return True if the queue is populated with events for the specified duration.
  */
  bool next_t(Queue &queue, const double t, const bool keep_size = false);

  /*!
  \brief Skip the next n events in the dataset.
  \param n Number of events to skip.
  \return True if the skip was successful.
  */
  bool skip_n(int n);

  /*!
  \brief Skip events for the specified duration.
  \param t Duration to skip events for.
  \return True if the skip was successful.
  */
  bool skip_t(const double t);

  /*!
  \brief Start reading from the first event in the dataset.
  \note The behaviour of reset should be implemented in the derived classes.
  */
  inline void reset() {
    return reset_();
  }

  /*!
  TODO
  */
  std::size_t count() {
    std::size_t cnt = 0;
    reset_();
    while(skip_n(1)) {
      cnt++;
    }
    reset_();
    return cnt;
  }

protected:
  virtual bool read_(Event &e) = 0;
  virtual void reset_() = 0;
};

} // namespace ev

#endif // OPENEV_READERS_ABSTRACT_READER_HPP
