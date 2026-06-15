// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/config_reference_copy.hpp to include/dbe/config_reference_copy.hpp).

/************************************************************
 * config_reference_copy.hpp
 *
 *  Created on: Feb 4, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/

#ifndef DBE_CONFIG_REFERENCE_COPY_HPP_
#define DBE_CONFIG_REFERENCE_COPY_HPP_

#include "dbe/config_reference.hpp"
#include "dbe/confobject_extra.hpp"

#include <functional>

namespace dbe
{

namespace inner
{

namespace configobject
{

//------------------------------------------------------------------------------------------
/**
 * Takes a tref and builds a complete object copy along with related object copies
 *
 * Template parameter permits to use different extractors , which can modify the information
 * that is to be kept from the original object
 */
template<typename T = config_object_linker> class aref: ref_interface<aref<T>>
{
private:
  // A reference to the current reference in the database, which may have been removed
  tref this_object_ref;

  // The copy of the object
  T this_object_image;

  const tref & ref() const
  {
    return this_object_ref;
  }

public:
  aref ( tref const & o )
    : this_object_ref ( o ),
      this_object_image ( o )
  {
  }

  friend class ref_interface<aref> ;
  friend class inner::dbcontroller;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Holds a graph copy of the subgraph associated with the referenced object, which is the
 * result of a deletion operation in dbcontroller. It can only be used to attempt reviving the
 * object
 *
 * The object copy can only be used for passing it to a dbcontroller for recreation
 * of the associated objects. A gref does not support querying the underlying copy.
 */
template<typename T = config_object_linker> class gref
{
public:
  typedef T t_extractor;
  typedef std::function<void ( dref const ) > config_action_notifiable;

  /**
   * Checks if the front of the queue is valid
   *
   * @return true / false on condition evaluation of the state of the object at front
   */
  bool is_null() const;

  /**
   * Passing a notifiable to permits to make a callback with the object and notify of it having been
   * in different states in the the program structure. The notifiable actually determines what type
   * of modification may be sent.
   *
   * @param tele is a notifiable that receives an object descriptor
   */
  void notify ( config_action_notifiable tele );

private:
  typedef std::deque<t_extractor> type_extractor_stack;
  typedef std::vector<dbe::tref> type_ref_container;


  /**
   * Create the objects in the in the queue and return the last object created ,
   * i.e. the bottom of the queue.
   *
   * @return the last object created
   */
  tref rebuild();

  /**
   * Include a ConfigObject and all its descendants
   *
   * @param obj
   */
  void record ( tref const & obj );

  /**
   * Filtering function checks after deletion of objects and keep only those that have been removed
   */
  void post();


  type_extractor_stack this_remove_stack;
  type_ref_container this_notify_stack;

  friend class inner::dbcontroller;
};
//------------------------------------------------------------------------------------------

}  // namespace configobject

}  // namespace inner

}  // namespace dbe

#endif /* DBE_CONFIG_REFERENCE_COPY_HPP_ */
