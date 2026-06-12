/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/*
 * config_api_set.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/config_api_set.hpp"
#include "dbe/config_api_info.hpp"
#include "dbe/config_api.hpp"
#include "dbe/config_api_commands.hpp"
#include "dbe/config_object_key.hpp"
#include "dbe/Conversion.hpp"
#include "dbe/dbcontroller.hpp"

#include "conffwk/Schema.hpp"

#include <QString>

#include <stdint.h>
#include <sys/types.h>
#include <algorithm>
#include <iterator>

//------------------------------------------------------------------------------------------
// NAMESPACE DBE::CONFIG::API::SET::NOACTIONS
//------------------------------------------------------------------------------------------
namespace dbe
{
namespace config
{
namespace api
{
namespace set
{
namespace noactions
{

void relation ( tref object, dunedaq::conffwk::relationship_t const & arelation,
                std::vector<dbe::cokey> const & keys )
{
  std::vector<dbe::inner::configobject::tref> trefs;

  for ( dbe::cokey const & key : keys )
  {
    trefs.push_back ( dbe::inner::dbcontroller::get ( key ) );
  }

  dbe::config::api::set::noactions::relation ( object, arelation, trefs );
}

void relation ( tref src, dunedaq::conffwk::relationship_t const & edge,
                std::vector<tref> const & targets )
{
  try
  {
    if ( not targets.empty() )
    {
      if ( info::relation::is_simple ( edge ) )
      {
        src.set_obj ( edge.p_name, targets.back() );
      }
      else
      {
        src.set_objs ( edge.p_name, targets );
      }
    }
    else
    {
     	src.set_obj_null( edge.p_name, info::relation::is_simple(edge) );
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, config::errors::parse ( e ) );
  }
}
//------------------------------------------------------------------------------------------
}
}
}
}
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// NAMESPACE DBE::CONFIG::API::SET
//------------------------------------------------------------------------------------------
namespace dbe
{
namespace config
{
namespace api
{
namespace set
{

//------------------------------------------------------------------------------------------
void relation ( tref src, dunedaq::conffwk::relationship_t const & edge,
                QStringList const & targets )
{
  std::vector<std::string> targetnames;

  for ( QString const & name : targets )
  {
    targetnames.push_back ( name.toStdString() );
  }

  commands::modobj ( src, edge, targetnames );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void attribute ( tref Object, dunedaq::conffwk::attribute_t const & attribute_info,
                 QStringList const & values )
{
  switch ( attribute_info.p_type )
  {
  case dunedaq::conffwk::bool_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<bool>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<bool> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::enum_type:
  case dunedaq::conffwk::date_type:
  case dunedaq::conffwk::time_type:
  case dunedaq::conffwk::string_type:
  case dunedaq::conffwk::class_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<std::vector<std::string>> ( values ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info, convert::to<std::string> ( values ) );
    }

    break;
  }

  case dunedaq::conffwk::float_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<float>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<float> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::double_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<double>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<double> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::s8_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<int8_t>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<int8_t> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::s16_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<int16_t>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<int16_t> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::s32_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<int32_t>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<int32_t> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::s64_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<int64_t>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<int64_t> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::u8_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<u_int8_t>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj ( Object, attribute_info,
                         convert::to<u_int8_t> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::u16_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<u_int16_t>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<u_int16_t> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::u32_type:
  {
    if ( attribute_info.p_is_multi_value )
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<u_int32_t>> ( values, attribute_info.p_int_format ) );
    }
    else
    {
      commands::modobj (
        Object, attribute_info,
        convert::to<u_int32_t> ( values, attribute_info.p_int_format ) );
    }

    break;
  }

  case dunedaq::conffwk::u64_type:
  {
    if ( attribute_info.p_is_multi_value ) commands::modobj (
        Object, attribute_info,
        convert::to<std::vector<u_int64_t>> ( values, attribute_info.p_int_format ) );
    else commands::modobj (
        Object, attribute_info,
        convert::to<u_int64_t> ( values, attribute_info.p_int_format ) );

    break;
  }

  default:
    break;
  }
}
//------------------------------------------------------------------------------------------
}
}
}
}
//------------------------------------------------------------------------------------------

