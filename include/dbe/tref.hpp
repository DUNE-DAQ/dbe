/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/tref.h to include/dbe/tref.hpp).
 */

/*
 * tref.h
 *
 *  Created on: 15 Jun 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_TREF_H_
#define DBE_TREF_H_

#include <string>

namespace dbe
{

template<typename> struct config_object_linked;
template<typename> struct config_object_aggregates;

namespace inner
{
namespace configobject
{
class tref;
class vref;
template<typename> class aref;
template<typename> class gref;
}
}

typedef inner::configobject::tref tref;
typedef inner::configobject::vref vref;
typedef inner::configobject::aref<config_object_linked<std::string>> aref;
typedef inner::configobject::gref<config_object_aggregates<std::string>> gref;

}  // namespace dbe

#endif /* DBE_TREF_H_ */
