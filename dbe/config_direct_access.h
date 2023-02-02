/************************************************************
 * direct_access.h
 *
 *  Created on: Feb 4, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/

#ifndef DBE_CONFIG_DIRECT_ACCESS_H_
#define DBE_CONFIG_DIRECT_ACCESS_H_

#include "dbcontroller.h"
#include "confobject_desc.hpp"

#include "config/ConfigObject.hpp"

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
  static std::vector<ConfigObject> query_class ( std::string const & classname,
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
  static ConfigObject create_object ( std::string const & fn, std::string const & cn,
                                      std::string const & name );

  /**
   * Retrieve an object by class name and uid
   *
   * @param class name
   * @param object uid
   * @return a config object that is not null only if it exists in the database
   */
  static ConfigObject get_object ( std::string const &, std::string const & );

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
  static void rename_object ( ConfigObject & object, std::string const & newname );

  /**
   * Destroys the object in the online database ( changes are not committed)
   *
   * @param the object definition to create in the database
   */
  static void destroy_object ( ConfigObject & );

  friend class inner::dbcontroller;
};

}  // namespace api

}  // namespace config

} // namespace dbe

#endif /* DBE_CONFIG_DIRECT_ACCESS_H_ */
