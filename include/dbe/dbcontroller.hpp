/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/dbcontroller.h to include/dbe/dbcontroller.hpp).
 */

#ifndef DBE_DBCONTROLLER_H_
#define DBE_DBCONTROLLER_H_

#include "dbe/tref.hpp"

#include "dbe/confobject_desc.hpp"
#include "dbe/confobject_extra.hpp"
#include "dbe/config_reference.hpp"

#include "dbe/config_direct_access.hpp"

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <mutex>
#include <unordered_map>

namespace dbe
{
namespace inner
{
//------------------------------------------------------------------------------------------
class dbcontroller
{
public:

  /**
   * Retrieve an object from the database, and empty reference is retrieved if the object
   * cannot be found in the database
   *
   * @param desc is a description of the object to be searched for in the database
   * @return a transaction reference to the object in the database
   */
  static configobject::tref get ( dbe::cokey const & desc );

  /**
   * Retrieve all objects from the database of a specific class
   *
   * @param cname is the class name for which to retrieve classes
   * @return a vector of references with the objects
   */
  static std::vector<configobject::tref> gets ( std::string const & cname,
                                                std::string const & query = "" );

  /**
   * Create a new configuration object from its representation
   *
   * @param rep is the representation, with attributes and relations included
   * @return
   */
  static configobject::tref create_object_request (
    dbe::t_config_object_preimage const & rep );

  /**
   * creates an object to the database that is described from an aref
   *
   * @param obj is an aref of an object, based on which the object is put onto the database
   * @return return a tref to the resulting object
   */
  template<typename T> static configobject::tref create_object_request (
    configobject::aref<T> const & obj );

  /**
   * Create an object in the database by first creating its associated subgraph as described
   * in a gref
   *
   * @param objref is the graph reference (gref) to the object
   * @return a transaction reference to a tref
   */
  template<typename T> static configobject::tref create_object_request (
    configobject::gref<T> const & objref );

  template<typename T> static configobject::tref create_object_request (
    configobject::gref<T> & objref,
    typename configobject::gref<T>::config_action_notifiable notice );

  /**
   * removes object from the database and returns an aref of the object before copying
   * @param obj is a tref to the object that must be removed from the database
   */
  template<typename T> static configobject::gref<T> delete_object_request (
    configobject::tref const & obj );

  template<typename T> static configobject::gref<T> delete_object_request (
    configobject::tref const & obj,
    typename configobject::gref<T>::config_action_notifiable notice );

  /**
   * Move an object to a new database file
   *
   * @param objref is the tref of the associated ConfigObject to be moved to a new database file
   * @param destfile is the destination file where this object must be moved to
   * @return a tref pointing to the resulting object, in case that there has been any type of relocation
   *              inside this is going to be different from the original. If there has been no relocation, i.e.
   *              in case of failure a null reference is returned
   */
  static configobject::tref move_object_request ( configobject::tref objref,
                                                  std::string const & destfile );

  /**
   * Rename an object in the database
   *
   * @param objref is a tref to the current object
   * @param newname is the new name of the object
   * @return a tref pointing to the resulting object. In case of failure this results in
   *                returning a null reference. In rare cases there may be relocations that
   *                will change the reference.
   */
  static configobject::tref rename_object_request ( configobject::tref objref,
                                                    std::string const & newname );

  /**
   * Empties the internal cache and sets all associated objects to destroyed
   * This results in all previously held references to be set to null
   */
  static void flush();

private:
  typedef dbe::cokey t_object_key;
  typedef std::shared_ptr<configobject::oref> t_object_handle;
  typedef std::unordered_map<t_object_key, t_object_handle, configobject::refhasher>
  t_object_map;

  /**
   * Obtain a reference to the dbcontroller
   * @return
   */
  static dbcontroller & ref();

  /**
   * Insert or replace a ConfigObject into the internal store
   *
   * @param the configuration object to insert, actually makes a copy of it
   * @return a tref to the inserted copy of the object
   */
  configobject::tref insert ( dunedaq::conffwk::ConfigObject const & );

  /**
   * Searches for an object matching the criteria in the underlying database connection and
   * then updates the internal map by comparing if it should be removed. All references to
   * the object are transparently updated. If this cannot be done then they are replaced
   *
   * @param the configuration object to lookup for in the cache and the database
   * @return the tref that points to that object in case of success , a null tref in case of failure
   */
  configobject::tref lookup ( dbe::cokey const & desc );

  /**
   * Rename an object and update the internal cache
   * @param objref
   * @param newname
   * @return
   */
  configobject::tref rename ( configobject::tref objref, std::string const & aname );

  /**
   * Takes care of removing an object from the underlying implementation and setting all
   * held references to it null . Performs check for derived objects having be removed.
   *
   * @param the configobject to remove from the underlying database
   * @return returns a list of objects that have been removed from the database
   *                these are in a form that permits to recreate the deleted objects
   */
  template<typename T> configobject::gref<T> remove ( dbe::tref ref );

  /**
   * Return a list of references to the objects in the database that reference and object
   *
   * @param objref the tref to the object for which references are to be retrieved
   * @param name the name of the relation for which references are to be retrieved
   * @param check_composite_only return only composite connections
   * @return
   */
  std::vector<configobject::tref> referenced_by ( configobject::tref objref,
                                                  std::string const & name = "*",
                                                  bool check_composite_only = true );

  t_object_map this_allobjects;

  typedef std::recursive_mutex t_mutex;
  typedef std::lock_guard<t_mutex> locker;
  static t_mutex this_lock;

  dbcontroller();
  dbcontroller ( dbcontroller const & ) = delete;
  dbcontroller & operator= ( dbcontroller const & ) = delete;

  friend class configobject::tref;
};

}  // namespace inner

} // end namespace dbe
//------------------------------------------------------------------------------------------

#endif /* DBE_DBCONTROLLER_H_ */
