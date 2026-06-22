// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/// Including DBE
#include "dbe/MyApplication.hpp"
#include "dbe/Exceptions.hpp"
/// Including TDAq
#include "conffwk/Configuration.hpp"
#include "ers/ers.hpp"
#include "logging/Logging.hpp"

#include "dbe/messenger.hpp"
#include "dbe/version.hpp"

char const * const dbe_lib_core_version = dbe_compiled_version;

bool dbe::MyApplication::notify ( QObject * rec, QEvent * ev )
{
  HERE_AUTO_DEF ( notify );

  try
  {
    return QApplication::notify ( rec, ev );
  }
  catch ( dunedaq::conffwk::Exception const & err )
  {
    TLOG() << "MyApplication: dunedaq::conffwk::Exception: " << err ;
    ERROR ( "Unexpected error occurred", dbe::config::errors::unwind(err), "dunedaq::conffwk::Exception caught at", HERE );
    return false;
  }
  catch ( ers::Issue const & err )
  {
    TLOG() << "MyApplication: ers::Issue: " << err ;
    ERROR ( "ers::Issue occurred", dbe::config::errors::unwind(err), "\n\nCaught at:", HERE );
    return false;
  }
  catch ( std::exception const & err )
  {
    TLOG() << "MyApplication: std::exception: " ;
    ERROR ( "Error sending event", err.what(), "for object", typeid ( *ev ).name(), "Receiver",
            typeid ( *rec ).name() );
    return false;
  }
  catch ( char const * str )
  {
    TLOG() << "MyApplication: EXCEPTION: " << str << std::endl ;
    ERROR ( "Unknown exception", str, "\n\nCaught at: ", HERE );
    return false;
  }
  catch ( ... )
  {
    TLOG_DEBUG(0) <<  "MyApplication: Unknown exception!"  ;
    ERROR ( "Unknown exception", "\n\nCaught at: ", HERE );
    return false;
  }

  TLOG() << "MyApplication: outside catch..." ;
  return false;
}
