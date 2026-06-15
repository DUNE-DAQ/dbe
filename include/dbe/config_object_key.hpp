// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/config_object_key.hpp to include/dbe/config_object_key.hpp).

/*
 * config_object_key.hpp
 *
 *  Created on: Feb 4, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFIG_OBJECT_KEY_HPP_
#define DBE_CONFIG_OBJECT_KEY_HPP_

#include <string>

namespace dbe
{

/**
 * The basic information needed to designate where an object is actually found
 */
template<typename S = std::string> class config_object_key
{
public:
  S this_name;
  S this_class;

  config_object_key ( S const & oname, S const & cname )
    : this_name ( oname ),
      this_class ( cname )
  {
  }
};
typedef config_object_key<> cokey;

//------------------------------------------------------------------------------------------
/**
 * Comparison between to orefs always results in comparison of their full name
 *
 * @param left oref to compare
 * @param right oref to compare
 * @return true in case the have the same full name
 */
template<typename T> inline bool operator == ( config_object_key<T> const & left,
                                               config_object_key<T> const & right )
{
  return left.this_name == right.this_name and left.this_class == right.this_class;
}
//------------------------------------------------------------------------------------------

}// namespace dbe

#endif /* DBE_CONFIG_OBJECT_KEY_HPP_ */
