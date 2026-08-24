/*!
\file readers.hpp
\brief Include all the OpenEV readers module.
\author Raul Tapia
*/
#ifndef OPENEV_READERS_HPP
#define OPENEV_READERS_HPP

#include "openev/readers/hdf5-reader.hpp"
#include "openev/readers/plain-text-reader.hpp"

namespace {
inline void workaround() {
  (void)ev::USING_HDF5_READER_HPP;
  (void)ev::USING_PLAIN_TEXT_READER_HPP;
}
} // namespace

#endif // OPENEV_READERS_HPP
