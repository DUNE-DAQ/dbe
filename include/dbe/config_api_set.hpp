/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/config_api_set.h to include/dbe/config_api_set.hpp).
 */

/*
 * config_api_set.h
 *
 *  Created on: Apr 19, 2016
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/config_object_key.hpp"
#include "dbe/config_reference.hpp"
#include "conffwk/Schema.hpp"

#include <QStringList>

#include <string>
#include <vector>

#ifndef DBE_CONFIG_API_SET_H_
#define DBE_CONFIG_API_SET_H_

namespace daq
{
namespace config
{
struct attribute_t;
struct relationship_t;
} /* namespace config */
} /* namespace daq */

namespace dbe
{

namespace config
{

namespace api
{

//------------------------------------------------------------------------------------------
//                                                 SET NAMESPACE
//------------------------------------------------------------------------------------------
/*
 * Set config related objects and structures
 */
namespace set
{
/**
 * Change the value of an attribute and set it appropriately to new values
 * by generating action commands, register this to the undo stack
 *
 * @param objectref
 * @param attribute_info a single or multi-value attribute, with name, and class information
 * @param attribute_values is a list (can have only one) of values
 */
void attribute ( dbe::inner::configobject::tref objectref,
                 dunedaq::conffwk::attribute_t const & attribute_info,
                 QStringList const & attribute_values );

/**
 * Link an object with one or many other objects
 * by generating action commands, register this to the undo stack
 *
 * object->objects
 *
 * objects linked must be of the relation class type
 *
 * @param object from which an outgoing link is set
 * @param relation_info the associate relation type information
 * @param object_names the name of the objects to be linked
 */
void relation ( dbe::inner::configobject::tref src,
                dunedaq::conffwk::relationship_t const & edge,
                QStringList const & targets );


namespace noactions
{
/*
 * Methods in this namespace do not generate undo/redo commands,
 * mostly used for internal actions on modifying objects
 */

/**
 * Change the value of an attribute and set it appropriately to new values
 *
 * @param objectref is the reference to the object to be modified
 * @param attribute_info a single or multi-value attribute, with name, and class information
 * @param attribute_values is a list (can have only one) of values
 */
template<typename T>
void attribute ( inner::configobject::tref Object,
                 dunedaq::conffwk::attribute_t const & AttributeData, T NewValueData,
                 bool NotEmit = false );

template<typename T>
void aclass ( inner::configobject::tref Object,
              dunedaq::conffwk::attribute_t const & AttributeData, T NewValueData, bool NotEmit =
                false );

template<typename T>
void anenum ( inner::configobject::tref Object,
              dunedaq::conffwk::attribute_t const & AttributeData, T NewValueData, bool NotEmit =
                false );


template<typename T>
void adate ( inner::configobject::tref Object,
             dunedaq::conffwk::attribute_t const & AttributeData, T NewValueData, bool NotEmit =
               false );


template<typename T>
void atime ( inner::configobject::tref Object,
             dunedaq::conffwk::attribute_t const & AttributeData, T NewValueData, bool NotEmit =
               false );

/**
 * Link an object with one or many other objects directly by acting on the reference
 *
 * object->objects
 *
 * objects linked must be of the relation class type
 *
 * @param an object from which an outgoing link is set
 * @param relation information for the edge
 * @param a list of objects to link to
 */
void relation ( dbe::inner::configobject::tref src,
                dunedaq::conffwk::relationship_t const & edge,
                std::vector<dbe::inner::configobject::tref> const & targets );

/**
 * Link an object with one or many other objects
 *
 * object->objects
 *
 * objects linked must be of the relation class type
 *
 * @param object from which an outgoing link is set
 * @param relation_info the associate relation type information
 * @param object_names the name of the objects to be linked
 */
void relation ( dbe::inner::configobject::tref object,
                dunedaq::conffwk::relationship_t const & arelation,
                std::vector<dbe::cokey> const & keys );

//------------------------------------------------------------------------------------------
} // end noactions namespace
} // end namespace set
}  // namespace api
}  // namespace config
}  // namespace dbe
//------------------------------------------------------------------------------------------

#endif
