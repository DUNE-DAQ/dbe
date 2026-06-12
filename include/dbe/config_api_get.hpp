/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/config_api_get.h to include/dbe/config_api_get.hpp).
 */

/*
 * config_api_get.h
 *
 *  Created on: Apr 19, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFIG_API_GET_H_
#define DBE_CONFIG_API_GET_H_

#include "dbe/config_reference.hpp"

#include "conffwk/ConfigObject.hpp"
#include "conffwk/Schema.hpp"
#include <QString>
#include <QStringList>

#include <string>
#include <vector>

namespace dbe
{
namespace ui
{
namespace config
{
class info;
} /* namespace config */
} /* namespace ui */
} /* namespace dbe */

namespace dbe
{
namespace config
{
namespace api
{
//------------------------------------------------------------------------------------------
//                                                 GET NAMESPACE
//------------------------------------------------------------------------------------------
/*
 * Retrieve config related structures
 */
namespace get
{
class file
{
public:
  /**
   * Retrieve a list of files included from the database created from the given filename
   * by parsin all  the files included in the file and its inclusions.
   *
   * @param root filename
   * @return a list of files
   */
  static QStringList inclusions ( QStringList const & candidates, QStringList files =
                                    { } );

  /**
   * Retrieve a list of of files from a single file
   *
   * @param the file name to process
   * @return the list of files
   */
  static QStringList inclusions_singlefile ( QString const & );
};

class direct;
/**
 * Permit retrieving information about attributes
 */
class attribute
{
  friend class dbe::config::api::get::direct;

  /**
   * Directly retrieve information about an attribute by directly operating on a config-object
   *
   * @param the object to read the attribute information from
   * @param the attribute to read from the given objects
   * @return a string representation of values
   */
  template<typename T> inline static std::vector<std::string> read (
    dunedaq::conffwk::ConfigObject &, dunedaq::conffwk::attribute_t const & );

  /**
   * Read values associated with an attribute of a given object
   *
   * @param the object to read the attribute information from
   * @param the attribute to read from the given objects
   * @return a string representation of values
   */
  template<typename T>  static std::vector<std::string> read (
    inner::configobject::tref, dunedaq::conffwk::attribute_t const & );
public:

  /**
   * Retrieve a list from of an unspecified type of the values of the attribute of an object
   *
   * @param obj is the reference of the object to operate upon
   * @param attr is the Attribute type information structure
   * @return a list of type T with the attribute values
   */
  template<typename T>  static T list ( dbe::inner::configobject::tref obj,
                                              dunedaq::conffwk::attribute_t const & attr );

};

namespace defaults
{
/**
 * Retrieve default values for objects
 */
struct attribute
{
  static QStringList value ( dunedaq::conffwk::attribute_t const & );
};
}

/**
 * Provide direct access methods to config layer for retrieval of attribute values
 */
class direct
{
  friend class dbe::ui::config::info;
  friend class dbe::config::api::get::attribute;

  template<typename T> static T attribute ( dunedaq::conffwk::ConfigObject &, dunedaq::conffwk::attribute_t const & );
};

}
//------------------------------------------------------------------------------------------

} /* namespace api */
} /* namespace config */
} /* namespace dbe */

#endif /* DBE_CONFIG_API_GET_H_ */
