#include "dbe/MainWindow.hpp"
#include "dbe/ObjectEditor.hpp"
#include "dbe/ObjectCreator.hpp"
#include "dbe/BatchChangeWidget.hpp"
#include "dbe/BuildingBlockEditors.hpp"
#include "dbe/CommitDialog.hpp"
#include "dbe/StyleUtility.hpp"
#include "dbe/CreateDatabaseWidget.hpp"
#include "dbe/Command.hpp"
#include "dbe/messenger.hpp"
#include "dbe/messenger_proxy.hpp"
#include "dbe/config_api.hpp"
#include "dbe/config_api_version.hpp"
#include "dbe/subtreeproxy.hpp"
#include "dbe/treenode.hpp"
#include "dbe/version.hpp"
#include "dbe/MyApplication.hpp"

#include "logging/Logging.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTime>
#include <QUndoStack>
#include <QSettings>
#include <QCloseEvent>
#include <QWhatsThis>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QItemDelegate>

#include <future>
#include <thread>

#include <boost/scope_exit.hpp>


namespace {
    // This allows to select data in the cells but to not modify them
    class DummyEditorDelegate : public QItemDelegate {
        public:
            void setModelData(QWidget * /* editor */, QAbstractItemModel * /* model */, const QModelIndex & /* index */) const override {}
    };
}

dbe::MainWindow::MainWindow ( QMap<QString, QString> const & cmdargs, QWidget * parent )
  : QMainWindow ( parent ),
    m_batch_change_in_progress ( false ),
    this_files ( nullptr ),
    this_filesort ( new QSortFilterProxyModel ( this ) ),
    this_classes ( nullptr ),
    this_treefilter ( nullptr ),
    isArchivedConf ( false )
{
  //qRegisterMetaType<RDBMap>("RDBMap");

  /// Setting up ui
  setupUi ( this );

  /// Initial Settings
  init();
  init_tabs();
  //init_rdb_menu();

  /// Setting up application controller
  attach();

  /// Reading Applications Settings/CommandLine
  load_settings ( false );
  argsparse ( cmdargs );

  if (isArchivedConf == true) {
      OpenDB->setEnabled(false);
      //OpenOracleDB->setEnabled(false);
      //ConnectToRdb->setEnabled(false);
      CreateDatabase->setEnabled(false);
      Commit->setEnabled(false);

      QMessageBox::information(this,
                               "DBE",
                               QString("The configuration is opened in archival/detached mode.")
                                      .append("\nYou can browse or modify objects, but changes cannot be saved or commited."));
  }

  UndoView->show();

}

void dbe::MainWindow::init_tabs()
{
  tableholder->addTab ( new TableTab ( tableholder ), "Table View" );
  tableholder->removeTab ( 0 );

  QPushButton * addtab_button = new QPushButton ( "+" );
  tableholder->setCornerWidget ( addtab_button, Qt::TopLeftCorner );
  connect ( addtab_button, SIGNAL ( clicked() ), this, SLOT ( slot_add_tab() ) );

  tableholder->setTabsClosable ( true );
  connect ( tableholder, SIGNAL ( tabCloseRequested ( int ) ), this,
            SLOT ( slot_remove_tab ( int ) ) );
}

void dbe::MainWindow::slot_add_tab()
{
  tableholder->addTab ( new TableTab ( tableholder ), "Table View" );
  tableholder->setCurrentIndex ( tableholder->count()-1 );
  tableholder->show();
}

void dbe::MainWindow::slot_remove_tab ( int i )
{
  if ( i == -1 || ( ( tableholder->count() == 1 ) && i == 0 ) )
  {
    return;
  }

  QWidget * Widget = tableholder->widget ( i );

  tableholder->removeTab ( i );

  delete Widget;

  Widget = nullptr;
}


void dbe::MainWindow::init()
{
  /// Window Settings
  setWindowTitle ( "DUNE DAQ Configuration Database Editor (DBE)" );
  /// Table Settings
  UndoView->setStack ( confaccessor::get_commands().get() );
  SearchLineTable->hide();
  SearchLineTable->setClearButtonEnabled(true);
  SearchTreeLine->setClearButtonEnabled(true);
  CaseSensitiveCheckBoxTable->hide();
  tableholder->removeTab ( 1 );

  /// Menus Settings
  HelpMenu->setEnabled ( false );  // Until help is updated to be useful!!!

  /// Commands Settings
  Commit->setEnabled ( false );
  UndoAction->setEnabled ( true );
  RedoAction->setEnabled ( true );

  /// Search Box Settings
  SearchBox->setFocusPolicy ( Qt::ClickFocus );

  /// What is this
  TreeView->setWhatsThis ( "This view shows the classes and objects of the database" );
  FileView->setWhatsThis ( "This view shows the file structure of the database" );
  UndoView->setWhatsThis ( "This view shows the commands in the Undo Command stack" );

  CommittedTable->setHorizontalHeaderLabels(QStringList() << "File" << "Comment" << "Date");
  CommittedTable->setAlternatingRowColors(true);
  CommittedTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  CommittedTable->horizontalHeader()->setDefaultSectionSize(250);
  CommittedTable->setWordWrap(true);
  CommittedTable->setTextElideMode(Qt::ElideRight);
  CommittedTable->setItemDelegate(new DummyEditorDelegate());

  // Make Files the current tab 
  InfoWidget->setCurrentIndex (0);


  /// Color Management
  StyleUtility::InitColorManagement();
}

void dbe::MainWindow::attach()
{
  connect ( OpenDB, SIGNAL ( triggered() ), this, SLOT ( slot_open_database_from_file() ) );
  connect ( Commit, SIGNAL ( triggered() ), this, SLOT ( slot_commit_database() ) );
  connect ( Exit, SIGNAL ( triggered() ), this, SLOT ( close() ) );
  connect ( UndoAction, SIGNAL ( triggered() ), UndoView->stack(), SLOT ( undo() ) );
  connect ( RedoAction, SIGNAL ( triggered() ), UndoView->stack(), SLOT ( redo() ) );
  connect ( UndoAll, SIGNAL ( triggered() ), this, SLOT ( slot_undo_allchanges() ) );
  connect ( BatchChange, SIGNAL ( triggered() ), this, SLOT ( slot_launch_batchchange() ) );
  connect ( BatchChangeTable, SIGNAL ( triggered() ), this,
            SLOT ( slot_launch_batchchange_on_table() ) );

  connect ( DisplayClassView, SIGNAL ( triggered ( bool ) ), TreeDockWidget,
            SLOT ( setVisible ( bool ) ) );
  connect ( DisplayTableView, SIGNAL ( triggered ( bool ) ), TableGroupBox,
            SLOT ( setVisible ( bool ) ) );
  connect ( DisplayMessages, SIGNAL ( triggered ( bool ) ), InfoDockWidget,
            SLOT ( setVisible ( bool ) ) );
  connect ( DisplayToolbar, SIGNAL ( triggered ( bool ) ), MainToolBar,
            SLOT ( setVisible ( bool ) ) );

  connect ( TreeDockWidget, SIGNAL ( visibilityChanged ( bool ) ), DisplayTableView,
            SLOT ( setChecked ( bool ) ) );
  connect ( InfoDockWidget , SIGNAL ( visibilityChanged ( bool ) ), DisplayMessages,
            SLOT ( setChecked ( bool ) ) );
  connect ( MainToolBar , SIGNAL ( visibilityChanged ( bool ) ), DisplayToolbar,
            SLOT ( setChecked ( bool ) ) );


  connect ( LoadDefaultSettings, SIGNAL ( triggered() ), this,
            SLOT ( LoadDefaultSetting() ) );
  connect ( CreateDatabase, SIGNAL ( triggered() ), this, SLOT ( slot_create_newdb() ) );
  //connect ( OpenOracleDB, SIGNAL ( triggered() ), this, SLOT ( slot_oracle_prepare() ) );

  connect ( WhatThisAction, SIGNAL ( triggered() ), this, SLOT ( slot_whatisthis() ) );
  connect ( UserGuide, SIGNAL ( triggered() ), this, SLOT ( slot_show_userguide() ) );
  connect ( UserChanges, SIGNAL ( triggered() ), this, SLOT ( slot_show_userchanges() ) );

  connect ( TreeView, SIGNAL ( activated ( QModelIndex ) ), this,
            SLOT ( slot_edit_object_from_class_view ( QModelIndex ) ) );

  connect( &confaccessor::ref(), SIGNAL(db_committed(const std::list<std::string>&, const std::string&)), this,
           SLOT(slot_update_committed_files(const std::list<std::string>&, const std::string&)));

  connect ( confaccessor::gethandler().get(), SIGNAL ( FetchMoreData ( const treenode * ) ),
            this,
            SLOT ( slot_fetch_data ( const treenode * ) ) );

  connect( &confaccessor::ref(), SIGNAL(object_created(QString, dref)), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(object_renamed(QString, dref)), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(object_changed(QString, dref)), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(object_deleted(QString, dref)), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(db_committed(const std::list<std::string>&, const std::string&)), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(IncludeFileDone()), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(RemoveFileDone()), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(ExternalChangesDetected()), this,
           SLOT(slot_toggle_commit_button()));
  connect( &confaccessor::ref(), SIGNAL(ExternalChangesAccepted()), this,
           SLOT(slot_toggle_commit_button()));
  connect( this, SIGNAL(signal_batch_change_stopped(const QList<QPair<QString, QString>>&)), this,
           SLOT(slot_toggle_commit_button()));

  connect ( &confaccessor::ref(), SIGNAL ( IncludeFileDone() ), this,
            SLOT ( slot_model_rebuild() ) );
  connect ( &confaccessor::ref(), SIGNAL ( RemoveFileDone() ), this,
            SLOT ( slot_model_rebuild() ) );
  connect ( &confaccessor::ref(), SIGNAL ( ExternalChangesAccepted() ), this,
            SLOT ( slot_process_externalchanges() ) );


  connect ( SearchBox, SIGNAL ( currentIndexChanged(int) ), this,
            SLOT ( slot_filter_query() ) );
  connect ( SearchTreeLine, SIGNAL ( textChanged ( const QString & ) ), this,
            SLOT ( slot_filter_textchange ( const QString & ) ) );
  connect ( SearchTreeLine, SIGNAL ( textEdited ( const QString & ) ), this,
            SLOT ( slot_filter_query() ) );
  connect ( SearchTreeLine, SIGNAL ( returnPressed() ), this, SLOT ( slot_filter_query() ) );
  connect ( SearchLineTable, SIGNAL ( textChanged ( const QString & ) ), this,
            SLOT ( slot_filter_table_textchange ( const QString & ) ) );
  connect ( CaseSensitiveCheckBoxTree, SIGNAL ( clicked ( bool ) ), this,
            SLOT ( slot_toggle_casesensitive_for_treeview ( bool ) ) );
  //connect ( ConnectToRdb, SIGNAL ( triggered ( QAction * ) ), this,
  //        SLOT ( slot_rdb_selected ( QAction * ) ) );

  connect ( information_about_dbe, SIGNAL ( triggered() ), this,
            SLOT ( slot_show_information_about_dbe() ) );

  // Connect to signals from the messenger system

  connect ( &dbe::interface::messenger_proxy::ref(),
            SIGNAL ( signal_debug ( QString const, QString const ) ), this,
            SLOT ( slot_debuginfo_message ( QString , QString ) ), Qt::QueuedConnection );

  connect ( &dbe::interface::messenger_proxy::ref(),
            SIGNAL ( signal_info ( QString const, QString const ) ), this,
            SLOT ( slot_information_message ( QString , QString ) ), Qt::QueuedConnection );

  connect ( &dbe::interface::messenger_proxy::ref(),
            SIGNAL ( signal_note ( QString const, QString const ) ), this,
            SLOT ( slot_notice_message ( QString , QString ) ), Qt::QueuedConnection );

  connect ( &dbe::interface::messenger_proxy::ref(),
            SIGNAL ( signal_warn ( QString const, QString const ) ), this,
            SLOT ( slot_warning_message ( QString , QString ) ), Qt::QueuedConnection );

  connect ( &dbe::interface::messenger_proxy::ref(),
            SIGNAL ( signal_error ( QString const, QString const ) ), this,
            SLOT ( slot_error_message ( QString, QString ) ), Qt::QueuedConnection );

  connect ( &dbe::interface::messenger_proxy::ref(),
            SIGNAL ( signal_fail ( QString const, QString const ) ), this,
            SLOT ( slot_failure_message ( QString , QString ) ), Qt::QueuedConnection );

  // connect ( this, SIGNAL ( signal_rdb_found(const QString&, const RDBMap& ) ),
  //           this, SLOT ( slot_rdb_found(const QString&, const RDBMap&) ), Qt::AutoConnection );
}

void dbe::MainWindow::build_class_tree_model()
{
  QStringList Headers
  { "Class Name", "# Objects" };

  if ( this_classes != nullptr )
  {
    delete this_classes;
    delete this_treefilter;
  }
  /// Creating new Main Model
  this_classes = new dbe::models::tree ( Headers );
  /// Resetting attached models
  this_treefilter = new models::treeselection();
  this_treefilter->setFilterRegExp ( "" );

  connect ( this_classes, SIGNAL ( ObjectFile ( QString ) ),
            this, SLOT ( slot_loaded_db_file ( QString ) ) );

  this_treefilter->setDynamicSortFilter ( true );
  this_treefilter->setSourceModel ( this_classes );
  slot_toggle_casesensitive_for_treeview ( true );
  TreeView->setModel ( this_treefilter );
  TreeView->setSortingEnabled ( true );
  TreeView->resizeColumnToContents ( 0 );
  TreeView->resizeColumnToContents ( 1 );

  connect ( HideCheckBox, SIGNAL ( toggled ( bool ) ), this_treefilter,
            SLOT ( ToggleEmptyClasses ( bool ) ) );

  connect ( ShowDerivedObjects, SIGNAL ( toggled ( bool ) ), this_classes,
            SLOT ( ToggleAbstractClassesSelectable ( bool ) ) );

  update_total_objects();
}

void dbe::MainWindow::build_table_model()
{
  /// Displaying table widgets
  SearchLineTable->clear();
  SearchLineTable->show();
  SearchLineTable->setProperty ( "placeholderText", QVariant ( QString ( "Table Filter" ) ) );
  CaseSensitiveCheckBoxTable->show();
}

void dbe::MainWindow::edit_object_at ( const QModelIndex & Index )
{
  treenode * tree_node = this_classes->getnode ( Index );

  /*
   * If an object node is linked to this index then launch the object editor
   * else build a table for the class , showing all objects
   */

  if ( dynamic_cast<ObjectNode *> ( tree_node ) )
  {
    ObjectNode * NodeObject = dynamic_cast<ObjectNode *> ( tree_node );
    tref ObjectToBeEdited = NodeObject->GetObject();
    slot_launch_object_editor ( ObjectToBeEdited );
  }
  else
  {
    // Class node
    QString const cname = tree_node->GetData ( 0 ).toString();
    dunedaq::conffwk::class_t cinfo = dbe::config::api::info::onclass::definition (
                                   cname.toStdString(),
                                   false );

    if ( not cinfo.p_abstract or ShowDerivedObjects->isChecked() )
    {
      if ( TableTab * CurrentTab = dynamic_cast<TableTab *> ( tableholder->currentWidget() ) )
      {
        CurrentTab->CreateModels();
        dbe::models::table * CurrentTabModel = CurrentTab->GetTableModel();
        CustomDelegate * CurrentDelegate = CurrentTab->GetTableDelegate();
        CustomTableView * CurrentView = CurrentTab->GetTableView();

        connect ( CurrentView, SIGNAL ( OpenEditor ( tref ) ), this,
                  SLOT ( slot_launch_object_editor ( tref ) ), Qt::UniqueConnection );
        connect ( CurrentDelegate, SIGNAL ( CreateObjectEditorSignal ( tref ) ), this,
                  SLOT ( slot_launch_object_editor ( tref ) ), Qt::UniqueConnection );

        if ( dynamic_cast<ClassNode *> ( tree_node ) )
        {
          BOOST_SCOPE_EXIT(CurrentTabModel)
          {
              emit CurrentTabModel->layoutChanged();
          }
          BOOST_SCOPE_EXIT_END

          emit CurrentTabModel->layoutAboutToBeChanged();

          CurrentTabModel->BuildTableFromClass ( cname, ShowDerivedObjects->isChecked() );
          build_table_model();
          tableholder->setTabText ( tableholder->currentIndex(), cname );
          CurrentTab->ResetTableView();
        }

        CurrentTab->ResizeHeaders();
      }
    }
  }
}


void dbe::MainWindow::build_file_model()
{
  /// Changed -> Now accepting rdbconfig this means || !ConfigWrapper::GetInstance().GetDatabaseImplementation().contains("rdbconfig") was removed

  if ( !confaccessor::db_implementation_name().contains ( "roksconflibs" ) )
  {
    if ( this_files != nullptr ) {
      delete this_files;
    }
    this_files = new FileModel();

    this_filesort.setSourceModel ( this_files );
    FileView->setModel ( &this_filesort );

    FileView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    FileView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    FileView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    FileView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    emit signal_new_file_model();
  }
}


void dbe::MainWindow::slot_fetch_data ( const treenode * ClassNode )
{
  if ( this_classes->canFetchMore ( this_classes->index ( ClassNode->GetRow(), 0,
                                                          QModelIndex() ) ) )
  {
    this_classes->fetchMore ( this_classes->index ( ClassNode->GetRow(), 0, QModelIndex() ) );
  }
}

bool dbe::MainWindow::slot_commit_database ( bool Exit )
{
  CommitDialog * SaveDialog = new CommitDialog();
  int DialogResult = SaveDialog->exec();

  if ( DialogResult )
  {
    QString CommitMessage = SaveDialog->GetCommitMessage();

    try
    {
      std::list<std::string> const & modified = confaccessor::save ( CommitMessage );
      confaccessor::clear_commands();

      build_file_model();

      if ( not modified.empty() )
      {
        std::string msg;

        for ( std::string const & f : modified )
        {
          msg += "\n" + f;
        }

        INFO ( "List of modified files committed to the database ", "Program execution success",
               msg );
      }
      else
      {
        WARN ( "Changes where committed successfully but list of modified files could not be retrieved",
               "Unexpected program execution" );
      }

    }
    catch ( dunedaq::conffwk::Exception const & e )
    {
      std::cout << "Caught Exception: " << e.what() << "\n";
      WARN ( "The changes could not be committed", dbe::config::errors::parse ( e ).c_str() )
      ers::error ( e );
      return false;
    }
    // Gaahhh confaccessor catches dunedaq::conffwk::Exception and
    // rethrows it as daq::dbe::CouldNotCommitChanges!!
    catch (daq::dbe::CouldNotCommitChanges const& exc)
    {
      std::cout << "Caught daq::dbe::CouldNotCommitChanges: " << exc.what() << "\n";
      std::string reason{exc.what()};
      auto cause = exc.cause();
      while (cause != nullptr) {
        reason = cause->what();
        cause = cause->cause();
      }
      WARN ("The changes could not be committed",
            // dbe::config::errors::parse(exc).c_str(),
            reason,
            "\n\nTry fixing includes from File Info window")
      ers::error (exc);
      return false;
    }
  }
  else
  {
    if ( Exit )
    {
      slot_abort_changes();
    }
  }
  return true;
}

void dbe::MainWindow::slot_abort_changes()
{
  try
  {
    if ( confaccessor::is_database_loaded() )
    {
      confaccessor::abort();
      confaccessor::clear_commands();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    ERROR ( "Database changes aborted", dbe::config::errors::parse ( e ).c_str() );
    ers::error ( e );
  }
}

void dbe::MainWindow::slot_abort_external_changes()
{
  try
  {
    if ( confaccessor::is_database_loaded() )
    {
      confaccessor::abort();
    }
  }
  catch ( dunedaq::conffwk::Exception const & e )
  {
    ERROR ( "External changes aborted", dbe::config::errors::parse ( e ).c_str() );
    ers::error ( e );
  }
}

void dbe::MainWindow::slot_launch_object_editor ( tref Object )
{
  bool WidgetFound = false;
  QString ObjectEditorName = QString ( "%1@%2" ).arg ( Object.UID().c_str() ).arg (
                               Object.class_name().c_str() );

  for ( QWidget * Editor : QApplication::allWidgets() )
  {
    ObjectEditor * Widget = dynamic_cast<ObjectEditor *> ( Editor );

    if ( Widget != nullptr )
    {
      if ( ( Widget->objectName() ).compare ( ObjectEditorName ) == 0 )
      {
        Widget->raise();
        Widget->setVisible ( true );
        WidgetFound = true;
      }
    }
  }

  if ( !WidgetFound )
  {
    ( new ObjectEditor ( Object ) )->show();
  }
}

void dbe::MainWindow::slot_launch_batchchange()
{
  if ( confaccessor::is_database_loaded() )
  {
    BatchChangeWidget * Batch = new BatchChangeWidget ( nullptr );
    Batch->setWindowModality ( Qt::WindowModal );
    Batch->show();
  }
  else
  {
    ERROR ( "Database must have been loaded", "No database loaded" );
  }
}

void dbe::MainWindow::slot_launch_batchchange_on_table()
{
  if ( !confaccessor::is_database_loaded() )
  {
    ERROR ( "Database must have been loaded", "No database loaded" );
    return;
  }

  dbe::models::table * CurrentTableModel = nullptr;
  TableTab * CurrentTab = dynamic_cast<TableTab *> ( tableholder->currentWidget() );
  if ( CurrentTab ) {
      CurrentTableModel = CurrentTab->GetTableModel();
  }

  std::vector<dref> TableObject;

  if ( !CurrentTab || !CurrentTableModel )
  {
    ERROR ( "Table cannot be processed", "Table is empty" );
    return;
  }

  if ( ( *CurrentTableModel->GetTableObjects() ).isEmpty() )
  {
    ERROR ( "Table cannot be processed", "Table is empty" );
    return;
  }

  QString Filter = SearchLineTable->text();

  for ( dref Object : *CurrentTableModel->GetTableObjects() )
  {
    if ( Filter.isEmpty() )
    {
      TableObject.push_back ( Object );
    }
    else
    {
      QString ObjectString = QString::fromStdString ( Object.UID() );

      if ( ObjectString.contains ( Filter, Qt::CaseInsensitive ) )
      {
        TableObject.push_back ( Object );
      }
    }
  }

  BatchChangeWidget * Batch = new BatchChangeWidget (
    true,
    CurrentTableModel->get_class_name(),
    TableObject, nullptr );
  Batch->setWindowModality ( Qt::WindowModal );
  Batch->show();
}

void dbe::MainWindow::LoadDefaultSetting()
{
  load_settings ( false );
}

QString dbe::MainWindow::find_db_repository_dir()
{
    if (confaccessor::dbfullname().isEmpty()) {
      return "";
    }

    const QStringList& incs =dbe::config::api::get::file::inclusions({confaccessor::dbfullname()});
    for(QString f : allFiles) {
        for(const QString& j : incs) {
            if(f.endsWith(j)) {
                return f.remove(j);
            }
        }
    }

    return "";
}

void dbe::MainWindow::slot_create_newdb()
{

  CreateDatabaseWidget * CreateDatabaseW = new CreateDatabaseWidget(nullptr, false, find_db_repository_dir());
  CreateDatabaseW->show();
  connect ( CreateDatabaseW, SIGNAL ( CanLoadDatabase ( const QString & ) ), this,
            SLOT ( slot_load_db_from_create_widget ( const QString & ) ) );
}

void dbe::MainWindow::slot_load_db_from_create_widget ( const QString & DatabaseName )
{
  if ( !DatabaseName.isEmpty() )
  {
    QFileInfo DatabaseFile = QFileInfo ( DatabaseName );

    if ( DatabaseFile.exists() )
    {
      QString Path = QString ( DatabaseFile.absoluteFilePath() );

      if ( dbreload() )
      {
        confaccessor::setdbinfo ( Path );

        if ( dbload() )
        {
          setinternals();
          build_class_tree_model();
          // // build_partition_tree_model();
          // build_resource_tree_model();
          build_file_model();
        }
      }
    }
    else
    {
      WARN ( "File not found during database load", "File does not exist", "\n\n Filename:",
             DatabaseFile.fileName().toStdString() );
    }
  }
  else
  {
    ERROR ( "Database load error", "File was not selected" );
  }
}

bool dbe::MainWindow::dbreload()
{
  if ( confaccessor::is_database_loaded() )
  {
    QMessageBox MessageBox;
    MessageBox.setText (
      "Do you really wish to abandon the current database and load a new one ?" );
    MessageBox.setStandardButtons ( QMessageBox::Yes | QMessageBox::Cancel );
    MessageBox.setDefaultButton ( QMessageBox::Cancel );
    int UserOption = MessageBox.exec();

    switch ( UserOption )
    {

    case QMessageBox::Yes:
      return true;

    case QMessageBox::Cancel:
      return false;

    default:
      return false;
    }
  }
  else
  {
    return true;
  }
}

bool dbe::MainWindow::dbload()
{
    // For issues related to loading the configuration in a separate thread, see ATLASDBE-229

    const bool alreadyLoaded = confaccessor::is_database_loaded();

    // The QueuedConnection is mandatory to let the loop receive the signal even if
    // it is emitted before "exec" is called
    QEventLoop loop;
    connect(this, SIGNAL(signal_db_loaded()), &loop, SLOT(quit()), Qt::QueuedConnection);

    // Make life of the progress dialog longer
    // Show only the first time, when the configuration is not loaded
    // In other cases, just show a busy cursor
    std::unique_ptr<QProgressDialog> progress_bar;
    if(!alreadyLoaded) {
        progress_bar.reset(new QProgressDialog( "Loading Configuration...", QString(), 0, 0, this ));
        progress_bar->setWindowModality ( Qt::WindowModal );
        progress_bar->show();
    }

    BOOST_SCOPE_EXIT(void)
    {
        QApplication::restoreOverrideCursor();
    }
    BOOST_SCOPE_EXIT_END

    QApplication::setOverrideCursor ( QCursor ( Qt::WaitCursor ) );

    // Close widgets
    for ( QWidget * widget : QApplication::allWidgets() )
    {
        if ( dynamic_cast<ObjectEditor *> ( widget ) )
        {
            widget->close();
        }
        else if ( dynamic_cast<widgets::editors::base *> ( widget ) )
        {
            widget->close();
        }
    }

    // Asynchronous execution only the first time the configuration is loaded
    std::future<bool> waiter = std::async ( alreadyLoaded ? std::launch::deferred : std::launch::async, [this]
    {
      const bool result = confaccessor::load(!isArchivedConf);
      emit signal_db_loaded(); // "loop.exec()" will return now
      return result;
    } );


    // Do not call "exec" if the previous call is not asynchronous
    if(!alreadyLoaded) {
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }

    // If "deferred", the async call is executed now and here
    return waiter.get();
}

void dbe::MainWindow::setinternals()
{
  confaccessor::clear_commands();
  confaccessor::gethandler()->ResetData();
  confaccessor::set_total_objects ( 0 );

  /// Disconnecting models from views

  for ( int i = 0; i < tableholder->count(); i++ )
  {
    TableTab * CurrentTab = dynamic_cast<TableTab *> ( tableholder->widget ( i ) );
    if ( CurrentTab ) {
        CurrentTab->DisconnectView();
    }
  }

  FileView->setModel ( NULL );
}

void dbe::MainWindow::load_settings ( bool LoadSettings )
{
  /// Load Settings means default settings
  QSettings * Settings;
  QString userPath = QDir::homePath() + "/.conffwk/ATLAS_TDAQ_DBE";
  QString userFile = "DBE_User_Settings.conf";

  if ( !LoadSettings )
  {
    if ( QDir ( userPath ).exists ( userFile ) )
      Settings = new QSettings ( "ATLAS_TDAQ_DBE",
                                 "DBE_User_Settings" );
    else
      Settings = new QSettings ( ":theme/DBE_Default_User_Settings.conf",
                                 QSettings::NativeFormat );
  }
  else
  {
    Settings = new QSettings ( ":theme/DBE_Default_User_Settings.conf",
                               QSettings::NativeFormat );
  }

  Settings->beginGroup ( "MainWindow-layout" );
  resize ( Settings->value ( "size" ).toSize() );
  move ( Settings->value ( "pos" ).toPoint() );
  DisplayTableView->setChecked ( Settings->value ( "TableView" ).toBool() );
  DisplayClassView->setChecked ( Settings->value ( "ClassView" ).toBool() );

  DisplayMessages->setChecked ( Settings->value ( "Messages" ).toBool() );
  restoreGeometry ( Settings->value ( "geometry" ).toByteArray() );
  restoreState ( Settings->value ( "state" ).toByteArray() );
  Settings->endGroup();

  Settings->beginGroup ( "MainWindow-checkboxes" );
  CaseSensitiveCheckBoxTree->setChecked (
    Settings->value ( "tree-case-sensitive" ).toBool() );
  CaseSensitiveCheckBoxTable->setChecked (
    Settings->value ( "table-case-sensitive" ).toBool() );
  Settings->endGroup();
}

void dbe::MainWindow::WriteSettings()
{
  QSettings Settings ( "ATLAS_TDAQ_DBE", "DBE_User_Settings" );
  Settings.beginGroup ( "MainWindow-layout" );
  Settings.setValue ( "size", size() );
  Settings.setValue ( "pos", pos() );
  Settings.setValue ( "TableView", DisplayTableView->isChecked() );
  Settings.setValue ( "ClassView", DisplayClassView->isChecked() );

  Settings.setValue ( "Messages", DisplayMessages->isChecked() );
  Settings.setValue ( "geometry", saveGeometry() );
  Settings.setValue ( "state", saveState() );
  Settings.endGroup();

  Settings.beginGroup ( "MainWindow-checkboxes" );
  Settings.setValue ( "tree-case-sensitive", CaseSensitiveCheckBoxTree->isChecked() );
  Settings.setValue ( "table-case-sensitive", CaseSensitiveCheckBoxTable->isChecked() );
  Settings.endGroup();
}

void dbe::MainWindow::argsparse ( QMap<QString, QString> const & opts )
{
  if ( !opts.isEmpty() )
  {
    dbinfo LoadConfig;
    QString FileToLoad;

    QString FileName = opts.value ( "f" );
    QString RdbFileName = opts.value ( "r" );
    QString RoksFileName = opts.value ( "o" );
    QString HashVersion = opts.value ( "v" );

    if ( !FileName.isEmpty() )
    {
      FileToLoad = FileName;
      LoadConfig = dbinfo::oks;

      if ( !HashVersion.isEmpty() )
      {
        ::setenv("TDAQ_DB_VERSION", QString("hash:").append(HashVersion).toStdString().c_str(), 1);
        ::setenv("OKS_GIT_PROTOCOL", "http", 1);
        isArchivedConf = true;
      }
    }
    else if ( !RdbFileName.isEmpty() )
    {
      FileToLoad = RdbFileName;
      LoadConfig = dbinfo::rdb;
    }
    else if ( !RoksFileName.isEmpty() )
    {
      FileToLoad = RoksFileName;
      LoadConfig = dbinfo::roks;
    }

    if ( not FileToLoad.isEmpty() )
    {
      dbopen ( FileToLoad, LoadConfig );
    }
  }
}

// /**
//  * Create Rdb menu based on the available Rdb information
//  */
// void dbe::MainWindow::init_rdb_menu()
// {
//   ConnectToRdb->clear();

//   std::list<IPCPartition> pl;
//   IPCPartition::getPartitions(pl);
//   TLOG_DEBUG(1) <<  "Found " << pl.size() << " partitions"  ;

//   pl.push_front(IPCPartition("initial"));

//   auto f = [pl, this] () {
//       for ( auto it = pl.begin(); it != pl.end(); ++it )
//       {
//           lookForRDBServers ( *it );
//       }
//   };

//   std::thread t(f);
//   t.detach();
// }

// void dbe::MainWindow::slot_rdb_found(const QString& p, const RDBMap& rdbs) {
//     QMenu * part_menu = new QMenu(p);

//     for(auto it = rdbs.begin(); it != rdbs.end(); ++it) {
//         QAction * newAct = new QAction ( it.key(), part_menu );

//         QFont actFont = newAct->font();
//         if(it.value() == true) {
//             newAct->setToolTip ( QString ( "This is a Read-Only instance of the DB" ) );
//             actFont.setItalic ( true );
//         } else {
//             newAct->setToolTip ( QString ( "This is a Read/Write instance of the DB" ) );
//             actFont.setBold ( true );
//         }

//         newAct->setFont ( actFont );

//         part_menu->addAction ( newAct );
//     }

//     ConnectToRdb->addMenu ( part_menu );
// }

// /**
//  * Add rdb servers for each partition
//  *
//  * @param p is the partition source for which to populate with server information
//  */
// void dbe::MainWindow::lookForRDBServers ( const IPCPartition & p )
// {
//   TLOG_DEBUG(2) <<  "dbe::MainWindow::addRDBServers()"  ;

//   if ( p.isValid() )
//   {
//     TLOG_DEBUG(2) <<  "Inserting partition = " << p.name()  ;

//     RDBMap rdbs;

//     try
//     {
//         {
//             std::map<std::string, rdb::cursor_var> objects;
//             p.getObjects<rdb::cursor, ::ipc::use_cache, ::ipc::unchecked_narrow> ( objects );
//             std::map<std::string, rdb::cursor_var>::iterator rdb_it = objects.begin();

//             while ( rdb_it != objects.end() )
//             {
//                 TLOG_DEBUG(2) <<  "Found server : " << rdb_it->first  ;

//                 rdbs.insert(QString::fromStdString(rdb_it->first), true);

//                 ++rdb_it;
//             }
//         }

//         {
//             std::map<std::string, rdb::writer_var> objects;
//             p.getObjects<rdb::writer, ::ipc::use_cache, ::ipc::unchecked_narrow> ( objects );
//             std::map<std::string, rdb::writer_var>::iterator rdb_it = objects.begin();

//             while ( rdb_it != objects.end() )
//             {
//                 TLOG_DEBUG(2) <<  "Found server : " << rdb_it->first  ;

//                 rdbs.insert(QString::fromStdString(rdb_it->first), false);

//                 ++rdb_it;
//             }
//         }
//     }
//     catch ( daq::ipc::InvalidPartition& e )
//     {
//       ers::error ( e );
//     }

//     if(rdbs.isEmpty() == false) {
//         emit signal_rdb_found (QString::fromStdString(p.name()), rdbs);
//     }
//   }
// }

// void dbe::MainWindow::slot_rdb_selected ( QAction * action )
// {
//   QMenu * parentMenu = qobject_cast<QMenu *> ( action->parent() );

//   if ( parentMenu )
//   {
//     if ( dbreload() )
//     {
//       BOOST_SCOPE_EXIT(void)
//       {
//           QApplication::restoreOverrideCursor();
//        }
//       BOOST_SCOPE_EXIT_END

//       QApplication::setOverrideCursor(Qt::WaitCursor);

//       confaccessor::setdbinfo ( action->text() + "@" + parentMenu->title(), dbinfo::rdb );

//       if ( dbload() )
//       {
//         setinternals();
//         build_class_tree_model();
//         build_partition_tree_model();
//         build_resource_tree_model();
//         build_file_model();
//       }
//     }
//   }
// }

// void dbe::MainWindow::slot_oracle_prepare()
// {
//   if ( this_oraclewidget == nullptr )
//   {
//     this_oraclewidget = new OracleWidget();
//     connect ( this_oraclewidget, SIGNAL ( OpenOracleConfig ( const QString & ) ), this,
//               SLOT ( slot_load_oracle ( const QString & ) ) );
//   }

//   this_oraclewidget->raise();
//   this_oraclewidget->show();
// }

// void dbe::MainWindow::slot_load_oracle ( const QString & OracleDatabase )
// {
//   if ( dbreload() )
//   {
//     confaccessor::setdblocation ( OracleDatabase );

//     if ( dbload() )
//     {
//       setinternals();
//       build_class_tree_model();
//       build_partition_tree_model();
//       build_resource_tree_model();
//       build_file_model();
//     }
//   }

//   if ( this_oraclewidget != nullptr )
//   {
//     this_oraclewidget->close();
//   }
// }

void dbe::MainWindow::slot_whatisthis()
{
  QWhatsThis::enterWhatsThisMode();
}

void dbe::MainWindow::slot_show_information_about_dbe()
{
  static QString const title ( "About DBE" );
  static QString const msg = QString().
                             append ( "DBE is an editor to work with OKS and RDB backends that manages most of the hard work for you in editing the configuration database\n" ).
                             append ( "\n\nMaintained :\t\tC&C Working group \n\t\t\t(atlas-tdaq-cc-wg@cern.ch)" ).
                             append ( "\nProgram version:\t\t" ).append ( dbe_compiled_version ).
                             append ( "\nLibraries version:\t" ).
                             append ( "\n\t\t\tdbecore(" ).append ( dbe_lib_core_version ).
                             append ( "),\n\t\t\tdbe_config_api(" ).append ( dbe_lib_config_api_version ).
                             append ( "),\n\t\t\tdbe_structure(" ).append ( dbe_lib_structure_version ).
                             append ( "),\n\t\t\tdbe_internal(" ).append ( dbe_lib_internal_version ).append ( ')' ).
                             append ( "\nRepo commit hash:\t" ).append ( dbe_compiled_commit );

  QMessageBox::about ( this, title, msg );
}

void dbe::MainWindow::slot_show_userguide()
{
  QDesktopServices::openUrl ( QUrl ( "https://atlasdaq.cern.ch/dbe/" ) );
}

void dbe::MainWindow::slot_show_userchanges()
{
  InfoWidget->setCurrentIndex ( InfoWidget->indexOf ( CommitedTab ) );
}

void dbe::MainWindow::slot_undo_allchanges()
{
  UndoView->stack()->setIndex ( 0 );
}

void dbe::MainWindow::slot_toggle_casesensitive_for_treeview ( bool )
{
  if ( CaseSensitiveCheckBoxTree->isChecked() )
    this_treefilter->setFilterCaseSensitivity (
      Qt::CaseSensitive );
  else
  {
    this_treefilter->setFilterCaseSensitivity ( Qt::CaseInsensitive );
  }
  update_total_objects();
}

void dbe::MainWindow::slot_model_rebuild()
{
  /// Preparing data
  confaccessor::gethandler()->ResetData();
  confaccessor::set_total_objects ( 0 );
  /// Disconnecting models from views

  for ( int i = 0; i < tableholder->count(); i++ )
  {
    TableTab * CurrentTab = dynamic_cast<TableTab *> ( tableholder->widget ( i ) );
    if( CurrentTab ) {
        CurrentTab->DisconnectView();
    }
  }

  FileView->setModel ( NULL );

  build_class_tree_model();
  build_file_model();
}

void dbe::MainWindow::slot_filter_textchange ( const QString & FilterText )
{
  if ( this_treefilter != nullptr and SearchBox->currentIndex() != 1 )
  {
    this_treefilter->SetFilterType ( models::treeselection::RegExpFilterType );

    if ( SearchBox->currentIndex() == 2 )
    {
      this_treefilter->SetFilterRestrictionLevel ( 1000 );
    }
    else
    {
      this_treefilter->SetFilterRestrictionLevel ( 1 );
    }

    this_treefilter->setFilterRegExp ( FilterText );
  }

  update_total_objects();
}

void dbe::MainWindow::slot_filter_query()
{
  if ( this_treefilter == nullptr )
  {
    return;
  }

  QString Tmp = SearchTreeLine->text();
  if ( SearchBox->currentIndex() == 1 )
  {
    this_treefilter->SetFilterType ( models::treeselection::ObjectFilterType );
    std::vector<dbe::tref> Objects = ProcessQuery ( Tmp );

    this_treefilter->SetQueryObjects ( Objects );
    this_treefilter->setFilterRegExp ( Tmp );
    update_total_objects();
  }
  else {
    this_treefilter->ResetQueryObjects ( );
    slot_filter_textchange( Tmp );
  }
}

void dbe::MainWindow::slot_filter_table_textchange ( const QString & FilterText )
{
  TableTab * CurrentTab = dynamic_cast<TableTab *> ( tableholder->currentWidget() );

  if ( CurrentTab )
  {
    dbe::models::tableselection * TableFilter = CurrentTab->GetTableFilter();

    if ( TableFilter == nullptr )
    {
      return;
    }

    TableFilter->SetFilterType ( dbe::models::tableselection::RegExpFilter );

    if ( CaseSensitiveCheckBoxTable->isChecked() )
      TableFilter->setFilterCaseSensitivity (
        Qt::CaseSensitive );
    else
    {
      TableFilter->setFilterCaseSensitivity ( Qt::CaseInsensitive );
    }

    TableFilter->setFilterRegExp ( FilterText );
  }
}

void dbe::MainWindow::slot_tree_reset()
{
    // Keep track of the selected tab
    int IndexOfCurrentTab = tableholder->currentIndex();

    // Here are the all the open tabs
    std::vector<QModelIndex> idxs;

    for(int i = 0; i < tableholder->count(); ++i) {
        TableTab * CurrentTab = dynamic_cast<TableTab *>(tableholder->widget(i));
        if(CurrentTab) {
            if(CurrentTab->GetTableModel()) {
                const QString& TableClassName = CurrentTab->GetTableModel()->get_class_name();
                if(!TableClassName.isEmpty()) {
                    treenode * NodeClass = confaccessor::gethandler()->getnode(TableClassName);
                    if(NodeClass != nullptr) {
                        idxs.push_back(this_classes->getindex(NodeClass));
                    }
                }
            }
        }
    }

    // Remove all the tabs
    while(tableholder->count() != 0) {
        tableholder->widget(0)->deleteLater();
        tableholder->removeTab(0);
    }

    // Disconnecting models from views
    build_class_tree_model();

    // Re-create all the tabs
    for(const auto& idx : idxs) {
        slot_add_tab();
        edit_object_at(idx);
    }

    // Set the current tab
    tableholder->setCurrentIndex ( IndexOfCurrentTab );
}

void dbe::MainWindow::update_total_objects()
{
  int total=0;
  for (int item=0; item<this_treefilter->rowCount(); item++) {
    auto index = this_treefilter->index(item, 1);
    auto data = this_treefilter->data(index);
    total += data.toInt();
  }
  TotalObjectsLabel->setText (
    QString ( "Total Objects: %1" ).arg ( total ) );
}

void dbe::MainWindow::closeEvent ( QCloseEvent * event )
{
  if ( isArchivedConf || check_close() )
  {
    WriteSettings();

    foreach ( QWidget * widget, QApplication::allWidgets() ) widget->close();

    event->accept();
  }
  else
  {
    event->ignore();
  }
}

std::vector<dbe::tref> dbe::MainWindow::ProcessQuery ( QString const & Tmp )
{
  if ( not Tmp.isEmpty() )
  {
    QString const Query = QString ( "(this (object-id \".*%1.*\" ~=))" ).arg ( Tmp );

    try
    {
      std::vector<dbe::tref> result;

      for ( std::string const & cname : dbe::config::api::info::onclass::allnames <
            std::vector<std::string >> () )
      {
        std::vector<dbe::tref> class_matching_objects = inner::dbcontroller::gets (
                                                          cname, Query.toStdString() );

        result.insert ( result.end(), class_matching_objects.begin(),
                        class_matching_objects.end() );
      }

      return result;
    }
    catch ( dunedaq::conffwk::Exception const & ex )
    {
      ers::error ( ex );
      ERROR ( "Query process error", dbe::config::errors::parse ( ex ).c_str() );
    }
  }

  return
    {};
}

bool dbe::MainWindow::eventFilter ( QObject * Target, QEvent * Event )
{
  if ( Target == SearchBox->lineEdit() && Event->type() == QEvent::MouseButtonRelease )
  {
    if ( !SearchBox->lineEdit()->hasSelectedText() )
    {
      SearchBox->lineEdit()->selectAll();
      return true;
    }
  }

  return false;
}

bool dbe::MainWindow::check_close()
{
  bool OK = true;

  foreach ( QWidget * widget, QApplication::allWidgets() )
  {
    ObjectCreator * ObjectCreatorInstance = dynamic_cast<ObjectCreator *> ( widget );
    ObjectEditor * ObjectEditorInstance = dynamic_cast<ObjectEditor *> ( widget );

    if ( ObjectEditorInstance )
    {
      OK = ObjectEditorInstance->CanCloseWindow();
    }

    if ( !OK )
    {
      return false;
    }

    if ( ObjectCreatorInstance )
    {
      OK = ObjectCreatorInstance->CanClose();
    }

    if ( !OK )
    {
      return false;
    }
  }

  {
    cptr<QUndoStack> undo_stack ( confaccessor::get_commands() );

    if ( undo_stack->isClean() )
    {
      if ( undo_stack->count() == 0 )
      {
        return true;
      }
      else
      {
        slot_abort_changes();
        return true;
      }
    }
    else
    {
      int ret =
        QMessageBox::question (
          0,
          tr ( "DBE" ),
          QString (
            "There are unsaved changes.\n\nDo you want to save and commit them to the DB?\n" ),
          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
          QMessageBox::Save );

      if ( ret == QMessageBox::Discard )
      {
        slot_abort_changes();
        return true;
      }
      else if ( ret == QMessageBox::Save )
      {
        return slot_commit_database ( true );
      }
      else if ( ret == QMessageBox::Cancel )
      {
        return false;
      }
      else
      {
        return true;
      }
    }
  }
}

void dbe::MainWindow::slot_edit_object_from_class_view ( QModelIndex const & ProxyIndex )
{
  edit_object_at ( this_treefilter->mapToSource ( ProxyIndex ) );
}

/**
 * Takes necessary actions to load a database from a file provided
 * @param dbpath is the path (absolute or relative to the DUNEDAQ_DB_PATH) of the associated file
 * @return
 */
bool dbe::MainWindow::dbopen ( QString const & dbpath, dbinfo const & loadtype )
{
    if ( dbreload() )
    {
      confaccessor::setdbinfo ( dbpath, loadtype );

      BOOST_SCOPE_EXIT(void)
      {
          QApplication::restoreOverrideCursor();
      }
      BOOST_SCOPE_EXIT_END

      QApplication::setOverrideCursor(Qt::WaitCursor);

      if ( dbload() )
      {
        setinternals();
        build_class_tree_model();
        // build_partition_tree_model();
        // build_resource_tree_model();
        build_file_model();
      }
    }

  return true;
}

void dbe::MainWindow::slot_open_database_from_file()
{
  QFileDialog FileDialog ( this, tr ( "Open File" ), ".", tr ( "XML files (*.xml)" ) );
  FileDialog.setAcceptMode ( QFileDialog::AcceptOpen );
  FileDialog.setFileMode ( QFileDialog::ExistingFile );
  FileDialog.setViewMode ( QFileDialog::Detail );

  if ( FileDialog.exec() )
  {
    QStringList FilesSelected = FileDialog.selectedFiles();

    if ( FilesSelected.size() )
    {
      QString DatabasePath = FilesSelected.value ( 0 );
      dbopen ( DatabasePath, dbinfo::oks );
    }
  }
}


/**
 * The purpose of this method is to replay local changes after the database has been externally
 * modified.
 *
 * It is called when there is a callback from config layer.
 *
 * The user is being given the option to ignore the external change and proceed without bringing
 * his current database to a consistent state. This will cause the database to be overwritten when
 * local changes are going to be applied.
 */
void dbe::MainWindow::slot_process_externalchanges()
{
  auto user_confirmation = [] ( QString const & msg )
  {
    QMessageBox ExternalMessageBox;
    ExternalMessageBox.setText ( msg );
    ExternalMessageBox.setStandardButtons ( QMessageBox::Yes | QMessageBox::No );
    ExternalMessageBox.setDefaultButton ( QMessageBox::Yes );
    return ExternalMessageBox.exec() == QMessageBox::Yes;
  };

  confaccessor::t_undo_stack_cptr undo_stack = confaccessor::get_commands();

  auto rewind_stack = [&undo_stack] ()
  {
      std::vector<bool > commands_original_undo_state;

      // Loop over the commands and set their undo-state to false such that when the undostack
      // index is rewind to zero they will not be undone. The purpose is to replay them on top of
      // current changes.

      for ( int i = 0; i < undo_stack->count(); ++i )
      {
        if ( dbe::actions::onobject const * Command =
               dynamic_cast<dbe::actions::onobject const *> ( undo_stack->command ( i ) )
           )
        {
          commands_original_undo_state.push_back ( Command->undoable() );
          Command->setundoable ( false );
        }
      }

      // Rewind the command stack by setting the index to zero
      // Commands will not be replayed since we have set their state to false
      undo_stack->setIndex ( 0 );

      // Reset the state of all commands one by one
      {
        auto cmdstate = commands_original_undo_state.begin();

        for ( int i = 0; i != undo_stack->count(); ++i )
        {
          if ( dbe::actions::onobject const * Command =
                 dynamic_cast<dbe::actions::onobject const *> ( undo_stack->command ( i ) )
             )
          {
            Command->setundoable ( *cmdstate++ );
          }
        }
      }
  };

  // Close active editor widgets before replaying changes
  for ( QWidget * widget : QApplication::allWidgets() )
  {
      if ( dynamic_cast<widgets::editors::relation *> ( widget ) )
      {
          widget->close();
      }
  }


  if ( undo_stack->count() != 0 )
  {
    const QString msg = QString("External changes to the database have been applied. Do you want to replay your changes on top? ")
                       + QString(" Otherwise any local change will be lost.\n");
    if ( user_confirmation ( msg ) )
    {
      rewind_stack();

      // Empty the internal stack and place the changes in a reverse order in a local stack
      confaccessor::t_internal_changes_stack internal_changes_reverse_copy;
      auto internal_changes = confaccessor::get_internal_change_stack();

      while ( not internal_changes->empty() )
      {
        internal_changes_reverse_copy.push ( internal_changes->top() );
        internal_changes->pop();
      }

      // Replay the commands one by one
      for ( int i = 0; i < undo_stack->count(); ++i )
      {
        config_internal_change Change = internal_changes_reverse_copy.top();
        internal_changes_reverse_copy.pop();
        internal_changes->push ( Change );

        try
        {

          dbe::actions::onobject const * Command =
            dynamic_cast<dbe::actions::onobject const *> ( undo_stack->command ( i ) );

          if ( not Command->redoable() )
          {
            undo_stack->redo();
          }
          else
          {
            // If the object we are trying to make the changes to does not exist it means it was deleted

            if ( ( dbe::config::api::info::has_obj ( Change.classname, Change.uid ) and Change
                   .request
                   != config_internal_change::CREATED )
                 or Change.request == config_internal_change::FILE_INCLUDED
                 or Change.request == config_internal_change::FILE_DELETED )
            {
              // If in virtue of external modification the object still exists and our action was not a creation
              Command->reload();
              undo_stack->redo();
            }
            else if ( not dbe::config::api::info::has_obj ( Change.classname, Change.uid ) and Change
                      .request
                      == config_internal_change::CREATED )
            {
              // If the external changes have removed the object and we have created it
              undo_stack->redo();
              Command->reload();
            }
            else
            {
              /// "Emptying" the command so it does nothing at all
              Command->setredoable ( false );
              Command->setundoable ( false );

              // Advance the stack by redoing an non-redoable (i.e. the redo action has no effect) command
              undo_stack->redo();
            }
          }

        }
        catch ( dunedaq::conffwk::Exception const & e )
        {
          WARN ( "Object reference could not be changed",
                 dbe::config::errors::parse ( e ).c_str(), "for object with UID:", Change.uid,
                 "of class", Change.classname );
        }
        catch ( ... )
        {
          WARN ( "Unknown exception during object modification", "s",
                 "\n\nFor object with UID:", Change.uid.c_str(), "of class:",
                 Change.classname.c_str() );
        }
      }
    } else {
        confaccessor::clear_commands();
    }
  }
  else
  {
    INFO ( "Database reloaded due external changes", "Database consistency enforcement" );
    confaccessor::clear_commands();
  }

  slot_tree_reset();
  build_file_model();

  // Emit the signal for connected listeners (e.g., the object editors)
  emit signal_externalchanges_processed();
}

/**
 * Permits to retrieve the main window pointer throughout the application
 *
 * @return a pointer of type MainWindow to the first class of type MainWindow
 */
dbe::MainWindow * dbe::MainWindow::findthis()
{
  QWidgetList allwidgets = QApplication::topLevelWidgets();

  QWidgetList::iterator it = allwidgets.begin();
  MainWindow * main_win = qobject_cast<MainWindow *> ( *it );

  for ( ; it != allwidgets.end() and main_win == nullptr; ++it )
  {
    main_win = qobject_cast<MainWindow *> ( *it );
  }

  return main_win;
}

//-----------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------
/**
 * This method permits to propagate and display messages from the messaging subsytem.
 *
 * It is important that the arguments are pass-by-copy because references will become invalid,
 * even if they are bound to consted temporaries, once the deleter from the other thread is called.
 *
 */
namespace {
    const int MAX_MESSAGE_LENGTH = 500;
}

void dbe::MainWindow::slot_failure_message ( QString const title, QString const msg )
{
    QMessageBox mb(this);
    mb.setIcon(QMessageBox::Icon::Critical);
    mb.setWindowTitle(title);
    mb.setStandardButtons(QMessageBox::Ok);
    if(msg.length() > MAX_MESSAGE_LENGTH) {
        QString&& m = msg.left(MAX_MESSAGE_LENGTH);
        m.append("...");
        mb.setText("<b>The message has been truncated because too long, look at the details for the full message</b>");
        mb.setInformativeText(m);
        mb.setDetailedText(msg);
    } else {
        mb.setText(msg);
    }

    mb.exec();
}

/**
 * This method permits to propagate and display messages from the messaging subsytem.
 *
 * It is important that the arguments are pass-by-copy because references will become invalid,
 * even if they are bound to consted temporaries, once the deleter from the other thread is called.
 *
 */
void dbe::MainWindow::slot_information_message ( QString const title, QString const msg )
{
    QMessageBox mb(this);
    mb.setIcon(QMessageBox::Icon::Information);
    mb.setWindowTitle(title);
    mb.setStandardButtons(QMessageBox::Ok);
    if(msg.length() > MAX_MESSAGE_LENGTH) {
        QString&& m = msg.left(MAX_MESSAGE_LENGTH);
        m.append("...");
        mb.setText("<b>The message has been truncated because too long, look at the details for the full message</b>");
        mb.setInformativeText(m);
        mb.setDetailedText(msg);
    } else {
        mb.setText(msg);
    }

    mb.exec();
}

/**
 * This method permits to propagate and display messages from the messaging subsytem.
 *
 * It is important that the arguments are pass-by-copy because references will become invalid,
 * even if they are bound to consted temporaries, once the deleter from the other thread is called.
 *
 */
void dbe::MainWindow::slot_debuginfo_message ( QString const title, QString const msg )
{
    QMessageBox mb(this);
    mb.setIcon(QMessageBox::Icon::Information);
    mb.setWindowTitle(title);
    mb.setStandardButtons(QMessageBox::Ok);
    if(msg.length() > MAX_MESSAGE_LENGTH) {
        QString&& m = msg.left(MAX_MESSAGE_LENGTH);
        m.append("...");
        mb.setText("<b>The message has been truncated because too long, look at the details for the full message</b>");
        mb.setInformativeText(m);
        mb.setDetailedText(msg);
    } else {
        mb.setText(msg);
    }

    mb.exec();
}

/**
 * This method permits to propagate and display messages from the messaging subsytem.
 *
 * It is important that the arguments are pass-by-copy because references will become invalid,
 * even if they are bound to consted temporaries, once the deleter from the other thread is called.
 *
 */
void dbe::MainWindow::slot_notice_message ( QString const title, QString const msg )
{
    QMessageBox mb(this);
    mb.setIcon(QMessageBox::Icon::Information);
    mb.setWindowTitle(title);
    mb.setStandardButtons(QMessageBox::Ok);
    if(msg.length() > MAX_MESSAGE_LENGTH) {
        QString&& m = msg.left(MAX_MESSAGE_LENGTH);
        m.append("...");
        mb.setText("<b>The message has been truncated because too long, look at the details for the full message</b>");
        mb.setInformativeText(m);
        mb.setDetailedText(msg);
    } else {
        mb.setText(msg);
    }

    mb.exec();
}

/**
 * This method permits to propagate and display messages from the messaging subsytem.
 *
 * It is important that the arguments are pass-by-copy because references will become invalid,
 * even if they are bound to consted temporaries, once the deleter from the other thread is called.
 *
 */
void dbe::MainWindow::slot_error_message ( QString const title, QString const msg )
{
    QMessageBox mb(this);
    mb.setIcon(QMessageBox::Icon::Critical);
    mb.setWindowTitle(title);
    mb.setStandardButtons(QMessageBox::Ok);
    if(msg.length() > MAX_MESSAGE_LENGTH) {
        QString&& m = msg.left(MAX_MESSAGE_LENGTH);
        m.append("...");
        mb.setText("<b>The message has been truncated because too long, look at the details for the full message</b>");
        mb.setInformativeText(m);
        mb.setDetailedText(msg);
    } else {
        mb.setText(msg);
    }

    mb.exec();
}

/**
 * This method permits to propagate and display messages from the messaging subsytem.
 *
 * It is important that the arguments are pass-by-copy because references will become invalid,
 * even if they are bound to consted temporaries, once the deleter from the other thread is called.
 *
 */
void dbe::MainWindow::slot_warning_message ( QString const title, QString const msg )
{
    QMessageBox mb(this);
    mb.setIcon(QMessageBox::Icon::Warning);
    mb.setWindowTitle(title);
    mb.setStandardButtons(QMessageBox::Ok);
    if(msg.length() > MAX_MESSAGE_LENGTH) {
        QString&& m = msg.left(MAX_MESSAGE_LENGTH);
        m.append("...");
        mb.setText("<b>The message has been truncated because too long, look at the details for the full message</b>");
        mb.setInformativeText(m);
        mb.setDetailedText(msg);
    } else {
        mb.setText(msg);
    }

    mb.exec();
}

//-----------------------------------------------------------------------------------------------------------------------------

cptr<dbe::CustomTreeView> dbe::MainWindow::get_view() const
{
  return cptr<CustomTreeView> ( TreeView );
}

void dbe::MainWindow::slot_batch_change_start()
{
  m_batch_change_in_progress = true;
}

void dbe::MainWindow::slot_batch_change_stop(const QList<QPair<QString, QString>>& objs)
{
  std::vector<dbe::dref> objects;
  for(const auto& o : objs) {
      objects.push_back(inner::dbcontroller::get({o.second.toStdString(), o.first.toStdString()}));
  }

  // This allows to not reset the main tree
  this_classes->objectsUpdated(objects);

  // In this case the corresponding trees are reset
  // In order to apply the same policy as in the class tree
  // the subtree_proxy class needs to be completed with proper
  // implementation of slots when objects are modified

  // Proper "refresh" of table tabs
  for ( int i = 0; i < tableholder->count(); i++ )
  {
      TableTab * CurrentTab = dynamic_cast<TableTab *> ( tableholder->widget ( i ) );
      if ( CurrentTab ) {
          dbe::models::table* m = CurrentTab->GetTableModel();
          if ( m ) {
              m->objectsUpdated(objects);
          }
      }
  }

  emit signal_batch_change_stopped(objs);

  m_batch_change_in_progress = false;
}

void dbe::MainWindow::slot_toggle_commit_button()
{
    if(isArchivedConf == false) {
        const auto& uncommittedFiles = confaccessor::uncommitted_files();

        if(uncommittedFiles.empty() == true) {
            Commit->setEnabled(false);
            Commit->setToolTip("There is nothing to commit");
        } else {
            Commit->setEnabled(true);

            std::string l;
            for(const std::string& f : uncommittedFiles) {
                l += "  " + f + "\n";
            }

            Commit->setToolTip(QString::fromStdString("Commit changes.\nHere are the uncommitted files:\n" + l));
        }

        build_file_model();

    } else {
        Commit->setEnabled(false);
    }
}

void dbe::MainWindow::slot_update_committed_files(const std::list<std::string>& files, const std::string& msg) {
    for(const std::string& f : files) {
        CommittedTable->insertRow(0);
        CommittedTable->setItem(0, 0, new QTableWidgetItem(QString::fromStdString(f)));
        CommittedTable->setItem(0, 1, new QTableWidgetItem(QString::fromStdString(msg)));
        CommittedTable->setItem(0, 2, new QTableWidgetItem(QDate::currentDate().toString() + " " + QTime::currentTime().toString()));
    }

    CommittedTable->resizeColumnsToContents();
}

bool dbe::MainWindow::check_ready() const
{
  return not m_batch_change_in_progress;
}

void dbe::MainWindow::slot_loaded_db_file( QString file )
{
    allFiles.insert(file);
}
