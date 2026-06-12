/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/// Including QT Headers
#include "dbe/Sorting.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/dbaccessor.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/messenger.hpp"

#include "dbe/change_attribute.hpp"
#include "dbe/change_enum.hpp"
#include "dbe/config_api.hpp"
#include "dbe/config_ui_info.hpp"
#include "dbe/version.hpp"

#include "conffwk/Configuration.hpp"

#include <QFileInfo>
#include <QStringList>
#include <QVector>

#include <unordered_set>
#include <functional>
#include <memory>
#include <vector>
#include <stack>

using namespace dunedaq::conffwk;

char const * const dbe_lib_internal_version = dbe_compiled_version;

//------------------------------------------------------------------------------------------
dbe::confaccessor & dbe::confaccessor::ref()
{
  static confaccessor me;
  return me;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::confaccessor::confaccessor()
  : CallId ( nullptr ),
    internal_change_stack ( new t_internal_changes_stack
                            { } ),
    sequenced_command_stack ( new QUndoStack() ),
    editconfig ( new datahandler() ),
    coreconfig ( nullptr ),
    this_change_enabled ( false ),
    this_total_objects ( 0 )
{}

dbe::confaccessor::~confaccessor()
{
  delete editconfig;
  delete coreconfig;
  {
    dbholder::t_lock l ( dbholder::database_lock );
    delete dbholder::database;
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool dbe::confaccessor::enabled()
{
  t_lock l ( ref().this_change_enabled_mutex );
  return ref().this_change_enabled;
}

void dbe::confaccessor::setenabled()
{
  t_lock l ( this_change_enabled_mutex );
  this_change_enabled = true;
}

void dbe::confaccessor::setdisabled()
{
  t_lock l ( this_change_enabled_mutex );
  this_change_enabled = false;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::confaccessor::force_emit_object_created ( QString const & src, dref const obj )
{
  t_lock l ( force_mut );
  emit object_created ( src, obj );
}

void dbe::confaccessor::force_emit_object_renamed ( QString const & src, dref const obj )
{
  t_lock l ( force_mut );
  emit object_renamed ( src, obj );
}

void dbe::confaccessor::force_emit_object_deleted ( QString const & src, dref const obj )
{
  t_lock l ( force_mut );
  emit object_deleted ( src, obj );
}

void dbe::confaccessor::force_emit_object_changed ( QString const & src, dref const obj )
{
  t_lock l ( force_mut );
  emit object_changed ( src, obj );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::confaccessor::GetFileCache ( QList<QStringList> & FileCache )
{
  IncludedFileCache = FileCache;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QString dbe::confaccessor::dbfullname()
{
  return ref().this_dblocation;
}

QString dbe::confaccessor::db_implementation_name()
{
  return ref().this_resource_location;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
cptr<dbe::datahandler> dbe::confaccessor::gethandler()
{
  static cptr<datahandler> r ( ref().editconfig );
  return r;
}

cptr<dbe::ui::config::info> dbe::confaccessor::guiconfig()
{
  static cptr<dbe::ui::config::info> r ( confaccessor::ref().coreconfig );
  return r;
}

dbe::confaccessor::t_undo_stack_cptr dbe::confaccessor::get_commands()
{
  static t_undo_stack_cptr rp ( ref().sequenced_command_stack );
  return rp;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
int dbe::confaccessor::get_total_objects()
{
  return ref().this_total_objects;
}

void dbe::confaccessor::increase_total_objects ( int const i )
{
  ref().this_total_objects += i;
}

void dbe::confaccessor::set_total_objects ( int const i )
{
  ref().this_total_objects = i;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------

template<>
void dbe::confaccessor::set_dbinfo<dbe::dbinfo::oks> ( QString const & location )
{
  static QString const implementation ( "oksconflibs" );

  this_dblocation = location;
  this_resource_location = implementation + ":" + location;

  setenabled();
}

template<>
void dbe::confaccessor::set_dbinfo<dbe::dbinfo::roks> ( QString const & location )
{
  this_dblocation = location;
  static QString const implementation ( "roksconflibs" );
  this_resource_location = implementation + ":" + location;
  t_lock l ( this_change_enabled_mutex );
  this_change_enabled = true;
}

template<>
void dbe::confaccessor::set_dbinfo<dbe::dbinfo::rdb> ( QString const & location )
{
  static QString const implementation ( "rdbconfig" );
  this_resource_location = implementation + ":" + location;
  this_dblocation = this_resource_location;
  t_lock l ( this_change_enabled_mutex );
  this_change_enabled = true;
}

void dbe::confaccessor::setdbinfo ( QString const & location, dbinfo const itype )
{
  switch ( itype )
  {

  case dbinfo::oks:
    ref().set_dbinfo<dbinfo::oks> ( location );
    break;

  case dbinfo::roks:
    ref().set_dbinfo<dbinfo::roks> ( location );
    break;

  case dbinfo::rdb:
    ref().set_dbinfo<dbinfo::rdb> ( location );
    break;

  default:
    ref().setenabled();
    break;
  }
}

void dbe::confaccessor::setdblocation ( QString const & Implementation )
{
  ref().this_resource_location = Implementation;
}

bool dbe::confaccessor::is_database_loaded()
{
  return dbaccessor::dbptr().get() != nullptr;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool dbe::confaccessor::check_file_rw ( QString const & fn )
{
  try
  {
    if ( not fn.endsWith ( ".schema.xml" ) )
    {
      return dbaccessor::dbptr()->is_writable ( fn.toStdString() );
    }
  }
  catch ( dunedaq::conffwk::Generic const & ex )
  {
    ERROR ( "Not possible to operate on file", dbe::config::errors::parse ( ex ),
            "\n\nCheck filename:", fn.toStdString() );
  }

  return false;
}

//------------------------------------------------------------------------------------------
void dbe::confaccessor::clear_commands()
{
  if ( ref().get_commands().get() )
  {
      ref().get_commands()->clear();
      t_internal_changes_stack cleared{ };
      ref().get_internal_change_stack()->swap ( cleared );
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::confaccessor::abort()
{
  dbaccessor::dbptr()->abort();
}


std::list<std::string> dbe::confaccessor::uncommitted_files() {
    std::list<std::string> dbs;
    dbaccessor::dbptr()->get_updated_dbs(dbs);
    return dbs;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool dbe::confaccessor::load(bool subscribeToChanges)
{
  try
  {
    dbholder::t_lock l ( dbholder::database_lock );

    if ( dbholder::database == nullptr )
    {
      dbholder::database = new Configuration ( ref().this_resource_location.toStdString() );

      if ( subscribeToChanges == true ) {
          ref().CallId = dbholder::database->subscribe (
                           ConfigurationSubscriptionCriteria(),
                           &confaccessor::CallbackFunction,
                           reinterpret_cast<void *> ( dbholder::database ) );
          }
    }
    else
    {
      dbe::inner::dbcontroller::flush();
      delete dbholder::database;
      dbholder::database = new Configuration ( ref().this_resource_location.toStdString() );
    }

    dbholder::database->prefetch_all_data();

    dbholder::database_concurrent_ptr = cptr<Configuration> ( dbholder::database );
    return true;
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    FAIL ( "Database loading failed", dbe::config::errors::parse ( e ).c_str() );
    return false;
  }

}

std::list<std::string> dbe::confaccessor::save ( const QString & CommitMessage )
{
  try
  {
    std::list<std::string> tobecommited{};

    if ( ref().is_database_loaded() )
    {
      dbaccessor::dbptr()->get_updated_dbs ( tobecommited );
      dbaccessor::dbptr()->commit ( CommitMessage.toStdString() );
      confaccessor::ref().db_committed(tobecommited, CommitMessage.toStdString());
    }

    return tobecommited;
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    // throw daq::dbe::CouldNotCommitChanges ( ERS_HERE, dbe::config::errors::parse ( e ) );
    throw daq::dbe::CouldNotCommitChanges ( ERS_HERE, "confaccessor", e );
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::confaccessor::init()
{
  QString DUNEDAQ_DB_PATH = getenv ( "DUNEDAQ_DB_PATH" );
  QString GUI_DATA = getenv ( "OKS_GUI_INIT_DATA" );

  QStringList CONFIG_DATABASE = GUI_DATA.split ( ":", Qt::SkipEmptyParts );
  QStringList DUNEDAQ_DB_PATH_SPLIT = DUNEDAQ_DB_PATH.split ( ":", Qt::SkipEmptyParts );

  // We need to read the current configuration ( to retrieve parameters affecting dbe )

  std::vector<std::string> full_path_names;

  for ( QString & FileName : CONFIG_DATABASE )
  {

    for ( QString & PATH : DUNEDAQ_DB_PATH_SPLIT )
    {
      QFileInfo CheckFile ( PATH + "/" + FileName );

      if ( CheckFile.exists() and CheckFile.isFile() )
      {
        QString fullpathname = PATH + "/" + FileName;
        full_path_names.push_back ( fullpathname.toStdString() );
      }
    }
  }

  confaccessor::ref().coreconfig = new ui::config::info ( full_path_names );

}

QList<QStringList> dbe::confaccessor::GetIncludedFileCache() const
{
  return IncludedFileCache;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::confaccessor::addfile ( std::string const & db, std::string const & fn )
{
  if ( not enabled() )
  {
    throw daq::dbe::ChangeNotAllowed ( ERS_HERE );
  }

  dbaccessor::dbptr()->add_include ( db, fn );
  emit IncludeFileDone();
}

void dbe::confaccessor::removefile ( std::string const & db, std::string const & fn )
{
  if ( not enabled() )
  {
    throw daq::dbe::ChangeNotAllowed ( ERS_HERE );
  }

  dbaccessor::dbptr()->remove_include ( db, fn );
  emit RemoveFileDone();
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::confaccessor::CallbackFunction ( std::vector<ConfigurationChange *> const &
                                           changes,
                                           void * parameter )
{
  dbe::confaccessor::ref().docallback ( changes, parameter );
}

void dbe::confaccessor::docallback ( std::vector<ConfigurationChange *> const & changes,
                                     void * parameter )
{
  Q_UNUSED ( parameter );

  t_lock l ( mut_changes );
  external_change_stack.clear();

  for ( ConfigurationChange * Change : changes )
  {
      const auto& cl = Change->get_class_name();
      const auto& mod = Change->get_modified_objs();
      const auto& cre = Change->get_created_objs();
      const auto& rem = Change->get_removed_objs();

      external_change_stack.push_back ( { cl, mod, cre, rem } );
   }

      emit ExternalChangesAccepted();
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::confaccessor::unsubscribe()
{
  if ( dbaccessor::dbptr().get() && ref().CallId != nullptr )
  {
    dbaccessor::dbptr()->unsubscribe ( ref().CallId );
  }
}

dbe::confaccessor::t_internal_changes_stack_cptr
dbe::confaccessor::get_internal_change_stack()
{
  static t_internal_changes_stack_cptr rp ( ref().internal_change_stack.get() );
  return rp;
}

//------------------------------------------------------------------------------------------

