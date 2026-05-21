/*!
\file core.hpp
\brief Include all the OpenEV core module.
\author Raul Tapia
*/
#ifndef OPENEV_CORE_HPP
#define OPENEV_CORE_HPP

#include "openev/core/matrices.hpp"
#include "openev/core/types.hpp"

namespace {
inline void workaround() {
  (void)ev::USING_TYPES_HPP;
  (void)ev::USING_MATRICES_HPP;
}
} // namespace

#endif // OPENEV_CORE_HPP
