/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/config_api_graph.h to include/dbe/config_api_graph.hpp).
 */

/*
 * config_api_graph.h
 *
 *  Created on: Apr 20, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFIG_API_GRAPH_H_
#define DBE_CONFIG_API_GRAPH_H_

#include "dbe/tref.hpp"

#include "conffwk/Schema.hpp"
#include "conffwk/ConfigObject.hpp"

#include <vector>

namespace daq
{
namespace config
{
struct relationship_t;
struct attribute_t;
} /* namespace config */
} /* namespace daq */

namespace dbe
{
namespace ui
{
namespace config
{
class info;
}
}
}

//------------------------------------------------------------------------------------------
//                                      DBE::CONFIG::API::GRAPH NAMESPACE
//------------------------------------------------------------------------------------------
namespace dbe
{
namespace config
{
namespace api
{
namespace graph
{

namespace linked
{

//------------------------------------------------------------------------------------------
// backward relations A->c->B querying on B returns A through c ,
// e.g. A connects to B ( B is queried upon)
namespace to
{
/**
 * Get objects referencing a given object
 *
 * e.g. when querying on A
 *
 * A<-c-B , and another A<-c-D returns {B,D}
 * but  A<-c-B , A<-d-B return {B}
 *
 * @param an object for which objects must be retrieved
 * @return a vector objects which link to this object
 */
template<typename T> std::vector<T> object ( tref const & );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// forward relations A->c->B querying on A return B through c
namespace by
{
/**
 * Get objects referenced by a given object
 *
 * e.g. A-c->B , and another A-c->D returns {B,D}
 * but  A-c->B , A-d->B return {B}
 *
 * @param item is the object for which objects must be retrieved
 * @return a vector of linked objects
 */
template<typename T> std::vector<T> object ( tref const & item );
}

namespace through
{
/**
 * Get objects referenced by an object through any relation
 *
 * e.g. A-c->B , and another A-c->D returns {B,D}
 * but  A-c->B , A-d->B return {B}
 *
 * @param item is the object for which objects must be retrieved
 * @return a vector of linked objects
 */
template<typename T> std::vector<T> relations ( tref const & item );

/**
 * Get objects referenced by an object through any attributes
 *
 * e.g. A-c->B , and another A-c->D returns {B,D}
 * but  A-c->B , A-d->B return {B}
 *
 * @param item is the object for which objects must be retrieved
 * @return a vector of linked objects
 */
template<typename T> std::vector<T> attributes ( tref const & item );

/**
 * Get object reference/s through a specified relation
 *
 * e.g. A-C->B
 *
 * returns B for object A through relation C
 *
 * @param item is the object to retrieve the relation value
 * @param relation is the relation to retrieve the value for
 * @return the value of the relation converted to the specified type
 */
template<typename T> T relation ( tref item, dunedaq::conffwk::relationship_t const & relation );

/**
 * Get object reference/s through a specified attribute
 *
 * e.g. A-C->B
 *
 * returns B for object A through relation C
 *
 * @param item is the object to retrieve the relation value
 * @param relation is the relation to retrieve the value for
 * @return the value of the relation converted to the specified type
 */
template<typename T> T attribute ( tref item, dunedaq::conffwk::attribute_t const & attr );

}
//------------------------------------------------------------------------------------------

}// end namespace related

/**
 * Provide direct access to config object methods for retrieval
 */
class direct
{
  friend class dbe::ui::config::info;

  /**
   * Get object reference/s through a specified relation, acting directly on a ConfigObject
   *
   * e.g. A-C->B
   *
   * returns B for object A through relation C
   *
   * @param item is the object to retrieve the relation value
   * @param relation is the relation to retrieve the value for
   * @return the value of the relation converted to the specified type
   */
  template<typename T> static T linked (
    dunedaq::conffwk::ConfigObject & item, dunedaq::conffwk::relationship_t const & relation );

  /**
   * Get object reference/s through a specified relation, acting directly on a ConfigObject
   *
   * e.g. A-C->B
   *
   * returns B for object A through relation C
   *
   * @param item is the object to retrieve the relation value
   * @param relation is the relation to retrieve the value for
   * @return the value of the relation converted to the specified type
   */
  template<typename T> static T linked (
    dunedaq::conffwk::ConfigObject & item, dunedaq::conffwk::attribute_t const & relation );
};
//------------------------------------------------------------------------------------------

}// namespace graph
} /* namespace api */
} /* namespace config */
} /* namespace dbe */
//------------------------------------------------------------------------------------------

#endif /* DBE_CONFIG_API_GRAPH_H_ */
