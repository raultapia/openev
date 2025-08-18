/*!
\file openev.hpp
\brief Include all the OpenEV modules.
\author Raul Tapia
*/

#ifndef OPENEV_OPENEV_HPP
#define OPENEV_OPENEV_HPP

#include "openev/options.hpp"

#if OE_IS_ENABLED_MODULE_ALGORITHMS
#include "openev/algorithms.hpp"
#endif // OE_IS_ENABLED_MODULE_ALGORITHMS

#if OE_IS_ENABLED_MODULE_CONTAINERS
#include "openev/containers.hpp"
#endif // OE_IS_ENABLED_MODULE_CONTAINERS

#if OE_IS_ENABLED_MODULE_CORE
#include "openev/core.hpp"
#endif // OE_IS_ENABLED_MODULE_CORE

#if OE_IS_ENABLED_MODULE_DEVICES
#include "openev/devices.hpp"
#endif // OE_IS_ENABLED_MODULE_DEVICES

#if OE_IS_ENABLED_MODULE_READERS
#include "openev/readers.hpp"
#endif // OE_IS_ENABLED_MODULE_READERS

#if OE_IS_ENABLED_MODULE_REPRESENTATIONS
#include "openev/representations.hpp"
#endif // OE_IS_ENABLED_MODULE_REPRESENTATIONS

#if OE_IS_ENABLED_MODULE_UTILS
#include "openev/utils.hpp"
#endif // OE_IS_ENABLED_MODULE_UTILS

#endif // OPENEV_OPENEV_HPP
