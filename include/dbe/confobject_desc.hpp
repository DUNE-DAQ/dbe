/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/confobject_desc.hpp to include/dbe/confobject_desc.hpp).
 */

/*
 * confobject_desc.hpp
 *
 *  Created on: Jan 14, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFOBJECT_DESC_HPP_
#define DBE_CONFOBJECT_DESC_HPP_

#include "dbe/config_object_key.hpp"
#include "dbe/config_reference.hpp"

#include <QStringList>
#include <map>
#include <string>
#include <vector>
#include <iterator>

namespace dbe
{
//------------------------------------------------------------------------------------------
/**
 * Provides a shallow copy holder , copies name, class and file only
 *
 * Permits to create an identical object and populate with new information
 *
 * An reference to the object is held internally, but this may be in an inconsistent state
 * with respect to the information held inside this descriptor
 */
template<typename S = std::string>
class config_object_description: private config_object_key<S>
{
private:
  mutable inner::configobject::tref this_referenced_object;
  S this_file;

  friend void transfer ( config_object_description & one,
                         config_object_description & another ) noexcept
  {
    std::swap ( one.this_referenced_object, another.this_referenced_object );
    std::swap ( one.this_class, another.this_class );
    std::swap ( one.this_name, another.this_name );
    std::swap ( one.this_file, another.this_file );
  }

public:
  config_object_description ( dbe::inner::configobject::tref const & o ) noexcept ( false )
    : config_object_key<S>
  { o.UID(), o.class_name() },
  this_referenced_object ( o ),
  this_file ( o.contained_in() )
  {
  }

  config_object_description ( config_object_description const & other )
    : config_object_key<S>
  { other.this_name, other.this_class },
  this_referenced_object
  { other.this_referenced_object },
  this_file ( other.this_file )
  {

  }

  config_object_description operator= ( config_object_description other )
  {
    transfer ( *this, other );
    return *this;
  }

  S UID() const
  {
    return this->this_name;
  }

  S class_name() const
  {
    return this->this_class;
  }

  S contained_in() const
  {
    return this->this_file;
  }

  bool is_valid() const
  {
    return not this_referenced_object.is_null();
  }

  /**
   * Returns an always valid reference to the underlying object
   *
   * If the object cannot be found in the database , an appropriate exception will be
   * thrown from the underlyin dbcontroller::get() lookup , and should be handled by
   * caller, accordingly.
   *
   * @return a transaction reference (tref) to the underlying object
   */
  inner::configobject::tref ref() const;

};
typedef config_object_description<> dref;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename S = std::string, typename I = std::string, typename A = std::string,
         template<typename ... > class V = std::vector, template <typename ...> class M = std::map >
struct config_object_image
{
  typedef S type_str;
  typedef I type_image;
  typedef A type_attribute_value;

  typedef V<type_image> type_images;
  typedef V<type_attribute_value> type_attributes;

  typedef M<type_str, type_attributes> type_attrmap;
  typedef M<type_str, type_images> type_relmap;

  typedef type_str type_file_name;

  type_attrmap attributes;
  type_relmap relations;

  type_image ref;
  type_file_name fn;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Provides a preimage abstraction of a config object by encoding the information
 * needed for the creation of a ConfigObject in the Configuration database.
 */
template<typename S> using config_object_preimage =
  config_object_image<S, config_object_key<S>, S>;
typedef config_object_preimage<std::string> t_config_object_preimage;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Provides a holder for deep copies of a config object
 */
template<typename S = std::string>
struct config_object_representation
{
  typedef S t_str;
  typedef config_object_description<S> t_confobject;
  typedef std::vector<t_confobject> t_confobjects;
  typedef std::map<S, QStringList> t_attrmap;
  typedef std::map<S, t_confobjects> t_relmap;

  t_attrmap attributes;
  t_relmap relations;

  t_confobject ref;

  t_config_object_preimage toimage()
  {
    t_config_object_preimage::type_attrmap lattributes;
    t_config_object_preimage::type_relmap lrelations;

    std::for_each ( std::begin ( this->attributes ), std::end ( this->attributes ),
                    [&lattributes] ( typename std::pair<S, QStringList> const & e )
    {
      t_config_object_preimage::type_attrmap::value_type::second_type v;

      for ( auto const & x : e.second )
      {
        v.push_back ( x.toStdString() );
      }

      lattributes.insert ( std::make_pair ( e.first, v ) );
    } );

    std::for_each ( std::begin ( this->relations ), std::end ( this->relations ),
                    [&lrelations] ( typename t_relmap::value_type const & e )
    {
      t_config_object_preimage::type_relmap::value_type::second_type v;

      for ( t_confobject const & x : e.second )
      {
        v.push_back ( { x.UID(), x.class_name() } );
      }

      lrelations.insert ( std::make_pair ( e.first, v ) );
    } );

    return
    {
      lattributes, lrelations,
      { this->ref.UID(), this->ref.class_name() }, this->ref.contained_in() };
  }
};
typedef config_object_representation<> t_config_object_rep;
//------------------------------------------------------------------------------------------

}// end namespace dbe

#endif /* DBE_CONFOBJECT_DESC_HPP_ */
