/*
 * config_api_commands.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/confaccessor.hpp"
#include "dbe/config_reference.hpp"
#include "dbe/confobject_desc.hpp"
#include "dbe/Command.hpp"
#include "dbe/dbcontroller.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/messenger.hpp"
#include "dbe/config_api.hpp"
#include "dbe/change_class.hpp"
#include "dbe/change_date.hpp"
#include "dbe/change_enum.hpp"
#include "dbe/change_time.hpp"
#include "dbe/change_attribute.hpp"


#include "conffwk/Schema.hpp"
#include "ers/Issue.hpp"

#include <QString>
#include <QUuid>

#include <stack>
#include <string>
#include <vector>



//------------------------------------------------------------------------------------------
// NAMESPACE DBE::CONFIG::API::COMMANDS
//------------------------------------------------------------------------------------------
/*
 * Purpose of this namespace is to group all commands that generate undo/redo objects
 * that acts on config related structures and objects .
 *
 * DBE user interface should access the database by appropriately calling methods in this
 * namespace. Direct access by accessing that database and retrieving ConfigObject directly
 * is strongly discouraged.
 */
namespace dbe
{
namespace config
{
namespace api
{
namespace commands
{

//------------------------------------------------------------------------------------------
void newobj ( std::string const & fn, std::string const & cn, std::string const & name,
              dbe::t_config_object_preimage::type_attrmap const & attributes,
              dbe::t_config_object_preimage::type_relmap const & relations, QUuid const & src )
{
  std::string description
  { "Object created : " + name + "@" + cn };

  config_internal_change request
    { config_internal_change::CREATED, description, name, cn, "" };

  confaccessor::get_commands()->push (
    new dbe::actions::object::create ( { attributes, relations, { name, cn}, fn }, src ) );

  confaccessor::get_internal_change_stack()->push ( request );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool delobj ( tref obj, QUuid const & src )
{
  try
  {
    std::string const description
    { "Object deleted : " + obj.UID() + "@" + obj.class_name()
    };
    config_internal_change request
      { config_internal_change::DELETED, description, obj.UID(), obj.class_name(), "" };
    confaccessor::get_commands()->push ( new dbe::actions::object::remove ( obj, src ) );
    confaccessor::get_internal_change_stack()->push ( request );
  }
  catch ( daq::dbe::ObjectChangeWasNotSuccessful const & dbe_err )
  {
    INFO ( "Delete Object: The object could not be deleted!\n\n", dbe::config::errors::parse ( dbe_err ).c_str() );
    return false;
  }

  return true;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool renobj ( tref obj, std::string const & newuuid, QUuid const & src )
{
  try
  {
    std::string const description =
    {
      "Object renamed : " + obj.UID() + "@" + obj.class_name() + " to " + newuuid + "@"
      + obj.class_name()
    };

    config_internal_change request
      { config_internal_change::RENAMED, description, obj.UID(), obj.class_name(), "" };

    confaccessor::get_commands()->push (
      new dbe::actions::object::rename ( obj, newuuid, src ) );

    confaccessor::get_internal_change_stack()->push ( request );

    return true;
  }
  catch ( daq::dbe::ObjectChangeWasNotSuccessful const & dbe_err )
  {
    INFO ( "It was not possible to rename object ", dbe::config::errors::parse ( dbe_err ).c_str(), "\n\nObject UID:",
           obj.UID().c_str() );
    return false;
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool movobj ( tref obj, std::string const & destination, QUuid const & src )
{
  try
  {
    std::string const description
    { "Object " + obj.UID() + "@" + obj.class_name() + " contained in " + obj.contained_in()
      + " moved to " + destination
    };

    config_internal_change request
      { config_internal_change::MOVED, description, obj.UID(), obj.class_name(), "" };

    confaccessor::get_commands()->push (
      new dbe::actions::object::move ( obj, destination, src ) );
    confaccessor::get_internal_change_stack()->push ( request );

    return true;
  }
  catch ( daq::dbe::ObjectChangeWasNotSuccessful const & dbe_err )
  {
    INFO ( "It was not possible to move object ", dbe::config::errors::parse ( dbe_err ).c_str(), "\n\nObject UID:",
           obj.UID().c_str() );
    return false;
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void modobj ( tref object, const dunedaq::conffwk::relationship_t & linkinfo,
              std::vector<std::string> const & others_names )
{
  try
  {
    confaccessor::get_commands()->push (
      new dbe::actions::object::changerefs ( object, linkinfo, others_names ) );
    std::string
    const description (
      others_names.size() != 1 ? "Multi Relationship : " + linkinfo.p_name :
      "Single Relationship : " + linkinfo.p_name );

    config_internal_change const change
    {
      config_internal_change::MODIFIED, description, object.UID(), object.class_name(), ""
    };

    confaccessor::get_internal_change_stack()->push ( change );
  }
  catch ( daq::dbe::ObjectChangeWasNotSuccessful const & dbe_err )
  {
    WARN ( "The object reference could not be changed", dbe::config::errors::parse ( dbe_err ).c_str() );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<>
void modobj<std::string> ( tref Object, const dunedaq::conffwk::attribute_t & AttributeData,
                           std::string Value )
{
  try
  {
    config_internal_change Change;
    Change.uid = Object.UID();
    Change.classname = Object.class_name();
    Change.request = config_internal_change::MODIFIED;
    Change.description = QString ( "Attribute " ).append ( AttributeData.p_name.c_str() )
                         .toStdString();

    if ( AttributeData.p_type == dunedaq::conffwk::string_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeAttribute<std::string> ( Object, AttributeData, Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::enum_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeEnum<std::string> ( Object, AttributeData, Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::class_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeClass<std::string> ( Object, AttributeData, Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::date_type )
    {

      confaccessor::get_commands()->push (
        new dbe::actions::ChangeDate<std::string> ( Object, AttributeData, Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::time_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeTime<std::string> ( Object, AttributeData, Value ) );
    }

    confaccessor::get_internal_change_stack()->push ( Change );
  }
  catch ( daq::dbe::ObjectChangeWasNotSuccessful const & dbe_err )
  {
    WARN ( "The object attribute could not be changed", dbe::config::errors::parse ( dbe_err ).c_str() );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<>
void modobj<std::vector<std::string>> (
                                     tref Object, const dunedaq::conffwk::attribute_t & AttributeData,
                                     std::vector<std::string> Value )
{
  try
  {
    config_internal_change Change;
    Change.uid = Object.UID();
    Change.classname = Object.class_name();
    Change.request = config_internal_change::MODIFIED;
    Change.description = QString ( "Attribute " ).append ( AttributeData.p_name.c_str() )
                         .toStdString();

    if ( AttributeData.p_type == dunedaq::conffwk::string_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeAttribute<std::vector<std::string>> ( Object, AttributeData,
                                                                      Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::enum_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeEnum<std::vector<std::string>> ( Object, AttributeData,
                                                                 Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::class_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeClass<std::vector<std::string>> ( Object, AttributeData,
                                                                  Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::date_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeDate<std::vector<std::string>> ( Object, AttributeData,
                                                                 Value ) );
    }
    else if ( AttributeData.p_type == dunedaq::conffwk::time_type )
    {
      confaccessor::get_commands()->push (
        new dbe::actions::ChangeTime<std::vector<std::string>> ( Object, AttributeData,
                                                                 Value ) );
    }

    confaccessor::get_internal_change_stack()->push ( Change );
  }
  catch ( daq::dbe::ObjectChangeWasNotSuccessful const & dbe_err )
  {
    WARN ( "The object attribute could not be changed", dbe::config::errors::parse ( dbe_err ).c_str()  );
  }
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------





//------------------------------------------------------------------------------------------
namespace file
{
void remove ( QString const & db, QString const & fn )
{
  std::string dbname = db.toStdString();
  std::string sinclude = fn.toStdString();

  try
  {
    config_internal_change Change;
    Change.filename = fn.toStdString();
    Change.request = config_internal_change::FILE_DELETED;
    Change.description = QString ( "File(deleted) : " ).append ( fn ).toStdString();

    confaccessor::get_commands()->push ( new dbe::actions::file::remove ( dbname, sinclude ) );
    confaccessor::get_internal_change_stack()->push ( Change );
  }
  catch ( daq::dbe::DatabaseChangeNotSuccessful const & dbe_err )
  {
    WARN ( "File removal failure", dbe::config::errors::parse ( dbe_err ).c_str() );
  }
}


void add ( const QString & Database, const QString & IncludedFile )
{
  std::string dbname = Database.toStdString();
  std::string sinclude = IncludedFile.toStdString();

  try
  {
    config_internal_change Change;
    Change.filename = IncludedFile.toStdString();
    Change.request = config_internal_change::FILE_INCLUDED;
    Change.description = QString ( "File(included) : " ).append ( IncludedFile ).toStdString();

    confaccessor::get_commands()->push ( new dbe::actions::file::add ( dbname, sinclude ) );
    confaccessor::get_internal_change_stack()->push ( Change );
  }
  catch ( daq::dbe::DatabaseChangeNotSuccessful const & dbe_err )
  {
    WARN ( "File inclusion did not succeed", dbe::config::errors::parse ( dbe_err ).c_str() );
  }
}

}

}
}
}
}
//------------------------------------------------------------------------------------------



