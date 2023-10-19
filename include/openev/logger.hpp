/*!
\file logger.hpp
\brief Logger utility.
\author Raul Tapia
*/
#ifndef OPENEV_LOGGER_HPP
#define OPENEV_LOGGER_HPP

#include <iostream>
#include <memory>

namespace ev::logger {

/*!
\brief Log message at info level.
\param message Message
*/
inline void info(const char *message) {
  std::cout << "INFO. openev: " << message << '\n';
}

/*!
\brief Log message at warning level.
\param message Message
*/
inline void warning(const char *message) {
  std::cout << "WARNING. openev: " << message << '\n';
}

/*!
\brief Log message at error level.
\param message Message
*/
inline void error(const char *message) {
  throw std::runtime_error("ERROR. openev: " + std::string(message, std::allocator<char>()));
}

} // namespace ev::logger

#endif // OPENEV_LOGGER_HPP
