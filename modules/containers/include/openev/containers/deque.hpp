/*!
\file deque.hpp
\brief Deque container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_DEQUE_HPP
#define OPENEV_CONTAINERS_DEQUE_HPP

#include "openev/containers/abstract-container.hpp"
#include "openev/core/types.hpp"
#include <deque>
#include <opencv2/core/types.hpp>

namespace ev {
constexpr bool USING_DEQUE_HPP = true;

/*!
\brief This class extends std::deque to implement event deques. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/deque">here</a>.

Event deques inherit all the properties from standard C++ deques. Event deques are double-ended queues that allow fast insertion and deletion at both their beginning and their end.
*/
template <typename T>
class Deque_ : public std::deque<Event_<T>>, public AbstractContainer_<Deque_<T>, T> {
  using std::deque<Event_<T>>::deque;
  using ResultType = TimeType;
};
using Dequei = Deque_<int>;    /*!< Alias for Deque_ using int */
using Dequel = Deque_<long>;   /*!< Alias for Deque_ using long */
using Dequef = Deque_<float>;  /*!< Alias for Deque_ using float */
using Dequed = Deque_<double>; /*!< Alias for Deque_ using double */
using Deque = Dequei;          /*!< Alias for Deque_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_DEQUE_HPP
