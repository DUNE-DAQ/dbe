// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/*
 * dbcontroller.cpp
 *
 *  Created on: Oct 30, 2015
 *      Author: lgeorgop
 */

#include "dbe/dbcontroller.hpp"
#include "dbe/config_reference_copy.hpp"
#include "dbe/messenger.hpp"
#include "dbe/msghandler.hpp"

#include <utility>

using namespace dunedaq::conffwk;

namespace dbe
{
namespace inner
{

namespace configobject
{
//------------------------------------------------------------------------------------------
//																	NAMESPACE DBE::INNER::CONFIGOBJECT
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T>
std::vector<T> ref_interface<T>::referenced_by(std::string const & name,
    bool check_composite_only) const
{
  std::vector<ConfigObject> referees;
  std::vector<tref> result;

  static_cast<ConfigObject &>(*this).referenced_by(referees, name, check_composite_only);

  for (ConfigObject const & x : referees)
  {
    try
    {
      result.push_back(inner::dbcontroller::get(
                         { x.UID(), x.class_name() }));
    }
    catch (daq::dbe::config_object_retrieval_result_is_null const & e)
    {
      // Nothing needed to do here, this is normal, just the query has resulted in null
    }
  }

  return result;
}

template
std::vector<tref> ref_interface<tref>::referenced_by(std::string const &, bool) const;

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> void gref<T>::record(tref const & o)
{
  t_extractor obj(o);
  for (auto const & linked : obj.relations)
  {
    for (auto const & candidate : linked.second)
    {
      record(candidate.ref());
    }
  }
  this_remove_stack.push_back(obj);
}

template<typename T> void gref<T>::post()
{
  type_extractor_stack acopy;
  std::swap(acopy, this_remove_stack);

  while (not acopy.empty())
  {
    t_extractor candidate = acopy.front();
    // Keep only objects that have been deleted
    if (not dbe::config::api::info::has_obj(candidate.ref.class_name(),
                                            candidate.ref.UID()))
    {
      this_remove_stack.push_back(candidate);
    }
    acopy.pop_front();
  }
}

template<typename T> void gref<T>::notify(config_action_notifiable tele)
{
  if (this_notify_stack.empty())
  {
    for (typename type_extractor_stack::value_type const & val : this_remove_stack)
    {
      tele(val.ref);
    }
  }
  else
  {
    for (typename type_ref_container::value_type const & val : this_notify_stack)
    {
      tele(val);
    }
  }
}

template<typename T> tref gref<T>::rebuild()
{
  if (not this_remove_stack.empty())
  {
    for (;;)
    {
      t_extractor record = this_remove_stack.front();
      tref last = dbcontroller::create_object_request(record.toimage());
      this_remove_stack.pop_front();
      if (not last.is_null())
      {
        this_notify_stack.push_back(last);
      }
      if (this_remove_stack.empty())
      {
        return last;
      }
    }
  }
  throw daq::dbe::gref_empty_internal_queue_is_invalid_state(ERS_HERE);
}

template<typename T> bool gref<T>::is_null() const
{
  return this_remove_stack.front().is_null();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T>
tref authorized_getter<T, tref>::operator()(std::string const & key)
{
  ConfigObject voisin
  { this->that->template getdirect<ConfigObject>(key) };

  if (voisin.is_null())
  {
    throw daq::dbe::config_object_retrieval_result_is_null(ERS_HERE,key);
  }

  // There is no need to catch the resulting exception from get because the object
  // has just been retrieved from the database, and the call to get just generates
  // the tref and adds the object to the internal lookup table.
  return dbcontroller::get({ voisin.UID(), voisin.class_name() });
}

template<typename T>
std::vector<tref> authorized_getter<T, std::vector<tref>>::operator()(
  std::string const & key)
{
  std::vector<ConfigObject> voisins
  { this->that->template getdirect<std::vector<ConfigObject>>(key) };

  std::vector<tref> references;

  for (ConfigObject const & voisin : voisins)
  {
    try
    {
      references.push_back(dbcontroller::get(
                             { voisin.UID(), voisin.class_name() }));
    }
    catch (daq::dbe::config_object_retrieval_result_is_null const & ex)
    {
      // Actually there is no need to handle this error here ,
      // since the object will not be added to the result list of references,
      // and can be safely ignored
    }
  }

  return references;
}

template class authorized_getter<tref, tref> ;
template class authorized_getter<tref, std::vector<tref>> ;
//------------------------------------------------------------------------------------------

}// end namespace configobject

//------------------------------------------------------------------------------------------
//																	                NAMESPACE DBE::INNER
//------------------------------------------------------------------------------------------

dbcontroller::t_mutex dbcontroller::this_lock;

//------------------------------------------------------------------------------------------
dbcontroller::dbcontroller() = default;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbcontroller & dbcontroller::ref()
{
  static dbcontroller self;
  return self;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbcontroller::flush()
{
  dbcontroller & me = dbcontroller::ref();
  locker l(me.this_lock);
  me.this_allobjects.clear();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
std::vector<tref> dbcontroller::referenced_by(configobject::tref objref,
    std::string const & name,
    bool check_composite_only)
{
  std::vector<ConfigObject> linked;
  objref.ref().referenced_by(linked, name, check_composite_only);

  std::vector<configobject::tref> references;

  for (ConfigObject const & anobj : linked)
  {
    try
    {
      references.push_back(get(
                             { anobj.UID(), anobj.class_name() }));
    }
    catch (daq::dbe::config_object_retrieval_result_is_null const & e)
    {
      // nothing needed to do here, since this just signal that
      // some of the relation results are null
    }
  }

  return references;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> configobject::gref<T> dbcontroller::delete_object_request(
  configobject::tref const & obj,
  typename configobject::gref<T>::config_action_notifiable notice)
{
  configobject::gref<T> subgraph = dbcontroller::ref().remove<T>(obj);
  subgraph.notify(notice);
  return subgraph;
}

template
configobject::gref<dbe::config_object_aggregates<std::string>>
dbcontroller::delete_object_request (
  configobject::tref const &,
  configobject::gref<dbe::config_object_aggregates<std::string>>::config_action_notifiable);

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/*
 * Create an object in the database.
 *
 * If the object cannot be created an exception will be thrown
 * by the underlying database access layer, as it cannot be handled here.
 *
 * If the object is already in the underlying container the object reference will be updated
 * to point to the new object reference, transparently to user classes of the controller
 */
configobject::tref dbcontroller::create_object_request(
  dbe::t_config_object_preimage const & image)
{
  ConfigObject result = config::api::rwdacc::create_object(image.fn,
      image.ref.this_class,
      image.ref.this_name);

  tref el = dbcontroller::ref().insert(result);
  try
  {
    return dbe::config::api::rwdacc::set_object(el,image.attributes, image.relations);
  }
  catch (dunedaq::conffwk::Exception const & e)
  {
    // just need to remove the object from the underlying database
    dbcontroller::ref().remove<config_object_aggregator>(el);
    throw;
  }
  catch (daq::dbe::Exception const & e)
  {
    // just need to remove the object from the underlying database
    dbcontroller::ref().remove<config_object_aggregator>(el);
    throw;
  }
}

template<typename T> configobject::tref dbcontroller::create_object_request(
  configobject::aref<T> const & obj)
{
  return dbcontroller::create_object_request(obj.this_object_image);
}

template<typename T> configobject::tref dbcontroller::create_object_request(
  configobject::gref<T> const & obj)
{
  return obj.rebuild();
}

template<typename T> configobject::tref dbcontroller::create_object_request(
  configobject::gref<T> & obj,
  typename configobject::gref<T>::config_action_notifiable notice)
{
  tref ret = obj.rebuild();
  obj.notify(notice);
  return ret;
}

template dbe::inner::configobject::tref dbe::inner::dbcontroller::create_object_request<
dbe::config_object_aggregates<std::string> >(
  dbe::inner::configobject::gref<dbe::config_object_aggregates<std::string> > &,
  dbe::inner::configobject::gref<dbe::config_object_aggregates<std::string> >::config_action_notifiable);
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
configobject::tref dbcontroller::move_object_request(configobject::tref objref,
    std::string const & destfile)
{
  if (not objref.is_null())
  {
    static_cast<ConfigObject &>(*(objref.refered)).move(destfile);
  }

  return objref;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
configobject::tref dbcontroller::rename_object_request(configobject::tref objref,
    std::string const & newname)
{
  return dbcontroller::ref().rename(objref, newname);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
configobject::tref dbcontroller::rename(configobject::tref objref,
                                        std::string const & aname)
{
  if (not objref.is_null())
  {
    // pick up the pointer
    std::shared_ptr<configobject::oref> current_ptr = objref.refered;

    // remove the current reference from the internal map
    this_allobjects.erase(*current_ptr);
    try
    {
      // rename the refered object
      dbe::config::api::rwdacc::rename_object(static_cast<ConfigObject &>(*current_ptr),
                                              aname);
    }
    catch (dunedaq::conffwk::Generic const & e)
    {
      // Logging the error but this may not affect program execution per-se
      FAIL("Object rename failure", dbe::config::errors::parse(e).c_str());
    }
    // position it in its proper place
    bool nofail;
    t_object_map::iterator position;
    std::tie(position, nofail) = this_allobjects.emplace(*current_ptr, current_ptr);

    if (not nofail)
    {
      throw daq::dbe::dbcontroller_internal_cache_failure(ERS_HERE);
    }
    return position->second;
  }
  return objref;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
configobject::tref dbcontroller::get(dbe::cokey const & key)
{
  return dbcontroller::ref().lookup(key);
}

std::vector<configobject::tref> dbcontroller::gets(std::string const & cname,
    std::string const & query)
{
  std::vector<ConfigObject> database_objects = config::api::rwdacc::query_class(cname,
      query);
  std::vector<configobject::tref> result;

  for (ConfigObject const & keyin : database_objects)
  {
    try
    {
      result.push_back(dbcontroller::ref().lookup(
                         { keyin.UID(), keyin.class_name() }));
    }
    catch (daq::dbe::config_object_retrieval_result_is_null const & e)
    {
      // nothing needs be done to specifically handle this case, it just that in some
      // cases the underlying database does not contain initialized objects
    }
  }
  return result;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
configobject::tref dbcontroller::lookup(dbe::cokey const & key)
{
  // Find the object in the database
  t_object_map::iterator position = this_allobjects.find(key);

  if (position == this_allobjects.end())
  {
    // if it is not in the cache we need to add it
    ConfigObject toinsert = dbe::config::api::rwdacc::get_object(key.this_class,
        key.this_name);
    if (not toinsert.is_null())
    {
      return insert(static_cast<ConfigObject>(toinsert));
    }
  }
  else
  {
    return position->second;
  }

  throw daq::dbe::config_object_retrieval_result_is_null ( ERS_HERE,
                                                           key.this_name + "@" + key.this_class );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
configobject::tref dbcontroller::insert(ConfigObject const & obj)
{
  // Construct the oref to add in the internal map
  std::shared_ptr<configobject::oref> element(new configobject::oref(obj));

  bool object_inserted;
  t_object_map::iterator position;

  locker l(this_lock);
  std::tie(position, object_inserted) = this_allobjects.emplace(*element, element);

  if (not object_inserted)
  {
    // Object is already defined in the database and we replace the old reference with the new.
    // A case is if there have been external modification and we try to replay changes
    static_cast<ConfigObject &>(*position->second) = obj;
  }

  return position->second;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> configobject::gref<T> dbcontroller::remove(dbe::tref ref)
{
  configobject::gref<T> subgraph;

  {
    locker l(this_lock);
    // Record the associated subgraph
    if (not ref.is_null())
    {
      subgraph.record(ref);
    }

    // Need to remove from the internal map all candidates, they will be added at post processing
    for (T const & key : subgraph.this_remove_stack)
    {
      // Remove from the internal map
      this_allobjects.erase({ key.ref.UID(), key.ref.class_name() });
    }
  }

  // Delete the object from the database
  dbe::config::api::rwdacc::destroy_object(ref.ref());
  // Post-process the object recorded subgraph
  subgraph.post();

  return subgraph;
}
//------------------------------------------------------------------------------------------

}
// namespace inner

template<typename S>
inner::configobject::tref config_object_description<S>::ref() const
{
  if (this_referenced_object.is_null())
  {
    this_referenced_object = dbe::inner::dbcontroller::get(
      { this->this_name, this->this_class });
  }
  return this_referenced_object;
}

}			// namespace dbe
