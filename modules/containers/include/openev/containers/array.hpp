/*!
\file array.hpp
\brief Array container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_ARRAY_HPP
#define OPENEV_CONTAINERS_ARRAY_HPP

#include "openev/containers/abstract-container.hpp"
#include "openev/core/types.hpp"
#include <array>
#include <cstddef>
#include <opencv2/core/types.hpp>

namespace ev {
constexpr bool USING_ARRAY_HPP = true;

/*!
\brief This class extends std::array to implement event arrays. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/array">here</a>.

Event arrays inherit all the properties from standard C++ arrays. Events in the array are stored contiguously.
*/
template <typename T, std::size_t N>
class Array_ : public std::array<Event_<T>, N>, public AbstractContainer_<Array_<T, N>, T> {
  static_assert(N > 0, "ev::Array_: the size must be greater than zero.");
  using std::array<Event_<T>, N>::array;
  using ResultType = TimeType;
};

template <std::size_t N>
using Arrayi = Array_<int, N>; /*!< Alias for Array_ using int */
template <std::size_t N>
using Arrayl = Array_<long, N>; /*!< Alias for Array_ using long */
template <std::size_t N>
using Arrayf = Array_<float, N>; /*!< Alias for Array_ using float */
template <std::size_t N>
using Arrayd = Array_<double, N>; /*!< Alias for Array_ using double */
template <std::size_t N>
using Array = Arrayi<N>; /*!< Alias for Array_ using int */
} // namespace ev

#endif // OPENEV_CONTAINERS_ARRAY_HPP
