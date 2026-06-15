// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/model_common_operations.hpp to include/dbe/model_common_operations.hpp).

/*
 * model_common_operations.hpp
 *
 *  Created on: Dec 7, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_MODEL_COMMON_OPERATIONS_HPP_
#define DBE_MODEL_COMMON_OPERATIONS_HPP_

#include "dbe/model_common_interface.hpp"
#include "dbe/treenode.hpp"
#include "dbe/config_api_commands.hpp"

#include <QUuid>

#include <boost/scope_exit.hpp>

//------------------------------------------------------------------------------------------
/**
 * Needs be to called from the derived class constructor to setup the connection to slots defined in
 * the derived class, since they cannot be know before.
 */
template<typename T>
inline void dbe::model_common_async_operations<T>::model_common_connections()
{
  static_cast<T *> ( this )->connect (
    &confaccessor::ref(), SIGNAL ( object_deleted ( QString const &, dref const & ) ),
    static_cast<T *> ( this ), SLOT ( slot_remove_object ( QString const &, dref const & ) ) );
  static_cast<T *> ( this )->connect (
    &confaccessor::ref(), SIGNAL ( object_renamed ( QString const &, dref const & ) ),
    static_cast<T *> ( this ), SLOT ( slot_rename_object ( QString const &, dref const & ) ) );
  static_cast<T *> ( this )->connect (
    &confaccessor::ref(), SIGNAL ( object_changed ( QString const &, dref const & ) ),
    static_cast<T *> ( this ), SLOT ( slot_update_object ( QString const &, dref const & ) ) );
  static_cast<T *> ( this )->connect (
    &confaccessor::ref(), SIGNAL ( object_created ( QString const &, dref const & ) ),
    static_cast<T *> ( this ), SLOT ( slot_create_object ( QString const &, dref const & ) ) );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T>
inline void dbe::model_common_async_operations<T>::remove_object (
  type_object_info const & obj )
{
  type_index oid ( static_cast<T *> ( this )->lookup ( obj ) );

  if ( oid.isValid() )
  {
    static_cast<T *> ( this )->remove_deleted_object ( oid );
  }
}

template<typename T>
inline void dbe::model_common_async_operations<T>::rename_object (
  type_object_info const & obj )
{
  type_index oid ( static_cast<T *> ( this )->lookup ( obj ) );

  if ( oid.isValid() )
  {
    static_cast<T *> ( this )->rename_contained_object ( oid, obj );
  }
}

template<typename T>
inline void dbe::model_common_async_operations<T>::update_object (
  type_object_info const & obj )
{
  type_index oid ( static_cast<T *> ( this )->lookup ( obj ) );

  if ( oid.isValid() )
  {
    static_cast<T *> ( this )->update_contained_object ( oid, obj );
  }
}

template<typename T>
inline void dbe::model_common_async_operations<T>::update_multiple_objects (
  std::vector<type_object_info> const & objects )
{
    BOOST_SCOPE_EXIT_TPL(this_)
    {
        emit static_cast<T *> ( this_ )->layoutChanged();
    }
    BOOST_SCOPE_EXIT_END

    static_cast<T *> ( this )->layoutAboutToBeChanged();

    {
        BOOST_SCOPE_EXIT_TPL(this_)
        {
            static_cast<T *> ( this_ )->blockSignals(false);
        }
        BOOST_SCOPE_EXIT_END

        static_cast<T *> ( this )->blockSignals(true);

        for(const auto& obj : objects) {
            static_cast<T *> ( this )->update_object(obj);
        }
    }
}

template<typename T>
inline void dbe::model_common_async_operations<T>::create_object (
  type_object_info const & obj )
{
  type_index oid ( static_cast<T *> ( this )->lookup ( obj ) );

  if ( not oid.isValid() )
  {
    static_cast<T *> ( this )->create_contained_object ( oid, obj );
  }
}

template<typename T> inline
dbe::model_common_async_operations<T>::model_common_async_operations()
  : MODEL_COMMON_INTERFACE_UUID_DECL
{
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// FOR FUTURE USE
//template<typename T> inline std::vector<dbe::dref> dbe::model_common_impl<T>::filter_indices(
//    type_indices::iterator b, type_indices::iterator e)
//{
//  typedef std::set<decltype(b->row())> t_qrows;
//  typedef std::vector<dref> t_objects;
//
//  t_objects objects_to_act_upon;
//  t_indices indices_to_check;
//
//  for(t_indices::iterator iter = b ; iter != e ; ++iter)
//  {
//    if(iter->isValid() and iter->column() == 0)
//    {
//      dref o = static_cast<T *>(this)->getobject(*iter);
//      if(o.is_valid())
//      {
//        indices_to_check.push_back(*iter);
//        objects_to_act_upon.push_back(o);
//      }
//    }
//  }
//
//}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> inline bool dbe::model_common_impl<T>::delete_objects (
  type_indices::iterator b, type_indices::iterator e )
{
  typedef std::set < decltype ( b->row() ) > t_qrows;
  typedef std::vector<type_object_info> t_objects;
  t_objects to_remove_objects;

  for ( type_indices::iterator iter = b; iter != e; ++iter )
  {
    if ( iter->isValid() and iter->column() == 0 )
    {
      dref o = static_cast<T *> ( this )->getobject ( *iter );

      if ( o.is_valid() )
      {
        to_remove_objects.push_back ( o );
      }
    }
  }

  t_objects removed_objects;

  for ( auto const & o : to_remove_objects )
  {
    if ( dbe::config::api::commands::delobj ( o.ref(), static_cast<T *> ( this )->uuid ) )
    {
      removed_objects.push_back ( o );
    }
  }

  t_qrows to_remove_rows;

  for ( auto const & o : removed_objects )
  {
    QModelIndex newloc = static_cast<T *> ( this )->lookup ( o );

    if ( newloc.isValid() )
    {
      to_remove_rows.insert ( newloc.row() );
    }
  }

  t_qrows::reverse_iterator uend = to_remove_rows.rbegin();

  if ( uend != to_remove_rows.rend() )
  {
    for ( int count = 0, bottom = *uend, last = bottom; uend != to_remove_rows.rend();
          count = 0, bottom = *uend )
    {
      for ( ; uend != to_remove_rows.rend() and count == bottom - *uend;
            last = *uend, ++uend, ++count )
        ;

      static_cast<T *> ( this )->removeRows ( last, count, b->parent() );
    }

    return true;
  }

  return false;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> inline bool dbe::model_common_impl<T>::update_objects (
  type_indices::iterator b, type_indices::iterator e )
{
  Q_UNUSED ( b );
  Q_UNUSED ( e );
  return false;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> inline bool dbe::model_common_impl<T>::create_objects (
  type_indices::iterator b, type_indices::iterator e )
{
  Q_UNUSED ( b );
  Q_UNUSED ( e );
  return false;
}
//------------------------------------------------------------------------------------------

#endif /* DBE_MODEL_COMMON_OPERATIONS_HPP_ */
