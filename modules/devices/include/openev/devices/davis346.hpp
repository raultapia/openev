/*!
\file davis346.hpp
\brief Specialization of DAVIS device for DAVIS346.
\author Raul Tapia
*/
#ifndef OPENEV_DEVICES_DAVIS346_HPP
#define OPENEV_DEVICES_DAVIS346_HPP

#include "openev/devices/davis.hpp"

namespace ev {
class Davis346 final : public Davis_ {
public:
  Davis346();
};

} // namespace ev

#endif // OPENEV_DEVICES_DAVIS346_HPP
