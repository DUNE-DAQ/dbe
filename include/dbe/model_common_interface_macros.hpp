// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/model_common_interface_macros.h to include/dbe/model_common_interface_macros.hpp).

/*
 * model_common_interface_macros.h
 *
 *  Created on: Dec 7, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_MODEL_COMMON_INTERFACE_MACROS_H_
#define DBE_MODEL_COMMON_INTERFACE_MACROS_H_

#include "dbe/model_common_interface.hpp"

//------------------------------------------------------------------------------------------
#define MODEL_COMMON_INTERFACE_TYPES(classname)\
  \
  typedef model_common_async_operations<classname>::type_index type_index;\
  typedef model_common_async_operations<classname>::type_indices type_indices;\
  typedef model_common_async_operations<classname>::type_object_ref type_object_ref;\
  typedef model_common_async_operations<classname>::type_class_info type_class_info;\
  typedef model_common_async_operations<classname>::type_object_info type_object_info;\
  \
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#define MODEL_COMMON_IMPL_REQ_DEF(classname) \
  \
  friend class model_common_async_operations<classname>;\
  friend class model_common_impl<classname>;\
    \
  public:\
  QModelIndex lookup(dbe::dref const &) override; \
  \
  MODEL_COMMON_INTERFACE_TYPES(classname)\
  \
  type_object_ref getobject(type_index const &index) const override; \
  type_class_info getclass(const QModelIndex & index) const override; \
  QAbstractItemModel* ReturnSourceModel() const override; \
  \
  private:\
  \
  void remove_deleted_object(type_index const & index) override; \
  void create_contained_object(type_index const & index, \
                               type_object_info const & obj) override; \
  void rename_contained_object(type_index const & index, \
                               type_object_info const & obj) override; \
  void update_contained_object(type_index const & index, \
                               type_object_info const & obj) override; \
  \
  bool removeRows(int row, int count, type_index const & parent) override;\
  \
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#define MODEL_COMMON_INTERFACE_REQ_DEF(classname) \
  \
  MODEL_COMMON_IMPL_REQ_DEF(classname) \
  \
  public:\
  \
  bool delete_objects(type_indices::iterator , type_indices::iterator ) override; \
  bool create_objects(type_indices::iterator , type_indices::iterator ) override; \
  bool update_objects(type_indices::iterator , type_indices::iterator ) override; \
  \
  private:\
  \

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#define MODEL_REMOVE_ROWS_DEF(classname) \
    bool classname::removeRows(int row, int count, type_index const & parent)
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#define MODEL_DELETE_OBJECTS_DEF(classname) \
    bool classname::delete_objects(type_indices::iterator b , type_indices::iterator e) \

#define MODEL_CREATE_OBJECTS_DEF(classname) \
    bool classname::create_objects(type_indices::iterator b, type_indices::iterator e) \

#define MODEL_UPDATE_OBJECTS_DEF(classname) \
    bool classname::update_objects(type_indices::iterator b, type_indices::iterator e) \

#define MODEL_RENAME_OBJECTS_DEF(classname) \
    bool classname::rename_objects(type_indices::iterator b, type_indices::iterator e) \
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/*
 * Below are macros that facilitate to define the methods required by model interface
 */

/**
 * Macro to define the method in derived models that actually
 * does the work of searching from an object for the index of
 * artifacts related to an object from a model
 */
#define MODEL_COMMON_INTERFACE_LOOKUP_IMPL(classname) \
    QModelIndex classname::lookup(type_object_info const & obj) \

/**
 * Macro to define the method in derived models that actually
 * does the work of removing artifacts related to an object from a model
 */
#define MODEL_COMMON_INTERFACE_DELETE_THAT_OBJ_IMPL(classname) \
    void classname::remove_deleted_object(type_index const & index) \

/**
 * Macro to define the method in derived models that actually
 * does the work of creating artifacts related to an object from a model
 */
#define MODEL_COMMON_INTERFACE_CREATE_THAT_OBJ_IMPL(classname) \
    void classname::create_contained_object(type_index const & index, \
                                             type_object_info const & obj) \

/**
 * Macro to define the method in derived models that actually
 * does the work of removing artifacts related to an object from a model
 */
#define MODEL_COMMON_INTERFACE_RENAME_THAT_OBJ_IMPL(classname) \
    void classname::rename_contained_object(type_index const & index, \
                                             type_object_info const & obj) \

/**
 * Macro to define the method in derived models that actually
 * does the work of updating artifacts related to an object from a model
 */
#define MODEL_COMMON_INTERFACE_UPDATE_THAT_OBJ_IMPL(classname) \
    void classname::update_contained_object(type_index const & index, \
                                            type_object_info const & obj) \

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// INCLUDE this macro in the derived class source file
//
// Slots need be defined in the header file of the derived class directly for Qt
// to be aware and generate the code needed , this forces the definition in the source file
// of the derived class for the slots, which can be uniformly achieved with this macro.
#define MODEL_COMMON_INTERFACE_SLOTS_DEF(classname) \
  void classname::slot_remove_object(QString const & src, type_object_info const & obj)\
  {\
    if(uuid.toString() != src) \
    { \
    remove_object(obj); \
    }\
  }\
  \
  void classname::slot_update_object(QString const & src, type_object_info const & obj)\
  {\
    if(uuid.toString() != src) \
    { \
    update_object(obj); \
    }\
  }\
  \
  void classname::slot_rename_object(QString const & src, type_object_info const & obj)\
  {\
    if(uuid.toString() != src) \
    { \
    rename_object(obj); \
    }\
  }\
  \
  void classname::slot_create_object(QString const & src, type_object_info const & obj)\
  {\
    if(uuid.toString() != src) \
    { \
    create_object(obj); \
    }\
  }\

//------------------------------------------------------------------------------------------

#endif /* DBE_MODEL_COMMON_INTERFACE_MACROS_H_ */
