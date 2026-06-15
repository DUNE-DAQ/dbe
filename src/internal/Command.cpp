// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

#include "dbe/Command.hpp"
#include "dbe/config_api_set.hpp" // checked
#include "dbe/config_api_graph.hpp" // checked
#include "dbe/confobject_desc.hpp" // checked
#include "dbe/confobject_extra.hpp"
#include "dbe/config_reference_copy.hpp"
#include "dbe/dbcontroller.hpp"
#include "dbe/messenger.hpp"

#include "conffwk/Configuration.hpp"

#include <QFileInfo>

#include <algorithm>

//------------------------------------------------------------------------------------------
dbe::actions::object::changerefs::changerefs (
  tref object, dunedaq::conffwk::relationship_t const & relation,
  std::vector<std::string> const & object_names_tolink, QUuid const & src,
  QUndoCommand * parent )
  : onobject ( object, parent, src ),
    this_relation ( relation ),
    this_current_neighbors (
      dbe::config::api::graph::linked::through::relation<std::vector<tref>> ( object, relation ) )
{
  try
  {
    for ( std::string const & oname : object_names_tolink )
    {
      try
      {
        this_target_neighbors.push_back ( inner::dbcontroller::get (
        { oname, relation.p_type } ) );
      }
      catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
      {
        // Actually there is no need to handle this error here ,
        // since the object will not be added to the result list of references,
        // and can be safely ignored
      }
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    failed();
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }

  setText (
    QObject::tr ( "Relation %1 of object %2 was updated." ).arg (
      this_relation.p_name.c_str() ).arg ( object.UID().c_str() ) );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::object::changerefs::redo()
{
  try
  {
    if ( redoable() )
    {
      failed();
      dbe::config::api::set::noactions::relation ( checkedref(), this_relation,
                                                   this_target_neighbors );
      confaccessor::ref().force_emit_object_changed ( source().toString(), checkedref() );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           ex.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, ex );
  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::object::changerefs::undo()
{
  try
  {
    if ( undoable() )
    {
      failed();
      dbe::config::api::set::noactions::relation ( checkedref(), this_relation,
                                                   this_current_neighbors );
      confaccessor::ref().force_emit_object_changed ( source().toString(), checkedref() );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    failed();
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           ex.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, ex );
  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::file::add::add ( std::string const & db_file,
                               std::string const & include_file,
                               QUuid const & src, QUndoCommand * parent )
  : state ( parent, src ),
    m_db_file ( db_file ),
    m_include_file ( include_file )
{
  setText (
    QObject::tr ( "Add Include File %1 to %2" ).arg (
      QFileInfo ( QString ( m_include_file.c_str() ) ).fileName() ).arg (
      QFileInfo ( QString ( m_db_file.c_str() ) ).fileName() ) );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::file::add::undo()
{
  try
  {
    if ( undoable() )
    {
      confaccessor::ref().removefile ( m_db_file, m_include_file );
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    failed();
    throw daq::dbe::DatabaseChangeNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::file::add::redo()
{
  try
  {
    if ( redoable() )
    {
      failed();
      confaccessor::ref().addfile ( m_db_file, m_include_file );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::DatabaseChangeNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::file::remove::remove ( std::string & db_file, std::string & include_file,
                                     QUuid const & src, QUndoCommand * parent )
  : state ( parent, src ),
    m_db_file ( db_file ),
    m_include_file ( include_file )
{
  setText (
    QObject::tr ( "Remove Include File %1 to %2" ).arg (
      QFileInfo ( QString ( m_include_file.c_str() ) ).fileName() ).arg (
      QFileInfo ( QString ( m_db_file.c_str() ) ).fileName() ) );
}

void dbe::actions::file::remove::undo()
{
  try
  {
    if ( undoable() )
    {
      failed();
      confaccessor::ref().addfile ( m_db_file, m_include_file );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::DatabaseChangeNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
}

void dbe::actions::file::remove::redo()
{
  try
  {
    if ( redoable() )
    {
      failed();
      confaccessor::ref().removefile ( m_db_file, m_include_file );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::DatabaseChangeNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::object::create::create ( dbe::t_config_object_preimage const & img,
                                       QUuid const & src, QUndoCommand * parent )
  : state ( parent, src ),
    this_object_key ( img )
{
  setText (
    QObject::tr ( "Creation of : Object %2@%1 " ).arg ( this_object_key.ref.this_class.c_str() )
    .arg ( this_object_key.ref.this_name.c_str() ) );
}

void dbe::actions::object::create::undo()
{
  try
  {
    if ( undoable() )
    {
      failed();
      // notify for the objects that have been actually removed
      inner::configobject::gref<config_object_aggregator>::config_action_notifiable deletion =
        [&] ( dref const obj )
      {
        confaccessor::ref().force_emit_object_deleted ( source().toString(), obj );
      };

      inner::dbcontroller::delete_object_request<config_object_aggregator> (
        inner::dbcontroller::get ( this_object_key.ref ), deletion );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, e );
  }
}

void dbe::actions::object::create::redo()
{
  try
  {
    if ( redoable() )
    {
      failed();
      inner::dbcontroller::create_object_request ( this_object_key );
      confaccessor::ref().force_emit_object_created (
        source().toString(), inner::dbcontroller::get ( this_object_key.ref ) );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::object::remove::remove ( tref anobject, QUuid const & uuid,
                                       QUndoCommand * parent )
  : onobject ( anobject, parent, uuid )
{
  setText (
    QObject::tr ( "Delete object of class %1 with ID %2" ).arg (
      ( this->checkedref().class_name() ).c_str() ).arg (
      ( this->checkedref().UID() ).c_str() ) );
}

dbe::actions::object::remove::~remove()
{
}

void dbe::actions::object::remove::undo()
{
  try
  {
    if ( undoable() )
    {
      failed();
      gref::config_action_notifiable creation = [&] ( dref obj )
      {
        confaccessor::ref().force_emit_object_created ( source().toString(), obj );
      };

      inner::dbcontroller::create_object_request ( this_remainder, creation );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           ex.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, ex );
  }

}

void dbe::actions::object::remove::redo()
{
  try
  {
    if ( redoable() and not checkedref().is_null() )
    {
      failed();
      // notify for the objects that have been actually removed
      gref::config_action_notifiable deletion = [&] ( dref const obj )
      {
        confaccessor::ref().force_emit_object_deleted ( source().toString(), obj );
      };

      this_remainder = inner::dbcontroller::delete_object_request <
                       decltype ( this_remainder ) ::t_extractor > ( checkedref(), deletion );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           ex.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, ex );
  }

}

//------------------------------------------------------------------------------------------
dbe::actions::object::rename::rename ( tref object, std::string const & Id,
                                       QUuid const & src,
                                       QUndoCommand * parent )
  : onobject ( object, parent, src ),
    oldname ( this->checkedref().UID() ),
    newname ( Id )
{
  setText (
    QObject::tr ( "Object changed ID from %1 to %2" ).arg ( this->checkedref().UID().c_str() )
    .arg ( newname.c_str() ) );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::object::rename::redo()
{
  try
  {
    if ( redoable() )
    {
      failed();
      dref object_before_change ( checkedref() );
      dbe::inner::dbcontroller::rename_object_request ( checkedref(), newname );

      confaccessor::ref().force_emit_object_renamed ( source().toString(), object_before_change );

      std::vector<tref> tonotify
      { this->checkedref().referenced_by ( "*", false ) };

      std::for_each ( tonotify.begin(), tonotify.end(), [&] ( tref const & x )
      {
        confaccessor::ref().force_emit_object_changed ( source().toString(), x );
      } );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           e.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, e );
  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::object::rename::undo()
{
  try
  {
    if ( undoable() )
    {
      failed();
      dref object_before_rename ( checkedref() );
      dbe::inner::dbcontroller::rename_object_request ( checkedref(), oldname );
      confaccessor::ref().force_emit_object_renamed ( source().toString(), object_before_rename );

      std::vector<tref> tonotify = checkedref().referenced_by ( "*", false );

      std::for_each ( tonotify.begin(), tonotify.end(), [&] ( tref const & x )
      {
        confaccessor::ref().force_emit_object_changed ( source().toString(), x );
      } );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           e.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, e );
  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::object::move::move ( tref object, std::string const & filename,
                                   QUuid const & src, QUndoCommand * parent )
  : onobject ( object, parent, src ),
    source_file ( this->checkedref().contained_in() ),
    destination_file ( filename )
{
  setText (
    QObject::tr ( "Object changed from file %1 to %2" ).arg ( source_file.c_str() ).arg (
      destination_file.c_str() ) );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::object::move::~move()
{
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::object::move::redo()
{
  try
  {
    if ( redoable() )
    {
      failed();
      inner::dbcontroller::move_object_request ( this->checkedref(), destination_file );
      emit confaccessor::ref().force_emit_object_changed ( source().toString(), this->checkedref() );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           e.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, e );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::actions::object::move::undo()
{
  try
  {
    if ( undoable() )
    {
      failed();
      inner::dbcontroller::move_object_request ( this->checkedref(), source_file );
      toggle();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, dbe::config::errors::parse ( e ) );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
  {
    FAIL ( "Operation did not complete because a lookup in the underlying database failed",
           e.what() );
    throw daq::dbe::ObjectChangeWasNotSuccessful ( ERS_HERE, e );
  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::onobject::onobject ( tref obj, QUndoCommand * parent, QUuid const & uuid )
  : dbe::actions::state ( parent, uuid ),
    this_object ( obj )
{
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::tref dbe::actions::onobject::checkedref() const
{
  return this_object.ref();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::actions::state::state ( QUndoCommand * parent, QUuid const & uuid )
  : QUndoCommand ( parent ),
    uuid ( uuid ),
    rewind ( true ),
    forward ( true ),
    invalid ( false )
{
}

void dbe::actions::state::toggle()
{
  invalid = not invalid;
  rewind = not rewind;
  forward = not forward;
}

void dbe::actions::state::failed() const
{
  invalid = true;
  rewind = false;
  forward = false;
}

bool dbe::actions::state::isvalid() const
{
  return not invalid;
}

void dbe::actions::state::setundoable ( bool state ) const
{
  rewind = state;
}

void dbe::actions::state::setredoable ( bool state ) const
{
  forward = state;
}

bool dbe::actions::state::undoable() const
{
  return rewind and not invalid;
}

bool dbe::actions::state::redoable() const
{
  return forward and not invalid;
}

QUuid const & dbe::actions::state::source() const
{
  return uuid;
}

bool dbe::actions::state::canRedo() const
{
  return redoable();
}

bool dbe::actions::state::canUndo() const
{
  return undoable();
}

void dbe::actions::state::reload() const
{
  // TODO evaluate: is this command that reloads the configuration object from the database really needed?
}

//------------------------------------------------------------------------------------------

