/*!
\file containers.hpp
\brief Array container for basic event structures.
\author Raul Tapia
*/
#ifndef OPENEV_CONTAINERS_ARRAY_HPP
#define OPENEV_CONTAINERS_ARRAY_HPP

#include "openev/core/types.hpp"
#include <array>
#include <cstddef>

namespace ev {
/*!
\brief This class extends std::array to implement event arrays. For more information, please refer <a href="https://en.cppreference.com/w/cpp/container/array">here</a>.

Event arrays inherit all the properties from standard C++ arrays. Events in the array are stored contiguously.
*/
template <typename T, std::size_t N>
class Array_ : public std::array<T, N> {
  using std::array<T, N>::array;

public:
  /*!
  \brief Time difference between the last and the first event in the array.
  \return Time difference
  */
  [[nodiscard]] inline double duration() const {
    return (std::array<T, N>::back()).t - (std::array<T, N>::front()).t;
  }

  /*!
  \brief Compute event rate as the ratio between the number of events and the time difference between the last and the first event in the array.
  \return Event rate
  */
  [[nodiscard]] inline double rate() const {
    return std::array<T, N>::size() / duration();
  }
};
template <std::size_t N>
using Arrayi = Array_<Eventi, N>; /*!< Alias for Array_ using Eventi */
template <std::size_t N>
using Arrayl = Array_<Eventl, N>; /*!< Alias for Array_ using Eventl */
template <std::size_t N>
using Arrayf = Array_<Eventf, N>; /*!< Alias for Array_ using Eventf */
template <std::size_t N>
using Arrayd = Array_<Eventd, N>; /*!< Alias for Array_ using Eventd */
template <std::size_t N>
using Array = Array_<Event, N>; /*!< Alias for Array_ using Event */
template <std::size_t N>
using AugmentedArrayi = Array_<AugmentedEventi, N>; /*!< Alias for augmented Array_ using AugmentedEventi */
template <std::size_t N>
using AugmentedArrayl = Array_<AugmentedEventl, N>; /*!< Alias for augmented Array_ using AugmentedEventl */
template <std::size_t N>
using AugmentedArrayf = Array_<AugmentedEventf, N>; /*!< Alias for augmented Array_ using AugmentedEventf */
template <std::size_t N>
using AugmentedArrayd = Array_<AugmentedEventd, N>; /*!< Alias for augmented Array_ using AugmentedEventd */
template <std::size_t N>
using AugmentedArray = Array_<AugmentedEvent, N>; /*!< Alias for augmented Array_ using AugmentedEvent */
} // namespace ev

#endif // OPENEV_CONTAINERS_ARRAY_HPP
