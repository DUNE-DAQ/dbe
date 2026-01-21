#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/// Including QT Headers
#include <QMap>
#include <QSet>
#include <QString>
#include <QMessageBox>
#include <QMainWindow>

#include <QTabWidget>
#include <QDragEnterEvent>
#include <QDropEvent>

/// Including UI Files
#include "ui_DBE.h"

/// Including DBE
#include "dbe/FileModel.hpp"
#include "dbe/CustomDelegate.hpp"
//#include "dbe/OracleWidget.hpp"
#include "dbe/TableTab.hpp"
#include "dbe/tree.hpp"
#include "dbe/table.hpp"
#include "dbe/tableselection.hpp"
#include "dbe/treeselection.hpp"
#include "dbe/subtreeproxy.hpp"

// /// Including IPC
//#include <ipc/partition.h>
//#include <rdb/rdb.hh>

#include <atomic>

namespace dbe
{
namespace Ui
{
class MainWindow;
}  // namespace Ui

class MainWindow:
  public QMainWindow,
  private dbe::Ui::MainWindow
{
  Q_OBJECT
public:
  explicit MainWindow ( QMap<QString, QString> const & CommandLine, QWidget * parent =
                          nullptr );

  static MainWindow * findthis();

  [[nodiscard]] bool check_ready() const;

  [[nodiscard]] cptr<dbe::CustomTreeView> get_view() const;

  [[nodiscard]] QString find_db_repository_dir();

private:
  //typedef QMap<QString, bool> RDBMap;

  QSet<QString> allFiles;

  bool m_batch_change_in_progress;
  FileModel * this_files;
  QSortFilterProxyModel this_filesort;

  dbe::models::tree * this_classes;
  models::treeselection * this_treefilter;

  //OracleWidget * this_oraclewidget;

  std::atomic<bool> isArchivedConf;

  std::vector<dbe::tref> ProcessQuery ( QString const & );

  void closeEvent ( QCloseEvent * event );

  bool eventFilter ( QObject * Target, QEvent * Event );

  bool check_close();

  void init();

  void attach();

  bool dbopen ( QString const &, dbinfo const & );

  bool dbload();

  bool dbreload();

  void setinternals();

  void build_class_tree_model();
  void build_table_model();
public:
  void build_file_model();
private:
  void load_settings ( bool LoadSettings = false );

  void WriteSettings();
  void argsparse ( QMap<QString, QString> const & );
  //void init_rdb_menu();
  //void lookForRDBServers ( const IPCPartition & p );
  void init_tabs();

  void edit_object_at ( const QModelIndex & Index );

  void update_total_objects();
  void display_message_box(const QString& title, const QString& msg,
                           const QMessageBox::Icon& icon);
private slots:
  void slot_create_newdb();
  void slot_open_database_from_file();
  void slot_load_db_from_create_widget ( const QString & );

  //void slot_rdb_selected ( QAction * );
  //void slot_rdb_found (const QString& p, const RDBMap& rdbs);

  //void slot_oracle_prepare();
  //void slot_load_oracle ( const QString & );

  bool slot_commit_database ( bool Exit = false );

  void slot_abort_changes();
  void slot_abort_external_changes();

  void slot_launch_object_editor ( tref );
  void slot_edit_object_from_class_view ( QModelIndex const & );

  void slot_fetch_data ( treenode const * );

  void slot_launch_batchchange();
  void slot_launch_batchchange_on_table();

  void LoadDefaultSetting();

  void slot_filter_query();
  void slot_filter_textchange ( const QString & );
  void slot_filter_table_textchange ( const QString & );

  void slot_tree_reset();
  void slot_model_rebuild();

  void slot_whatisthis();

  void slot_show_information_about_dbe();
  void slot_show_userguide();
  void slot_show_userchanges();

  void slot_process_externalchanges();
  void slot_undo_allchanges();

  void slot_toggle_casesensitive_for_treeview ( bool );
  void slot_add_tab();
  void slot_remove_tab ( int i );

  void slot_toggle_commit_button();

  void slot_update_committed_files(const std::list<std::string>&, const std::string&);

  void slot_loaded_db_file ( QString );

public slots:
  void slot_batch_change_start();
  void slot_batch_change_stop(const QList<QPair<QString, QString>>&);

  void slot_information_message ( QString const, QString const );
  void slot_warning_message ( QString const, QString const );
  void slot_error_message ( QString const, QString const );

signals:
  //void signal_rdb_found (const QString& p, const RDBMap& rdbs);
  void signal_batch_change_stopped(const QList<QPair<QString, QString>>&);
  void signal_db_loaded();
  void signal_externalchanges_processed();
  void signal_new_file_model();
};

}  // namespace dbe

#endif // MAINWINDOW_H
