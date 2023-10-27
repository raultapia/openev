/*!
\file containers.hpp
\brief Containers for basic event-based vision structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_HPP
#define OPENEV_CONTAINERS_HPP

#include "openev/logger.hpp"
#include "openev/types.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <queue>

namespace ev {
/*! \cond INTERNAL */
template <typename T, std::size_t N>
class EventArray_;

template <typename T>
class EventVector_;

template <typename T>
class EventQueue_;
/*! \endcond */

/*!
\brief This class extends std::array<Event, N> to implement event arrays. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/array">here</a>.

Event arrays inherit all the properties from standard C++ arrays. Events in the array are stored contiguously.
*/
template <typename T, std::size_t N>
class EventArray_ : public std::array<Event_<T>, N> {
  using std::array<Event_<T>, N>::array;

public:
  /*!
  \brief Time difference between the last and the first event in the array.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (std::array<Event_<T>, N>::back()).t - (std::array<Event_<T>, N>::front()).t;
  }
};
template <std::size_t N>
using EventArrayi = EventArray_<int, N>; /*!< Alias for EventArray_ using int */
template <std::size_t N>
using EventArrayl = EventArray_<long, N>; /*!< Alias for EventArray_ using long */
template <std::size_t N>
using EventArrayf = EventArray_<float, N>; /*!< Alias for EventArray_ using float */
template <std::size_t N>
using EventArrayd = EventArray_<double, N>; /*!< Alias for EventArray_ using double */
template <std::size_t N>
using EventArray = EventArrayi<N>; /*!< Alias for EventArray_ using int */

/*!
\brief This class extends std::vector<Event> to implement event vectors. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/vector">here</a>.

Event vectors inherit all the properties from standard C++ vectors. Events in the vector are stored contiguously.
*/
template <typename T>
class EventVector_ : public std::vector<Event_<T>> {
  using std::vector<Event_<T>>::vector;

public:
  /*!
  \brief Push back an event.
  \param e Event to push back
  */
  inline void push_back(const Event_<T> &e) {
    std::vector<Event_<T>>::push_back(e);
  }

  /*!
  \brief Emplace back an event.
  \param args Constructor arguments
  */
  template <typename... Args>
  inline void emplace_back(Args &&...args) {
    std::vector<Event_<T>>::emplace_back(std::forward<Args>(args)...);
  }

  /*!
  \brief Push back elements from an array of events.
  \param array Event array to push back
  */
  template <std::size_t N>
  inline void push_back(const EventArray_<T, N> &array) {
    EventVector_<T>::reserve(EventVector_<T>::size() + array.size());
    EventVector_<T>::insert(EventVector_<T>::end(), array.begin(), array.end());
  }

  /*!
  \brief Push back elements from a queue of events.
  \param queue Event queue to push back
  */
  inline void push_back(EventQueue_<T> &queue);

  /*!
  \brief Time difference between the last and the first event in the vector.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (std::vector<Event_<T>>::back()).t - (std::vector<Event_<T>>::front()).t;
  }
};
using EventVectori = EventVector_<int>;    /*!< Alias for EventVector_ using int */
using EventVectorl = EventVector_<long>;   /*!< Alias for EventVector_ using long */
using EventVectorf = EventVector_<float>;  /*!< Alias for EventVector_ using float */
using EventVectord = EventVector_<double>; /*!< Alias for EventVector_ using double */
using EventVector = EventVectori;          /*!< Alias for EventVector_ using int */

/*!
\brief This class extends std::queue<Event> to implement event queues. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/queue">here</a>.

Event queues inherit all the properties from standard C++ queues. Events queues are FIFO data structures not intended to be directly iterated.
*/
template <typename T>
class EventQueue_ : public std::queue<Event_<T>> {
  using std::queue<Event_<T>>::queue;

public:
  /*!
  \brief Push an event.
  \param e Event to push
  */
  inline void push(const Event_<T> &e) {
    std::queue<Event_<T>>::push(e);
  }

  /*!
  \brief Emplace back an event.
  \param args Constructor arguments
  */
  template <typename... Args>
  inline void emplace(Args &&...args) {
    std::queue<Event_<T>>::emplace(std::forward<Args>(args)...);
  }

  /*!
  \brief Push elements from an array of events.
  \param array Event array to push
  */
  template <std::size_t N>
  inline void push(const EventArray_<T, N> &array) {
    for(const Event_<T> &e : array) {
      std::queue<Event_<T>>::emplace(std::move(e));
    }
  }

  /*!
  \brief Push elements from a vector of events.
  \param vector Event vector to push
  */
  inline void push(const EventVector_<T> &vector);

  /*!
  \brief Time difference between the last and the first event in the queue.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (std::queue<Event_<T>>::back()).t - (std::queue<Event_<T>>::front()).t;
  }
};
using EventQueuei = EventQueue_<int>;    /*!< Alias for EventQueue_ using int */
using EventQueuel = EventQueue_<long>;   /*!< Alias for EventQueue_ using long */
using EventQueuef = EventQueue_<float>;  /*!< Alias for EventQueue_ using float */
using EventQueued = EventQueue_<double>; /*!< Alias for EventQueue_ using double */
using EventQueue = EventQueuei;          /*!< Alias for EventQueue_ using int */

template <typename T>
inline void EventVector_<T>::push_back(EventQueue_<T> &queue) {
  reserve(std::vector<Event_<T>>::size() + queue.size());
  while(!queue.empty()) {
    std::vector<Event_<T>>::emplace_back(std::move(queue.front()));
    queue.pop();
  }
}

template <typename T>
inline void EventQueue_<T>::push(const EventVector_<T> &vector) {
  for(const Event_<T> &e : vector) {
    std::queue<Event_<T>>::emplace(std::move(e));
  }
}

} // namespace ev

#endif // OPENEV_CONTAINERS_HPP
