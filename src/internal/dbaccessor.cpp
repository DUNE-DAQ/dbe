/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/*
 * dbaccessor.cpp
 *
 *  Created on: Mar 10, 2016
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/dbaccessor.hpp"

using namespace dunedaq::conffwk;

//------------------------------------------------------------------------------------------
dbe::dbholder::t_mutex dbe::dbholder::database_lock;
Configuration * dbe::dbholder::database = nullptr;
cptr<Configuration> dbe::dbholder::database_concurrent_ptr = cptr<Configuration> (
                                                               dbe::dbholder::database );
//------------------------------------------------------------------------------------------
cptr<Configuration> dbe::dbaccessor::dbptr()
{
  t_lock l ( dbholder::database_lock );
  return dbholder::database_concurrent_ptr;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool dbe::dbaccessor::is_loaded()
{
return dbaccessor::dbptr().get() != nullptr;
}
//------------------------------------------------------------------------------------------
