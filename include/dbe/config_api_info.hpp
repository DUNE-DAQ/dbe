// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/config_api_info.h to include/dbe/config_api_info.hpp).

/*
 * config_api_info.h
 *
 *  Created on: Apr 20, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFIG_API_INFO_H_
#define DBE_CONFIG_API_INFO_H_

#include "dbe/config_reference.hpp"
#include "conffwk/Schema.hpp"

#include <QString>
#include <string>
#include <vector>

namespace dunedaq
{
namespace config
{
struct attribute_t;
struct class_t;
struct relationship_t;
} /* namespace config */
} /* namespace daq */

//------------------------------------------------------------------------------------------
namespace dbe
{
namespace config
{
namespace api
{
namespace info
{
//------------------------------------------------------------------------------------------
//                                                 INFO NAMESPACE
//------------------------------------------------------------------------------------------
bool has_obj ( std::string const & classname, std::string const & object_uid );

/**
 * Retrieve attribute information for a given attribute name of a class
 * @param Attribute name
 * @param Class name
 * @return the attribute information
 */
 dunedaq::conffwk::attribute_t attributematch ( QString const &, QString const & );

namespace relation
{
/**
 * Retrieve relation information for a given relation name of a class
 *
 * @param Relation name
 * @param Class name
 * @return  the relation information
 */
template<typename T> dunedaq::conffwk::relationship_t match ( T const & , T const &);

template<>
dunedaq::conffwk::relationship_t match<QString>( QString const &, QString const & );
template<>
dunedaq::conffwk::relationship_t match<std::string>( std::string const &, std::string const & );

/**
 * Returns true if the underlying relation takes values only possible for a simple edge
 *
 * @param a dunedaq::conffwk::relationship_t to evaluate
 * @return true in case it is a simple edge
 */
bool is_simple ( dunedaq::conffwk::relationship_t const & );
}

class onclass
{
public:
  /**
   * Get a list of all classes defined in the database
   *
   * @return
   */
  template<typename T> static inline T allnames();
  /**
   * Get class information from class name
   * @param cn is the class name to retrieve information for
   * @param determines if class information should include direct-only relation,
   *        attribute , sub-class and superclass information
   * @return a class information object
   */
  static dunedaq::conffwk::class_t definition ( std::string const & cn, bool direct_only );

  /**
   * Retrieve references to config objects of a given class
   *
   * @param cname is the class name to retrieve objects for
   * @param keep_inherited if set to false filters out any objects inherited from this class
   * @return a vector of references
   */
  template<bool SORTED = true>
  static std::vector<dbe::inner::configobject::tref>
  objects ( std::string const & cname, bool const keep_inherited = true );

  /**
   * Determine if a class is derived from another
   * @param fromclass is the classname of the class the we want to see if a class is derived from
   * @param aclass is the classname of the candidate class to be derived from inclass
   * @return true if aclass is derived fromclass
   */
  static bool derived ( std::string const & fromclass, std::string const & aclass );
};

//------------------------------------------------------------------------------------------
}// end namespace info
}
}
}
//------------------------------------------------------------------------------------------

#endif /* DBE_CONFIG_API_INFO_H_ */
