/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/Exceptions.h to include/dbe/Exceptions.hpp).
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

/// Including ers headers
#include "ers/ers.hpp"

#include "conffwk/Errors.hpp"

#include <string>

namespace daq
{

ERS_DECLARE_ISSUE ( dbe, Exception, , )

ERS_DECLARE_ISSUE_BASE ( dbe, ObjectChangeWasNotSuccessful, Exception,
                         "Object was not changed.", , )

ERS_DECLARE_ISSUE_BASE (
  dbe, ChangeNotAllowed, Exception,
  "Only Configuration DB opened by rdbconfig or oksconflibs plug-in can be changed.", , )

ERS_DECLARE_ISSUE_BASE ( dbe, CouldNotCommitChanges, Exception,
                         "Changes could not be committed, no object was changed.", , )

ERS_DECLARE_ISSUE_BASE ( dbe, BadConversion, Exception, message, ,
                         ( ( const char * ) message ) )

ERS_DECLARE_ISSUE_BASE ( dbe, DatabaseChangeNotSuccessful, Exception,
                         "Database change was not successful, no object was not changed.", , )

ERS_DECLARE_ISSUE_BASE (
  dbe, ConfigObjectInvalidReference, Exception,
  "Internal DBE references cannot point to null ConfigObject references", , )

ERS_DECLARE_ISSUE_BASE ( dbe, cannot_handle_invalid_qmodelindex, Exception,
                         "An invalid qmodel index has been encountered", , )

ERS_DECLARE_ISSUE_BASE (
  dbe, gref_empty_internal_queue_is_invalid_state, Exception,
  "A gref with an empty internal queue has been encountered which should not be possible",
  , )

ERS_DECLARE_ISSUE_BASE (
  dbe, config_object_retrieval_result_is_null, Exception,
  "Config layer returned null reference for object: " << oname , ,
  ( ( std::string ) oname ) )

ERS_DECLARE_ISSUE_BASE ( dbe, dbcontroller_internal_cache_failure, Exception,
                         "A insertion to the internal map failed", , )

ERS_DECLARE_ISSUE_BASE ( dbe, null_config_reference_access, Exception,
                         "Attempt to access a ConfigObject through a null reference", , )

} /* end namespace daq */

namespace dbe
{
namespace config
{
namespace errors
{

/**
 * Parses the error in the exception to a string
 */
std::string const parse ( ers::Issue const & );
/**
 * Unwind the causes in an exception given
 */
std::string const unwind ( ers::Issue const & );
/**
 * Retrieve the reason in an exception given
 */
std::string const reason ( ers::Issue const & );
/**
 * Retrieve the originating / top level cause
 */
std::string const topcause ( ers::Issue const & );
/**
 * Provide all details as specified by TDAQ reporting verbosity
 */
std::string const dump ( ers::Issue const & );

}
}
} /* end namespace dbe */
#endif // EXCEPTIONS_H
