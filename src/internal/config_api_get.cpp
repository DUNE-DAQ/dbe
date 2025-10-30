/*
 * config_api_get.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/config_api_get.hpp"
#include "dbe/Conversion.hpp"
#include "dbe/dbaccessor.hpp"
#include "dbe/messenger.hpp"
#include "dbe/config_api.hpp"

using namespace dunedaq::conffwk;


//------------------------------------------------------------------------------------------
// NAMESPACE DBE::CONFIG::API::GET
//------------------------------------------------------------------------------------------

namespace dbe
{

namespace config
{

namespace api
{

namespace get
{
//------------------------------------------------------------------------------------------

template<typename T>
inline std::vector<std::string> attribute::read (
  ConfigObject & input, dunedaq::conffwk::attribute_t const & attr )
{
  std::vector<T> attrvalues;

  if ( attr.p_is_multi_value )
  {
    attrvalues = direct::attribute<std::vector<T>> ( input, attr );
  }
  else
  {
    attrvalues =
    { direct::attribute<T> ( input, attr ) };
  }

  std::vector<std::string> strings ( attrvalues.size() );
  std::transform ( attrvalues.begin(), attrvalues.end(),
                   strings.begin(), [&attr] ( T const & x )
  {
    return convert::valtostr ( x, attr.p_int_format );
  }

                 );

  return strings;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T>
inline T direct::attribute ( ConfigObject & input,
                             dunedaq::conffwk::attribute_t const & attr )
{
  T value;
  input.get ( attr.p_name, value );
  return value;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<>
QStringList direct::attribute<QStringList> ( ConfigObject & input,
                                             dunedaq::conffwk::attribute_t const & attr )
{
  std::vector<std::string> data;

  switch ( attr.p_type )
  {

  case dunedaq::conffwk::bool_type:
    data = get::attribute::read<bool> ( input, attr );
    break;

  case dunedaq::conffwk::enum_type:

  case dunedaq::conffwk::date_type:

  case dunedaq::conffwk::time_type:

  case dunedaq::conffwk::string_type:

  case dunedaq::conffwk::class_type:
    data = get::attribute::read<std::string> ( input, attr );
    break;

  case dunedaq::conffwk::float_type:
    data = get::attribute::read<float> ( input, attr );
    break;

  case dunedaq::conffwk::double_type:
    data = get::attribute::read<double> ( input, attr );
    break;

  case dunedaq::conffwk::s8_type:
    data = get::attribute::read<int8_t> ( input, attr );
    break;

  case dunedaq::conffwk::u8_type:
    data = get::attribute::read<u_int8_t> ( input, attr );
    break;

  case dunedaq::conffwk::s16_type:
    data = get::attribute::read<int16_t> ( input, attr );
    break;

  case dunedaq::conffwk::u16_type:
    data = get::attribute::read<u_int16_t> ( input, attr );
    break;

  case dunedaq::conffwk::s32_type:
    data = get::attribute::read<int32_t> ( input, attr );
    break;

  case dunedaq::conffwk::u32_type:
    data = get::attribute::read<u_int32_t> ( input, attr );
    break;

  case dunedaq::conffwk::s64_type:
    data = get::attribute::read<int64_t> ( input, attr );
    break;

  case dunedaq::conffwk::u64_type:
    data = get::attribute::read<u_int64_t> ( input, attr );
    break;
  }

  QStringList result;

  for ( auto const & x : data )
  {
    result.push_back ( x.c_str() );
  }

  return result;

}

template
QStringList direct::attribute<QStringList>
( ConfigObject &,
  dunedaq::conffwk::attribute_t const & );

template
ConfigObject direct::attribute<ConfigObject>
( ConfigObject &,
  dunedaq::conffwk::attribute_t const & );

template
std::vector<ConfigObject>
direct::attribute<std::vector<ConfigObject>> (
                                            ConfigObject &, dunedaq::conffwk::attribute_t const & );

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T>
inline T attribute::list ( dbe::inner::configobject::tref obj,
                           dunedaq::conffwk::attribute_t const & Attribute )
{
  std::vector<std::string> attrvalues;

  switch ( Attribute.p_type )
  {

  case dunedaq::conffwk::bool_type:
    attrvalues = get::attribute::read<bool> ( obj, Attribute );
    break;

  case dunedaq::conffwk::enum_type:

  case dunedaq::conffwk::date_type:

  case dunedaq::conffwk::time_type:

  case dunedaq::conffwk::string_type:

  case dunedaq::conffwk::class_type:
    attrvalues = get::attribute::read<std::string> ( obj, Attribute );
    break;

  case dunedaq::conffwk::float_type:
    attrvalues = get::attribute::read<float> ( obj, Attribute );
    break;

  case dunedaq::conffwk::double_type:
    attrvalues = get::attribute::read<double> ( obj, Attribute );
    break;

  case dunedaq::conffwk::s8_type:
    attrvalues = get::attribute::read<int8_t> ( obj, Attribute );
    break;

  case dunedaq::conffwk::u8_type:
    attrvalues = get::attribute::read<u_int8_t> ( obj, Attribute );
    break;

  case dunedaq::conffwk::s16_type:
    attrvalues = get::attribute::read<int16_t> ( obj, Attribute );
    break;

  case dunedaq::conffwk::u16_type:
    attrvalues = get::attribute::read<u_int16_t> ( obj, Attribute );
    break;

  case dunedaq::conffwk::s32_type:
    attrvalues = get::attribute::read<int32_t> ( obj, Attribute );
    break;

  case dunedaq::conffwk::u32_type:
    attrvalues = get::attribute::read<u_int32_t> ( obj, Attribute );
    break;

  case dunedaq::conffwk::s64_type:
    attrvalues = get::attribute::read<int64_t> ( obj, Attribute );
    break;

  case dunedaq::conffwk::u64_type:
    attrvalues = get::attribute::read<u_int64_t> ( obj, Attribute );
    break;
  }

  return dbe::convert::to<T> ( attrvalues );
}

// Template method declaration of attribute::list for type QStringList
template
QStringList attribute::list<QStringList> ( dbe::inner::configobject::tref obj,
                              dunedaq::conffwk::attribute_t const & );
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QStringList file::inclusions ( QStringList const & candidates, QStringList files )
{
  if ( candidates.isEmpty() )
  {
    return files;
  }
  else
  {
    QStringList newcandidates =
    { };

    for ( QString const & fname : candidates )
    {
      if ( not ( fname.endsWith ( "schema.xml" ) or files.contains ( fname ) ) )
      {
        files.push_back ( fname );
        // Query the current file for other inclusions
        std::list<std::string> configfiles;

        try
        {
          if ( not ( fname.contains ( "rdbconfig:" ) or fname.contains ( "roksconflibs:" ) ) )
          {
            // Process file based sources
            dbaccessor::dbptr()->get_includes ( fname.toStdString(), configfiles );
          }
          else
          {
            // Process other sources (e.g. Oracle and RDB )
            dbaccessor::dbptr()->get_includes ( "", configfiles );
            files.removeAll ( fname ); // Once used RDB / Oracle sources must be removed
          }
        }
        catch ( dunedaq::conffwk::Exception const & ex )
        {
          ERROR ( "Include did not succeed for ", dbe::config::errors::parse ( ex ).c_str(),
                  "filename:", fname.toStdString().c_str() );
        }

        // Keep only files that have not already been processed
        // i.e. they are not in the list of given files.
        for ( std::string const & configfile : configfiles )
        {
          QString const & qconfigfile = QString::fromStdString ( configfile );

          if ( not files.contains ( qconfigfile ) )
          {
            newcandidates.push_back ( qconfigfile );
          }
        }

      }

    }

    return get::file::inclusions ( newcandidates, files );
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QStringList file::inclusions_singlefile ( QString const & FileName )
{
  std::string dbname = FileName.toStdString();
  std::list<std::string> incList;

  QStringList dbfiles;

  try
  {
    dbaccessor::dbptr()->get_includes ( dbname, incList );

    for ( std::string const & includeName : incList )
    {
      dbfiles.push_back ( QString ( includeName.c_str() ) );
    }
  }
  catch ( dunedaq::conffwk::Exception & ex )
  {
    WARN ( "Include : Could not retrieve included files",
           dbe::config::errors::parse ( ex ).c_str() );
  }

  return dbfiles;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QStringList get::defaults::attribute::value ( dunedaq::conffwk::attribute_t const & attr )
{
  return QString::fromStdString ( attr.p_default_value ).split ( "," );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
}
}
}
}

//------------------------------------------------------------------------------------------

