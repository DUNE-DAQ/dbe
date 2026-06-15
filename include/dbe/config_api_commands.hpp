// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/config_api_commands.h to include/dbe/config_api_commands.hpp).

/*
 * config_api_commands.h
 *
 *  Created on: Apr 19, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFIG_API_COMMANDS_H_
#define DBE_CONFIG_API_COMMANDS_H_

#include "dbe/confobject_desc.hpp"
#include "dbe/config_reference.hpp"

#include "conffwk/Schema.hpp"
#include "conffwk/ConfigObject.hpp"

#include <QMap>
#include <QUuid>
#include <QStringList>

#include <string>
#include <vector>
#include <algorithm>

namespace dbe
{
namespace config
{
namespace api
{
//------------------------------------------------------------------------------------------
//                              COMMANDS NAMESPACE
//------------------------------------------------------------------------------------------
namespace commands
{
/*
 * Enclose methods for the creation of actions related to database / object operations
 */

/**
 * Create a command that once activated will create an object and when undone it will
 * remove the object from the database
 *
 * emits an object_created signal
 *
 * @param fn is the file where it should be created
 * @param class_name is the class it belongs to
 * @param UID of the new UID of the object to be created
 * @param attributes is the attribute map
 * @param relations is the relational map
 */
void newobj ( std::string const & fn, std::string const & class_name,
              std::string const & UID,
              dbe::t_config_object_preimage::type_attrmap const & attributes,
              dbe::t_config_object_preimage::type_relmap const & relations,
              QUuid const & src );

/**
 * Create a command that once activated it will delete an object and when undone
 * will recreate the object from the database
 *
 * emits an object_deleted signal
 *
 * @param obj an object copy shadowing the one to be removed from the database
 * @param src is the internal uuid of the object reqesting the change
 * @return false in case of error
 *  */
bool delobj ( inner::configobject::tref obj, QUuid const & src );

/**
 * Create a command that once activated will rename an object and when undone the
 * object will be renamed to its old name.
 *
 * emits an object_deleted signal
 *
 * @param obj an object copy shadowing the one to be removed from the database
 * @param newuuid is the unique name assigned to the object
 * @return false in case of error
 */
bool renobj ( inner::configobject::tref obj, std::string const & newuuid,
              QUuid const & src );

/**
 * Creates a command object holding move from current file to new file and undoing capabilities
 *
 * @param obj an object copy shadowing the one to be moved
 * @param destination that the object is going to be moved to
 * @return true in case that the move was successful
 */
bool movobj ( inner::configobject::tref obj, std::string const & destination,
              QUuid const & src );

/**
 * Set a RELATION to new values , causes the Object and the objects designated by the new
 * values to become linked
 *
 *  object -> others[x]
 *
 * @param object that holds the outgoing link [ -> ]
 * @param link to set between object that others
 * @param others are the objects to be connected to object
 */
void modobj ( inner::configobject::tref obj, dunedaq::conffwk::relationship_t const & link,
              std::vector<std::string> const & others );

/**
 * Set an ATTRIBUTE to a specific value by generating appropriately modification commands
 *
 * @param obj to be modified
 * @param attr provides information about the attribute to be modified
 * @param value to be assigned to the new object
 */
template<typename T>
void modobj ( inner::configobject::tref obj, dunedaq::conffwk::attribute_t const & attr,
              T value );

namespace file
{
void add ( QString const & db, QString const & fn );

void remove ( QString const & db, QString const & fn );
}

}
//------------------------------------------------------------------------------------------
//                                END COMMANDS NAMESPACE
//------------------------------------------------------------------------------------------
}
}
}

#endif /* DBE_CONFIG_API_COMMANDS_H_ */
