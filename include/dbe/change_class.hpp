/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/change_class.hpp to include/dbe/change_class.hpp).
 */

/*
 * change_class.hpp
 *
 *  Created on: Nov 10, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_CHANGE_CLASS_HPP_
#define DBE_CHANGE_CLASS_HPP_

#include "dbe/confaccessor.hpp"
#include "dbe/config_api_set.hpp"
#include "dbe/Command.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/messenger.hpp"

#include "conffwk/ConfigObject.hpp"
#include "conffwk/Errors.hpp"
#include "conffwk/Schema.hpp"

#include <QObject>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <string>

namespace dbe
{
namespace actions
{
//------------------------------------------------------------------------------------------
template<typename T>
class ChangeClass:
  public onobject
{
public:
  ChangeClass ( tref Object, dunedaq::conffwk::attribute_t AttributeData, T NewValueData,
                QUndoCommand * parent = nullptr );
  void undo();
  void redo();

private:
  T OldValue;
  T NewValue;
  dunedaq::conffwk::attribute_t Attribute;
  bool Success;
};

template<typename T>
ChangeClass<T>::ChangeClass ( tref Object, dunedaq::conffwk::attribute_t AttributeData,
                              T NewValueData, QUndoCommand * parent )
  : onobject ( Object, parent ),
    NewValue ( NewValueData ),
    Attribute ( AttributeData ),
    Success ( true )
{
  try
  {
    QStringList Data
    { dbe::config::api::get::attribute::list<QStringList> ( Object, Attribute ) };
    OldValue = convert::to<T> ( Data );
  }
  catch ( dunedaq::conffwk::Exception const & )
  {
  }

  setText (
    QObject::tr ( "Attribute %1 of object %2 was updated." ).arg (
      AttributeData.p_name.c_str() ).arg ( Object.UID().c_str() ) );
}

template<typename T>
void ChangeClass<T>::undo()
{
  try
  {
    if ( isvalid() )
    {
      failed();
      dbe::config::api::set::noactions::aclass ( this->checkedref(), Attribute, OldValue );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    Success = false;
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
void ChangeClass<T>::redo()
{
  try
  {
    if ( isvalid() )
    {
      failed();
      QStringList Data
      { dbe::config::api::get::attribute::list<QStringList> ( this->checkedref(), Attribute ) };
      OldValue = convert::to<T> ( Data );

      dbe::config::api::set::noactions::aclass ( this->checkedref(), Attribute, NewValue );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    Success = false;
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
#endif // DBE_CHANGE_CLASS_HPP_
