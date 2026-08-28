/*!
\file vector.hpp
\brief Vector container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_VECTOR_HPP
#define OPENEV_CONTAINERS_VECTOR_HPP

#include "openev/containers/abstract-container.hpp"
#include "openev/core/types.hpp"
#include <opencv2/core/types.hpp>
#include <vector>

namespace ev {
constexpr bool USING_VECTOR_HPP = true;

/*!
\brief This class extends std::vector to implement event vectors. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/vector">here</a>.

Event vectors inherit all the properties from standard C++ vectors. Events in the vector are stored contiguously.
*/
template <typename T>
class Vector_ : public std::vector<Event_<T>>, public AbstractContainer_<Vector_<T>, T> {
  using std::vector<Event_<T>>::vector;
  using ResultType = TimeType;
};
using Vectori = Vector_<int>;    /*!< Alias for Vector_ using int */
using Vectorl = Vector_<long>;   /*!< Alias for Vector_ using long */
using Vectorf = Vector_<float>;  /*!< Alias for Vector_ using float */
using Vectord = Vector_<double>; /*!< Alias for Vector_ using double */
using Vector = Vectori;          /*!< Alias for Vector_ using Event */
} // namespace ev

#endif // OPENEV_CONTAINERS_VECTOR_HPP
