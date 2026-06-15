/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/confaccessor.h to include/dbe/confaccessor.hpp).
 */

#ifndef CONFACCESSOR_H
#define CONFACCESSOR_H

#include "dbe/datahandler.hpp"
#include "dbe/config_direct_access.hpp"
#include "dbe/cptr.hpp"
#include "dbe/dbinfo.hpp"

#include "conffwk/Configuration.hpp"
#include "conffwk/Change.hpp"

#include <QUndoCommand>
#include <QObject>

#include <string>
#include <stack>

extern char const * const dbe_lib_internal_version;

namespace dbe
{
//------------------------------------------------------------------------------------------
struct config_internal_change
{
  enum modification
  {
    CREATED,
    DELETED,
    MODIFIED,
    RENAMED,
    MOVED,
    FILE_INCLUDED,
    FILE_DELETED
  };

  /// Type of modification
  modification request;
  std::string description;

  /// Object Identification
  std::string uid;
  std::string classname;
  std::string filename;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
struct ConfigWrapperExternalChange
{
  std::string ClassName;
  std::vector<std::string> ModifiedObjects;
  std::vector<std::string> CreatedObjects;
  std::vector<std::string> DeletedObjects;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class confaccessor:
  public QObject
{
  Q_OBJECT
public:
  typedef config_internal_change t_internal_change;
  typedef std::stack<t_internal_change, std::vector<t_internal_change>>
                                                                     t_internal_changes_stack;
  typedef cptr<t_internal_changes_stack> t_internal_changes_stack_cptr;

  typedef QUndoStack t_undo_stack;
  typedef cptr<t_undo_stack> t_undo_stack_cptr;

  ~confaccessor();
  /**
   * Retrieve a reference to this singleton
   *
   * @return a reference to this
   */
  static confaccessor & ref();

  /**
   * Must be called to read default OKS configuration type information
   */
  static void init();

  /**
   * Load the database from the already specified source
   *
   * @return true when the database has been loaded
   */
  static bool load(bool subscribeToChanges = true);

  /**
   * Abort all database changes
   */
  static void abort();

  /**
   * Check if the underlying database is actually loaded
   *
   * @return true when the database is loaded
   */
  static bool is_database_loaded();

  /**
   * Check if changes are allowed
   *
   * @return true in case changes are allowed
   */
  static bool enabled();

  /**
   * Commit changes to the database
   *
   * @param CommitMessage to enter for the database
   */
  static std::list<std::string> save ( QString const & );

  /**
   * Get a thread safe pointer to the data handler
   *
   * @return returns a thread safe pointer
   */
  static cptr<datahandler> gethandler();

  /**
   * Access the default oks configuration type information
   * @return
   */
  static cptr<ui::config::info> guiconfig();

  /**
   * Get a thread safe pointer to the internal command stack
   * @return
   */
  static t_undo_stack_cptr get_commands();

  /**
   * Clear internal changes commands
   */
  static void clear_commands();

  /**
   * Get a thread safe pointer to the internal change stack
   *
   * @return
   */
  static t_internal_changes_stack_cptr get_internal_change_stack();

  /**
   * Force emit a creation signal from this object
   */
  void force_emit_object_created ( QString const &, dref const );

  /**
   * Force emit that an object has been deleted
   *
   * Parameters must be passed by value, because this signal may be emitted from other
   * than QThreads threads, which may cause a temporary to die prematurely in the other
   * thread.
   *
   * @param UUID string representation of the object causing the creation
   * @param a description of the object having be created
   */
  void force_emit_object_deleted ( QString const &, dref const );

  /**
   * Force emit that an object has been renamed
   *
   * @param UUID string representation of the object causing the creation
   * @param a description of the object having be changed, actually contains the original
   * name
   */
  void force_emit_object_renamed ( QString const &, dref const );

  /**
   * Force emit an object changed signal from this object
   *
   * @param UUID string representation of the object causing the creation
   * @param the object that was changed
   */
  void force_emit_object_changed ( QString const &, dref const );


  static void setdbinfo ( QString const & location, dbinfo const itype = dbinfo::oks);

  static void setdblocation ( const QString & Implementation );

  static bool check_file_rw ( const QString & FileName );

  QList<QStringList> GetIncludedFileCache() const;

  void addfile ( std::string const & db, std::string const & fn );
  void removefile ( std::string const & db, std::string const & fn );

  static QString dbfullname();
  static QString db_implementation_name();

  static void CallbackFunction ( const std::vector<dunedaq::conffwk::ConfigurationChange *> & changes,
                                 void * parameter );

  static void unsubscribe();

  static std::list<std::string> uncommitted_files();

  static int get_total_objects();
  static void set_total_objects ( int const i );
  static void increase_total_objects ( int const i );

signals:

  void object_created ( QString const & source, dref const & obj );
  void object_renamed ( QString const & source, dref const & obj );
  void object_changed ( QString const & source, dref const & obj );
  void object_deleted ( QString const & source, dref const & obj );

  void db_committed(const std::list<std::string>& files, const std::string& msg);

  void IncludeFileDone();
  void RemoveFileDone();

  void ResetTree();

  void ExternalChangesDetected();
  void ExternalChangesAccepted();

public slots:
  void GetFileCache ( QList<QStringList> & FileCache );

private:
  typedef std::mutex t_mutex;
  typedef std::lock_guard<t_mutex> t_lock;

  typedef std::vector<ConfigWrapperExternalChange> t_external_changes_stack;

  QString this_dblocation;
  QString this_resource_location;

  dunedaq::conffwk::Configuration::CallbackId CallId;

  std::shared_ptr<t_internal_changes_stack> internal_change_stack;
  t_external_changes_stack external_change_stack;
  t_undo_stack * sequenced_command_stack;

  datahandler * editconfig;
  ui::config::info * coreconfig;

  QList<QStringList> IncludedFileCache;

  mutable t_mutex this_change_enabled_mutex;
  mutable t_mutex mut_changes;
  mutable t_mutex force_mut;

  bool this_change_enabled;

  int this_total_objects;

  void setenabled();
  void setdisabled();

  /**
   * Set database information
   *
   * @param p the location string
   */
  template<dbinfo N> void set_dbinfo ( QString const & p );

  void docallback ( std::vector<dunedaq::conffwk::ConfigurationChange *> const & changes, void * parameter );

  confaccessor();
  confaccessor ( confaccessor const & ) = delete;
  confaccessor & operator= ( confaccessor const & ) = delete;

  friend class dbaccessor;
};
//------------------------------------------------------------------------------------------

template<> void confaccessor::set_dbinfo<dbinfo::oks> ( QString const & p );
template<> void confaccessor::set_dbinfo<dbinfo::roks> ( QString const & p );
template<> void confaccessor::set_dbinfo<dbinfo::rdb> ( QString const & p );

extern template void confaccessor::set_dbinfo<dbinfo::oks> ( QString const & p );
extern template void confaccessor::set_dbinfo<dbinfo::roks> ( QString const & p );
extern template void confaccessor::set_dbinfo<dbinfo::rdb> ( QString const & p );

} // end namespace dbe

#endif // CONFACCESSOR_H
