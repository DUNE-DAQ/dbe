// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/*
 * config_api.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/Exceptions.hpp"
#include "dbe/version.hpp"
#include "dbe/config_api_version.hpp"

#include "conffwk/Errors.hpp"
#include "ers/Issue.hpp"
#include <algorithm>
#include <sstream>
#include <string>

char const * const dbe_lib_config_api_version = dbe_compiled_version;

//------------------------------------------------------------------------------------------
//                                      DBE::CONFIG::ERRORS NAMESPACE
//------------------------------------------------------------------------------------------
namespace dbe
{
namespace config
{
namespace errors
{


/**
 * Unwind all causes linked to this exception
 *
 * @param ex the exception to process
 * @return a string
 */
std::string const unwind ( ers::Issue const & exception )
{

  if ( ers::Issue const * cause = exception.cause() )
  {
    std::stringstream s;

    while ( cause != nullptr )
    {
      s << cause->what() << "\n";
      cause = cause->cause();
    }

    return s.str();
  }
  else
  {
    return topcause ( exception );
  }

}

/**
 * Provide a string with the reason given by the exception
 *
 * @param ex is the exception to process
 * @return a string
 */
std::string const reason ( ers::Issue const & exception )
{
  return exception.what();
}

/**
 * Provide a string with the top level cause in this exception
 *
 * @param ex is the exception to process
 * @return a string
 */
std::string const topcause ( ers::Issue const & exception )
{
  if ( ers::Issue const * cause = exception.cause() )
  {
    return cause->what();
  }
  else
  {
    return exception.what();
  }
}

/**
 * Dump the exception caught at full detail level as specified by the environment
 * @param ex is the exception to process
 * @return a string
 */
std::string const dump ( ers::Issue const & exception )
{
  std::stringstream s;
  s << exception;
  return s.str();
}

std::string const parse ( ers::Issue const & exception )
{
  return unwind ( exception );
}

//------------------------------------------------------------------------------------------
}
}
}
//------------------------------------------------------------------------------------------

