// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/config_direct_access.h to include/dbe/config_direct_access.hpp).

/************************************************************
 * direct_access.h
 *
 *  Created on: Feb 4, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/

#ifndef DBE_CONFIG_DIRECT_ACCESS_H_
#define DBE_CONFIG_DIRECT_ACCESS_H_

#include "dbe/dbcontroller.hpp"
#include "dbe/confobject_desc.hpp"

#include "conffwk/ConfigObject.hpp"

#include <string>

namespace dbe
{

namespace config
{

namespace api
{

/**
 * Direct access to Config layer through ConfigObjects
 */
class rwdacc
{
  /**
   * Retrieve objects of class matching a query string (regex)
   *
   * @param classname is the class to query for
   * @param query is a regular expression to match against the objects in the given class
   * @return  a list of configobjects
   */
  static std::vector<dunedaq::conffwk::ConfigObject> query_class ( std::string const & classname,
                                                 std::string const & query );

  /**
   * Creates the object in the database ( changes are not committed)
   *
   * @param fn is the file where the object is to be created in
   * @param cn is the name of the class that the object belongs to
   * @param name is the UID that should be given to the object
   * @param attributes are the attributes of the object to be created
   * @param relations are the relations for the object
   *
   * @return a copy of the thin wrapper ConfigObject
   */
  static dunedaq::conffwk::ConfigObject create_object ( std::string const & fn, std::string const & cn,
                                      std::string const & name );

  /**
   * Retrieve an object by class name and uid
   *
   * @param class name
   * @param object uid
   * @return a config object that is not null only if it exists in the database
   */
  static dunedaq::conffwk::ConfigObject get_object ( std::string const &, std::string const & );

  /**
   * Given a reference to an object set its attributes and relations
   *
   * @param newobj to set
   * @param attributes to set in the object
   * @param relations to set in the object
   * @return an empty tref in case of failure
   */
  static tref set_object ( tref newobj,
                           dbe::t_config_object_preimage::type_attrmap const & attributes,
                           dbe::t_config_object_preimage::type_relmap const & relations );


  /**
   * Rename an object in the database
   *
   * @param object reference to rename
   * @param newname as a string
   * @return a reference with the object
   */
  static void rename_object ( dunedaq::conffwk::ConfigObject & object, std::string const & newname );

  /**
   * Destroys the object in the online database ( changes are not committed)
   *
   * @param the object definition to create in the database
   */
  static void destroy_object ( dunedaq::conffwk::ConfigObject & );

  friend class inner::dbcontroller;
};

}  // namespace api

}  // namespace config

} // namespace dbe

#endif /* DBE_CONFIG_DIRECT_ACCESS_H_ */
