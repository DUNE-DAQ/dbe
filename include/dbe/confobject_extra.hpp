/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/confobject_extra.hpp to include/dbe/confobject_extra.hpp).
 */

/*
 * confobject_extra.hpp
 *
 *  Created on: Jan 19, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFOBJECT_EXTRA_HPP_
#define DBE_CONFOBJECT_EXTRA_HPP_

#include "dbe/config_api_get.hpp"
#include "dbe/config_api_info.hpp"
#include "dbe/config_api_graph.hpp"
#include "dbe/confobject_desc.hpp"

#include "conffwk/Schema.hpp"
#include <string>

namespace dbe
{

//------------------------------------------------------------------------------------------
template<typename C>
class config_object_extractor
{
public:
  typedef typename C::t_confobjects t_confobjects;
  typedef typename C::t_attrmap t_attrmap;
  typedef typename C::t_relmap t_relmap;

  /**
   * Get attributes defined for an object
   * @param obj the object to parse
   * @return the relational map of attribute names and value lists
   */
  static t_attrmap getattr ( typename C::t_confobject const & obj )
  {
    dunedaq::conffwk::class_t const & classt = dbe::config::api::info::onclass::definition (
                                            obj.class_name(), false );
    t_attrmap attributes;

    for ( dunedaq::conffwk::attribute_t const & attr : classt.p_attributes )
    {
      typename t_attrmap::mapped_type values
      {
        dbe::config::api::get::attribute::list<typename t_attrmap::mapped_type> ( obj.ref(),
        attr ) };
      attributes[attr.p_name] = values;
    }

    return attributes;
  }

  /**
   * Get relations for an object
   *
   * Relations can be filtered depending on the filter function defined in base class C<S>
   *
   * @param obj the object to lookup relations for
   * @return the relational map of relations name and linked objects
   */
  static t_relmap getrel ( typename C::t_confobject const & obj )
  {
    dunedaq::conffwk::class_t const & classt = dbe::config::api::info::onclass::definition (
                                            obj.class_name(), false );
    t_relmap relations;

    for ( dunedaq::conffwk::relationship_t const & link : classt.p_relationships )
    {
      if ( C::filter ( link ) )
      {
        t_confobjects linked
        {
          dbe::config::api::graph::linked::through::relation<t_confobjects> ( obj.ref(),
          link ) };

        if ( not linked.empty() )
        {
          relations[link.p_name] = linked;
        }
      }
    }

    return relations;
  }
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Retrieve an object representation with all its relational aggregate mapping
 */
template<typename S = std::string>
struct config_object_aggregates:
  config_object_representation<S>
{
  typedef typename config_object_representation<S>::t_confobject t_confobject;
  typedef config_object_extractor<config_object_aggregates<S>> t_extractor;

  config_object_aggregates ( config_object_description<S> const & o )
    :
    config_object_representation<S>
  {
    t_extractor::getattr ( o ), t_extractor::getrel ( o ), o
  }
  {
  }

  static bool filter ( dunedaq::conffwk::relationship_t const & l )
  {
    return l.p_is_aggregation;
  }

};
typedef config_object_aggregates<> config_object_aggregator;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Retrieve an object representation with all its relational mappings
 */
template<typename S = std::string>
struct config_object_linked:
  config_object_representation<S>
{
  typedef typename config_object_representation<S>::t_confobject t_confobject;
  typedef config_object_extractor<config_object_linked<S>> t_extractor;

  config_object_linked ( config_object_description<S> const & o )
    :
    config_object_representation<S>
  {
    t_extractor::getattr ( o ), t_extractor::getrel ( o ), o
  }
  {
  }

  static bool filter()
  {
    return true;
  }
};
typedef config_object_linked<> config_object_linker;
//------------------------------------------------------------------------------------------

}// namespace dbe

#endif /* DBE_CONFOBJECT_EXTRA_HPP_ */
