// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/model_common_interface.h to include/dbe/model_common_interface.hpp).

/*
 * model_common_interface.h
 *
 *  Created on: Nov 16, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_MODEL_COMMON_INTERFACE_H_
#define DBE_MODEL_COMMON_INTERFACE_H_

#include "dbe/confaccessor.hpp"
#include "conffwk/ConfigObject.hpp"

#include <QString>
#include <QModelIndex>
#include <QUuid>

#include <vector>
#include <set>

#define MODEL_COMMON_INTERFACE_UUID_DEF QUuid const uuid;
#define MODEL_COMMON_INTERFACE_UUID_DECL uuid(QUuid::createUuid())

#define MODEL_COMMON_TYPES_TOPLEVEL_DEFINITIONS\
    typedef QModelIndex type_index;\
    typedef std::vector<type_index> type_indices;\
    typedef dbe::tref type_object_ref;\
    typedef dbe::dref type_object_info;\
    typedef dunedaq::conffwk::class_t type_class_info;\
    \


namespace dbe
{
//------------------------------------------------------------------------------------------

class model_common_slots
{
  MODEL_COMMON_TYPES_TOPLEVEL_DEFINITIONS

public:
  virtual ~model_common_slots() = default;

private:
  // These methods should be placed in slots declaration such that they can be parsed
  // appropriately to create slots at each derived class
  /**
   * Slot to notify that an object has been removed from another class
   *
   * This must be overriden in "private slots:" of the derived class
   *
   * @param A string describing a UUID of the class that initiated the deletion
   * @param A descriptor of the config object that has been deleted
   */
  virtual void slot_remove_object ( QString const &, dref const & ) = 0;

  /**
   * Slot to notify that an object has been renamed from another class
   *
   * This must be overriden in "private slots" of the derived class
   *
   * @param A string description of the UUID of the class that initiated this call
   * @param A descripto of the config object that has been renamed, the new name
   *              can be retrieved by dereferencing the ConfigObject and retrieving the new name
   */
  virtual void slot_rename_object ( QString const &, dref const & ) = 0;

  /**
   * Slot to notify that an object has been updated from another class
   *
   * This must be overriden in "private slots" of the derived class
   *
   * @param A string description of the UUID of the class that initiated this call
   * @param A descriptor of the ConfigObject that has been renamed, the new values
   *              can be retrieved by dereferencing the ConfigObject and retrieving them directly
   */
  virtual void slot_update_object ( QString const &, dref const & ) = 0;

  /**
   * Slot to notify that an object has been created from another class
   *
   * This must be overriden in "private slots" of the derived class
   *
   * @param A string description of the UUID of the class that initiated this call
   * @param A descriptor of the ConfigObject that has been renamed, the new values
   *              can be retrieved by dereferencing the ConfigObject and retrieving them directly
   */
  virtual void slot_create_object ( QString const &, dref const & ) = 0;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
struct model_common_data_access_interface
{
  MODEL_COMMON_TYPES_TOPLEVEL_DEFINITIONS
public:
  virtual type_object_ref getobject ( type_index const & index ) const = 0;

  virtual type_class_info getclass ( type_index const & index ) const = 0;

  virtual QAbstractItemModel * ReturnSourceModel() const = 0;

  virtual ~model_common_data_access_interface() = default;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class model_initiate_actions_interface
{
  MODEL_COMMON_TYPES_TOPLEVEL_DEFINITIONS

public:
  virtual ~model_initiate_actions_interface() = default;
  //-------------------------- FORWARD CHAIN ------------------------------------------------------------------------
  // Front end functionality permits explicit actions on the objects linked to this model
  // These methods should provoke actions onto the database itself
  /**
   * Entry point from a view knowing this model to force deletion of object from the database
   *
   * @param iterator to the beginning of container with model indices for removal
   * @param iterator to the end of container with model indices for removal
   */
  virtual bool delete_objects ( type_indices::iterator, type_indices::iterator ) = 0;
  virtual bool update_objects ( type_indices::iterator, type_indices::iterator ) = 0;
  virtual bool create_objects ( type_indices::iterator, type_indices::iterator ) = 0;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Abstract class of the interface shared across table and tree models
 */
class model_common_interface:
  public model_initiate_actions_interface,
  public model_common_data_access_interface,
  private model_common_slots
{
  MODEL_COMMON_TYPES_TOPLEVEL_DEFINITIONS

  friend class model_common_slots;
public:
  virtual ~model_common_interface() = default;

protected:
  /**
   * Remove model information related to an object deleted from the database
   *
   * @param is the index of the object that needs to be removed
   */
  virtual void remove_deleted_object ( type_index const & index ) = 0;

  /**
   * Cause model information related to a specific object to be updated after an object
   * has been renamed in the database .
   *
   * @param index is the current index of the object to be renamed
   * @param obj is the object information reference which permits to retrieve the new name
   */
  virtual void rename_contained_object ( type_index const & index,
                                         type_object_info const & obj ) = 0;

  /**
   * Cause model to update information about an object in the given index
   *
   * @param index is the current index of the object in the model
   * @param obj is the information reference to the object that contains the
   * updated information
   */
  virtual void update_contained_object ( type_index const & index,
                                         type_object_info const & obj ) = 0;
  /**
   * Cause model to create information about a new object that should be included in this
   * object
   *
   * @param index where the new object should be included
   * @param obj is the information reference to the new object
   */
  virtual void create_contained_object ( type_index const & index,
                                         type_object_info const & obj ) = 0;

  /**
   * Lookup an object and retrieve its matching model index
   *
   * @param the object description to be removed
   * @return the model index linked to a matching configobject
   */
  virtual type_index lookup ( type_object_info const & ) = 0;

  QUuid getuuid() const;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * This class purpose is to provide a unified interface for performing actions on the database
 *
 * It can be called directly from views linked to objects. In this current model the controller
 * is actually implemented within the model. This is ok since we do not expects massive
 * amount of user inputs that would need be handled in an asynchronous manner, and would
 * require a controller to be linked to this model-view type interface.
 */
template<typename T> class model_common_impl:
  public model_common_interface
{
  MODEL_COMMON_TYPES_TOPLEVEL_DEFINITIONS

public:

  /**
   * Given a range of indices issue commands to delete the related objects
   *
   * @param start iterator of the range
   * @param end iterator of the range
   * @return false in case of error
   */
  bool delete_objects ( type_indices::iterator, type_indices::iterator ) override;

  /**
   *
   * @param
   * @param
   * @return
   */
  bool update_objects ( type_indices::iterator, type_indices::iterator ) override;

  /**
   * Create a range of objects
   * @param
   * @param
   * @return
   */
  bool create_objects ( type_indices::iterator, type_indices::iterator ) override;

//  private:
//  FOR FUTURE USE
//    static std::vector<type_object_info> filter_indices(type_indices::iterator,
//                                                        type_indices::iterator);
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Provides a unified manner to handle implemented methods in derived classes of
 * model_common_slots::slot_... methods (e.g. defined with MODEL_COMMON_INTERFACE_SLOTS... macros )
 * by first looking up for the object at hand.
 *
 * Each of the methods in this class are called from the derived class causing a call chain
 * Derived::slot -> model_common_async_operations method -> implemented method in Derived class
 *
 * e.g. for object deletion
 *
 * T::slot_remove_object -> remove_object -> T::remove_deleted_object
 */
template<typename T> class model_common_async_operations
{
public:
  MODEL_COMMON_TYPES_TOPLEVEL_DEFINITIONS

  model_common_async_operations();

  /**
   * Setup the necessary connections to signals from confaccessor to the derived object
   *
   * THIS HAS TO BE CALLED FROM THE DERIVED CLASS otherwise it will have no effect
   */
  void model_common_connections();

protected:
  //-------------------------- BACK CHAIN -------------------------------------------------------------------------------
  // Back end functionality that permits to implicitly act on an object  referenced  in this model
  // These methods should NOT take action onto the database but only on the referenced model

  /**
   * Actually calls the derived class implementation of the prototype remove_deleted_object
   *
   * @param the object to be matched and removed from the index
   */
  void remove_object  ( type_object_info const & );
  void create_object  ( type_object_info const & );
  void rename_object  ( type_object_info const & );
  void update_object  ( type_object_info const & );
  void update_multiple_objects ( std::vector<type_object_info> const & );

  MODEL_COMMON_INTERFACE_UUID_DEF

};
//------------------------------------------------------------------------------------------

}// namespace dbe

#include "dbe/model_common_interface_macros.hpp"
#include "dbe/model_common_operations.hpp"

#endif /* DBE_MODEL_COMMON_INTERFACE_H_ */
