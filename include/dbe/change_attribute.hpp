// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/change_attribute.hpp to include/dbe/change_attribute.hpp).

/*
 * change_attribute.hpp
 *
 *  Created on: Nov 10, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_CHANGE_ATTRIBUTE_HPP_
#define DBE_CHANGE_ATTRIBUTE_HPP_

#include "dbe/confaccessor.hpp"
#include "dbe/config_api_set.hpp"
#include "dbe/config_api_get.hpp"
#include "dbe/Command.hpp"
#include "dbe/Exceptions.hpp"

#include "conffwk/ConfigObject.hpp"
#include "conffwk/Errors.hpp"
#include "conffwk/Schema.hpp"

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <string>

#include  "dbe/messenger.hpp"

namespace dbe
{
namespace actions
{

template<typename T>
class ChangeAttribute:
  public onobject
{
public:
  ChangeAttribute ( tref Object, dunedaq::conffwk::attribute_t AttributeData, T NewValueData,
                    QUndoCommand * parent = nullptr );
  void undo();
  void redo();
private:
  T OldValue;
  T NewValue;
  dunedaq::conffwk::attribute_t Attribute;
};

template<typename T>
ChangeAttribute<T>::ChangeAttribute ( tref Object, dunedaq::conffwk::attribute_t AttributeData,
                                      T NewValueData, QUndoCommand * parent )
  : onobject ( Object, parent ),
    NewValue ( NewValueData ),
    Attribute ( AttributeData )
{
  try
  {
    failed();
    QStringList Data
    { dbe::config::api::get::attribute::list<QStringList> ( this->checkedref(), Attribute ) };
    OldValue = convert::to<T> ( Data, Attribute.p_int_format );
    toggle();
  }
  catch ( dunedaq::conffwk::Exception const & )
  {
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           ex.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, ex );
  }

  setText (
    QObject::tr ( "Attribute %1 of object %2 was updated." ).arg (
      AttributeData.p_name.c_str() ).arg ( Object.UID().c_str() ) );
}

template<typename T>
void ChangeAttribute<T>::undo()
{
  try
  {
    if ( undoable() )
    {
      failed();
      dbe::config::api::set::noactions::attribute ( this->checkedref(), Attribute, OldValue );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    failed();
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, e );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           ex.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, ex );
  }

}

template<typename T>
void ChangeAttribute<T>::redo()
{
  try
  {
    if ( redoable() )
    {
      failed();

      QStringList Data
      { dbe::config::api::get::attribute::list<QStringList> ( this->checkedref(), Attribute ) };

      OldValue = convert::to<T> ( Data, Attribute.p_int_format );

      dbe::config::api::set::noactions::attribute ( this->checkedref(), Attribute, NewValue );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    failed();
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, e );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           ex.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, ex );
  }

}

}
}
#endif /* DBE_CHANGE_ATTRIBUTE_HPP_ */
