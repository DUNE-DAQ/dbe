// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

#include "dbe/messenger.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/ObjectCreator.hpp"
#include "dbe/Validator.hpp"
#include "dbe/StyleUtility.hpp"
#include "ui_ObjectCreator.h"
#include "dbe/config_api_set.hpp"
#include "dbe/config_api_graph.hpp"
#include "dbe/config_api_commands.hpp"
#include "dbe/config_api.hpp"

#include "logging/Logging.hpp"

#include <boost/scope_exit.hpp>

#include <QCompleter>
#include <QMenu>
#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QMessageBox>

enum EditStates
{
  editorOk = 0,
  classNotSelected = 1,
  uidNotSet = 2,
  fileNotSet = 4,
  nullNotFilled = 8
};

//------------------------------------------------------------------------------------------
dbe::ObjectCreator::~ObjectCreator() = default;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/*
 * Create an object in a class
 */
dbe::ObjectCreator::ObjectCreator ( dunedaq::conffwk::class_t const & cinfo, QWidget * parent )
  :
  QWidget ( parent ),
  ui ( new Ui::ObjectCreator ),
  this_object_class ( cinfo ),
  this_target_object ( nullptr ),
  this_relation ( dunedaq::conffwk::relationship_t() ),
  this_files ( nullptr ),
  this_status_bar ( nullptr ),
  this_state ( 0 ),
  UidSet ( false ),
  this_associated_editor ( nullptr ),
  ContextMenu ( nullptr ),
  this_is_temporary ( false ),
  this_object_changed ( false ),
  this_create_copy ( false ),
  uuid ( QUuid::createUuid() )
{
  ui->setupUi ( this );
  this_status_bar = new QStatusBar ( ui->StatusFrame );

  QString cname = QString::fromStdString ( this_object_class.p_name );
  setWindowTitle ( QString ( "Create New Object of Class %1" ).arg ( cname ) );

  SetComboClass();
  ui->LineEditUID->setToolTip ( QString ( "Type Object unique ID and press ENTER" ) );

  ui->ComboBoxForbiddenUid->hide();
  ui->SetFileButton->hide();

  SetController();
  BuildFileModel();
  SetStatusBar();
  BuildContextMenu();

  int index = ui->ComboBoxClass->findText ( cname );
  ui->ComboBoxClass->setCurrentIndex ( 1 );  // If index is 0 somehow this doesnt work
  ui->ComboBoxClass->setCurrentIndex ( index ); // Now set it to the right value posssibly 0
  setup_editor();
  ui->ComboBoxClass->setEditText ( cname );

  ui->SetUidButton->setVisible ( false );
  UpdateActions();

  QSize SplitterSize = ui->splitter->size();
  int Width = SplitterSize.width() / 2;

  QList<int> List;
  List.push_back ( Width );
  List.push_back ( Width );

  ui->splitter->setSizes ( List );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/*
 * Clone an object in a class , uid will be required to be set explicitly
 */
dbe::ObjectCreator::ObjectCreator ( tref const & clonefrom,
                                    dunedaq::conffwk::relationship_t const & the_relation,
                                    QWidget * parent )
  :
  ObjectCreator (
   config::api::info::onclass::definition ( clonefrom.class_name(), false ), parent )
{
  this_target_object.reset ( new tref ( clonefrom ) );
  this_relation = the_relation;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::ObjectCreator::ObjectCreator ( tref const & clonefrom, QWidget * parent )
  :
  QWidget ( parent ),
  ui ( new Ui::ObjectCreator ),
  this_object_class (
   config::api::info::onclass::definition ( clonefrom.class_name(), false ) ),
  this_src_object ( new tref ( clonefrom ) ),
  this_files ( nullptr ),
  this_status_bar ( nullptr ),
  UidSet ( false ),
  this_associated_editor ( nullptr ),
  ContextMenu ( nullptr ),
  this_is_temporary ( false ),
  this_object_changed ( false ),
  this_create_copy ( true ),
  uuid ( QUuid::createUuid() )
{
  ui->setupUi ( this );
  this_status_bar = new QStatusBar ( ui->StatusFrame );

  setWindowTitle (
    QString ( "Create New Object of Class %1" ).arg (
      QString::fromStdString ( this_object_class.p_name ) ) );

  SetComboClass();
  ui->LineEditUID->setToolTip ( QString ( "Type Object unique ID and press ENTER" ) );

  ui->ComboBoxForbiddenUid->hide();
  ui->SetFileButton->hide();

  SetController();
  BuildFileModel();
  SetStatusBar();
  BuildContextMenu();

  int index = ui->ComboBoxClass->findText ( QString::fromStdString (
                                              clonefrom.class_name() ) );

  ui->ComboBoxClass->setCurrentIndex ( index );
  setup_copy_editor();

  /// Setting the same file as the object being copied
  QString const & fn = QString::fromStdString ( this_src_object->contained_in() );
  QFileInfo fileinfo ( fn );
  QModelIndexList ListOfMatch = this_files->match (
                                  this_files->index ( 0, 0 ),
                                  Qt::DisplayRole, fileinfo.fileName(), 1000,
                                  Qt::MatchContains | Qt::MatchWrap | Qt::MatchRecursive );

  ui->FileView->selectRow ( ListOfMatch.at ( 0 ).row() );

  ActiveFileChanged ( fn );

  // Set uid to the that of the object being copied
  ui->LineEditUID->setText ( QString::fromStdString ( clonefrom.UID() ) );
  ui->ComboBoxClass->setEditText ( QString::fromStdString ( clonefrom.class_name() ) );

  ui->SetUidButton->setVisible ( false );
  UpdateActions();

  QSize spliter_size = ui->splitter->size();
  int w = spliter_size.width() / 2;
  ui->splitter->setSizes ( QList<int>
  { w, w } );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectCreator::SetComboClass()
{
  QStringList ListOfClasses ( dbe::config::api::info::onclass::allnames<QStringList>() );
  ui->ComboBoxClass->addItems ( ListOfClasses );

  QCompleter * completer = new QCompleter ( ui->ComboBoxClass->model(), ui->ComboBoxClass );
  completer->setCaseSensitivity ( Qt::CaseInsensitive );
  completer->setFilterMode(Qt::MatchContains);
  ui->ComboBoxClass->setCompleter ( completer );
  ui->ComboBoxClass->setEditable ( true );

  QVariant VarFromList ( ListOfClasses );
  ValidatorAcceptMatch * Validator = new ValidatorAcceptMatch ( VarFromList, this );
  ui->ComboBoxClass->setValidator ( Validator );
}


void dbe::ObjectCreator::SetController()
{
  connect ( ui->FileView, SIGNAL ( stateChanged ( const QString & ) ),
            this, SLOT ( ActiveFileChanged ( const QString & ) ),
            Qt::UniqueConnection );

  connect ( ui->CreateObjectButton, SIGNAL ( clicked() ), this, SLOT ( CreateObject() ),
            Qt::UniqueConnection );

  connect ( ui->CreateOpenObjectButton, SIGNAL ( clicked() ), this, SLOT ( CreateOpenObject() ),
            Qt::UniqueConnection );

  connect ( ui->ExitButton, SIGNAL ( clicked() ), this, SLOT ( close() ),
            Qt::UniqueConnection );

  connect ( ui->LineEditUID, SIGNAL ( textChanged ( QString ) ), this,
            SLOT ( UpdateActions ( QString ) ),
            Qt::UniqueConnection );

  connect ( ui->LineEditUID, SIGNAL ( textEdited ( const QString & ) ),
            this, SLOT ( MustPressReturn ( const QString & ) ),
            Qt::UniqueConnection );

  connect ( this, SIGNAL ( stateChanged() ), this, SLOT ( UpdateActions() ),
            Qt::UniqueConnection );

  connect ( this, SIGNAL ( stateChanged() ), this, SLOT ( SetObjectChanged() ),
            Qt::UniqueConnection );
}

void dbe::ObjectCreator::BuildFileModel()
{
  if ( not confaccessor::db_implementation_name().contains ( "roksconflibs" ) )
  {
    if ( this_files != nullptr )
    {
      delete this_files;
      this_files = nullptr;
    }

    this_files = new FileModel ( confaccessor::ref().GetIncludedFileCache() );

    this_sort.setSourceModel ( this_files );
    ui->FileView->setModel ( &this_sort );
    ui->FileView->sortByColumn ( 2, Qt::DescendingOrder );

    // ui->FileView->HideReadOnlyFilesSlot(true);
    ui->FileView->setColumnHidden(2, true);
    ui->FileView->setColumnHidden(3, true);
    ui->FileView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  }
}

void dbe::ObjectCreator::SetStatusBar()
{
  this_status_bar->setSizeGripEnabled ( false );
  this_status_bar->setAutoFillBackground ( true );
  this_status_bar->showMessage (
    QString ( "Select class, set new object UID, select file where to create object!" ) );
  ui->StatusFrame->setFrameStyle ( QFrame::NoFrame );
  ui->StatusLayout->addWidget ( this_status_bar );
}

void dbe::ObjectCreator::UpdateActions()
{
  bool ReleaseEditor = true;
  this_state = editorOk;

  if ( !ui->LineEditUID->IsValid() || ui->LineEditUID->text().isEmpty() )
  {
    this_state |= uidNotSet;
    ReleaseEditor = false;
    this_newuid = QString();
  }
  else
  {
    this_state &= ~uidNotSet;
    this_newuid = ui->LineEditUID->text();
  }

  if ( this_file_for_new_object.isEmpty() )
  {
    this_state |= fileNotSet;
    ReleaseEditor = false;
  }
  else
  {
    this_state &= ~fileNotSet;
  }

  if ( ReleaseEditor )
  {
    this_associated_editor->setEnabled ( true );
  }

  if ( this_associated_editor != nullptr )
  {
    if ( !this_associated_editor->IsEditorValid() )
    {
      this_state |= nullNotFilled;
    }
    else
    {
      this_state &= ~nullNotFilled;
    }
  }

  if ( this_state == editorOk )
  {
    ui->CreateObjectButton->setEnabled ( true );
    ui->CreateObjectButton->setPalette ( StyleUtility::PaleGreenPalleteButton );

    ui->CreateOpenObjectButton->setEnabled ( true );
    ui->CreateOpenObjectButton->setPalette ( StyleUtility::PaleGreenPalleteButton );

    this_status_bar->setPalette ( QApplication::palette ( this ) );
    this_status_bar->showMessage ( GetMessage() );
  }
  else
  {
    ui->CreateObjectButton->setEnabled ( false );
    ui->CreateOpenObjectButton->setEnabled ( false );
    QPalette buttonAlert;
    buttonAlert.setColor ( QPalette::Disabled, QPalette::Button, QColor ( "grey" ) );
    ui->CreateObjectButton->setPalette ( buttonAlert );
    ui->CreateOpenObjectButton->setPalette ( buttonAlert );
    this_status_bar->setPalette ( StyleUtility::WarningStatusBarPalleteWindow );
    this_status_bar->showMessage ( GetMessage() );
  }
}

void dbe::ObjectCreator::UpdateActions ( QString Dummy )
{
  Q_UNUSED ( Dummy )
  UpdateActions();
}

QString dbe::ObjectCreator::GetMessage()
{
  if ( this_state == editorOk )
    return QString (
             "Now you can create object '%1' of class '%2'." ).arg ( this_newuid ).arg (
             QString ( ( this_object_class.p_name ).c_str() ) );
  else if ( GetState ( classNotSelected | uidNotSet | fileNotSet ) )
    return QString (
             "Select class, set unique ID, select file where to create object and set obligatory attributes and relationships!" );
  else if ( GetState ( classNotSelected ) )
  {
    return QString ( "First select class!" );
  }
  else if ( GetState ( fileNotSet ) )
    return QString (
             "Set active file where to create new object" );
  else if ( GetState ( uidNotSet ) )
  {
    return QString ( "Set the object UID" );
  }
  else if ( GetState ( nullNotFilled ) )
    return QString (
             "All NOT NULL attributes or relationships must be set!" );

  return QString ( "Problem to process Object Create operation!" );
}

bool dbe::ObjectCreator::GetState ( int Flags )
{
  return ( ( this_state & Flags ) == Flags );
}

void dbe::ObjectCreator::BuildContextMenu()
{
  ContextMenu = new QMenu ( this );
  ContextMenu->addAction ( tr ( "&Edit Include Files" ), this, SLOT ( AddInclude() ),
                           QKeySequence ( tr ( "Ctrl+Z" ) ) );
}

void dbe::ObjectCreator::AddInclude()
{}

void dbe::ObjectCreator::FillUidComboBox()
{
  ui->ComboBoxForbiddenUid->clear();

  QStringList listOfUID;
  QString nameOfClass = QString ( this_object_class.p_name.c_str() );

  std::vector<tref> const & vec = dbe::config::api::info::onclass::objects (
                                    nameOfClass.toStdString(),
                                    false );

  for ( size_t i = 0; i < vec.size(); ++i )
  {
    ui->ComboBoxForbiddenUid->addItem ( QString ( vec.at ( i ).UID().c_str() ) );
    listOfUID << QString ( vec.at ( i ).UID().c_str() );
  }

  if ( !listOfUID.isEmpty() )
  {
    ui->ComboBoxForbiddenUid->setToolTip (
      QString ( "Already used object unique ID of class %1 " ).arg ( nameOfClass ) );
    ui->ComboBoxForbiddenUid->show();
  }

  QVariant varFromList ( listOfUID );
  ValidatorAcceptNoMatch * myval = new ValidatorAcceptNoMatch ( varFromList, this );
  ui->LineEditUID->SetValidator ( myval );
}

void dbe::ObjectCreator::CreateOpenObject() {
    CreateObject(true);
}

void dbe::ObjectCreator::CreateObject(bool openEditor)
{
  setCursor ( Qt::WaitCursor );

  BOOST_SCOPE_EXIT(this_)
  {
      this_->unsetCursor();
  }
  BOOST_SCOPE_EXIT_END

  if ( this_file_for_new_object.isEmpty() )
  {
    ERROR ( "Object creation not feasible", "Object file must be selected" )
  }
  else if ( this_newuid.isEmpty() )
  {
    ERROR ( "Object creation not feasible", "Object name (uid) cannot be an empty string" );
  }
  else if ( this_associated_editor )
  {
    this_associated_editor->SetUsedForCopy ( true );

    bool done = this_associated_editor->ParseToCreate ( this_newuid.toStdString(),
                                                        this_file_for_new_object.toStdString() );
    this_associated_editor->SetUsedForCopy ( false );
    this_associated_editor->ResetObjectChanged();
    this_object_changed = false;

    if ( done )
    {
      close();

      if ( openEditor == true ) {
          ( new ObjectEditor ( dbe::inner::dbcontroller::get({this_newuid.toStdString(), this_object_class.p_name}) ) )->show();
      }
    }
  }
}

void dbe::ObjectCreator::SetObjectChanged()
{
  this_object_changed = true;
}

void dbe::ObjectCreator::ActiveFileChanged ( const QString & fname )
{
  if ( fname.isEmpty() or not confaccessor::check_file_rw ( fname ) )
  {
    this_file_for_new_object = QString();
    emit stateChanged();
    return;
  }

  this_file_for_new_object = fname;
  ui->FileView->setToolTip (
    QString ( "Activate file which will store the new object: " ).append (
      this_file_for_new_object ) );
  QString setFilesWithPath = QString ( "Selected File: %1" ).arg ( this_file_for_new_object );
  QString setOnlyFiles = QString ( "Selected File: %1" ).arg (
                           QFileInfo ( this_file_for_new_object ).fileName() );
  ui->FileLabel->setText ( setOnlyFiles );
  ui->FileLabel->setToolTip ( setFilesWithPath );
  ui->FileView->setStyleSheet ( "selection-background-color : rgb(190,238,158)" );
  emit stateChanged();
}

void dbe::ObjectCreator::setup_editor()
{
  if ( ui->ComboBoxClass->IsValid() )
  {

    this_newuid = QString();
    QString cname = QString::fromStdString ( this_object_class.p_name );

    // Create the editor for the new object to be able to modify attributes before creation
    this_associated_editor = new ObjectEditor ( cname.toStdString(), this );
    this_associated_editor->setWindowModality ( Qt::NonModal );
    this_associated_editor->HideDetailWidget ( true );
    this_associated_editor->setDisabled ( true );
    ui->EditorLayout->addWidget ( this_associated_editor );

    {
      // Set appropriately the stretch factor such that contents are resized
      // favoring the object editor side
      QSizePolicy pol = ui->splitter->widget ( 1 )->sizePolicy();
      pol.setHorizontalStretch ( 2 );
//      pol.setHorizontalPolicy(QSizePolicy::Maximum);
      ui->splitter->widget ( 1 )->setSizePolicy ( pol );
    }

    {
      // Set horizontal size of the editor pane to the first widget size
      this_associated_editor->resize ( this_associated_editor->sizeHint() );
      QList<QTableWidget *> const & editor_widgets
      {
        this_associated_editor->findChildren<QTableWidget *>()
      };

      this_associated_editor->setMinimumWidth ( editor_widgets[0]->width() / 2 );

      QRect this_widget_geometry = this->geometry();
      this_widget_geometry.setWidth ( editor_widgets[0]->width() +
                                      ui->ObjectCreatorWidget->width() );
      this->setGeometry ( this_widget_geometry );

    }

    // Set the labels to the class name
    ui->ClassLabel->setText ( QString ( "Selected Class: %1" ).arg ( cname ) );
    setWindowTitle ( QString ( "Create New Object of class %1" ).arg ( cname ) );

    connect ( this_associated_editor, SIGNAL ( WidgetUpdated() ), this,
              SLOT ( UpdateActions() ),
              Qt::UniqueConnection );

    FillUidComboBox();
    ui->ComboBoxForbiddenUid->hide(); /// To anyone wondering this is a list of all the uids that cannot be used(at some point i thought this could be useful to show to the user) // -- that was a very stupid idea!

    QString const & linetext = ui->LineEditUID->text();

    if ( !linetext.isEmpty() )
    {
      ui->LineEditUID->clear();
      ui->LineEditUID->setText ( linetext );
      SetUID();
    }

    emit stateChanged();
    this_object_changed = false;
  }
}

void dbe::ObjectCreator::setup_copy_editor()
{
  if ( ui->ComboBoxClass->IsValid() )
  {
    QString cname = QString::fromStdString ( this_object_class.p_name );

    ui->ClassLabel->setText ( QString ( "Selected Class: %1" ).arg ( cname ) );
    setWindowTitle ( QString ( "Create New Object of Class %1" ).arg ( cname ) );

    this_associated_editor = new ObjectEditor ( *this_src_object, this, true );
    this_associated_editor->setWindowModality ( Qt::NonModal );
    this_associated_editor->SetUsedForCopy ( true );
    this_associated_editor->HideDetailWidget ( true );
    this_associated_editor->setDisabled ( true );

    ui->EditorLayout->addWidget ( this_associated_editor );
    connect ( this_associated_editor, SIGNAL ( WidgetUpdated() ), this,
              SLOT ( UpdateActions() ),
              Qt::UniqueConnection );

    FillUidComboBox();
    ui->ComboBoxForbiddenUid->hide();

    QString tmpSt = ui->LineEditUID->text();

    if ( !tmpSt.isEmpty() )
    {
      ui->LineEditUID->clear();
      ui->LineEditUID->setText ( tmpSt );
      SetUID();
    }

    emit stateChanged();
    this_object_changed = false;
  }
}

void dbe::ObjectCreator::SetUID()
{
  if ( this_file_for_new_object.isEmpty() )
  {
    ERROR ( "Object creation is not feasible", "Database file not selected" );
    return;
  }

  if ( ui->LineEditUID->IsValid() )
  {
    this_newuid = ui->LineEditUID->text();

    QString textForUser = QString ( "New Object ID: %1" ).arg ( this_newuid );
    ui->UidLabel->setText ( textForUser );
  }
  else
  {
    WARN ( "Object creation is not feasible", "Invalid UID provided" );
    return;
  }

  if ( !this_file_for_new_object.isEmpty() )
  {
    UidSet = true;
    this_associated_editor->setDisabled ( false );
    emit stateChanged();
  }
  else
  {
    TLOG_DEBUG(0) <<  "active file empty!! check!!"  ;
  }
}

void dbe::ObjectCreator::MustPressReturn ( const QString & )
{
  QPalette yellow;
  yellow.setColor ( QPalette::Active, QPalette::Window, QColor ( "yellow" ) );

  this_status_bar->setPalette ( yellow );
  this_status_bar->showMessage ( QString ( "Press ENTER to set new UID!" ) );
}

bool dbe::ObjectCreator::CanClose()
{

  if ( ( this_object_changed  and ui->CreateObjectButton->isEnabled() ) or
       ( this_associated_editor and this_associated_editor->WasObjectChanged() ) )
  {
    int ret =
      QMessageBox::question (
        0,
        tr ( "DBE - ObjectCreate" ),
        QString ( "There are pending actions for this dialogue" ),
        QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel );

    if ( ret == QMessageBox::Discard )
    {
      this_object_changed = false;
      this_associated_editor->ResetObjectChanged();

      try
      {
        config::api::commands::delobj (
          inner::dbcontroller::get ( { this_newuid.toStdString(), this_object_class.p_name } ),
          uuid );
      }
      catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
      {
        // nothing specific handle needed in this , it is possible that there is no resul
      }

      return true;
    }
    else if ( ret == QMessageBox::Cancel )
    {
      return false;
    }

    return true;
  }
  else
  {
    return true;
  }
}

void dbe::ObjectCreator::closeEvent ( QCloseEvent * event )
{
  if ( CanClose() )
  {
    if ( this_associated_editor )
    {
      this_associated_editor->close();
      this_associated_editor = nullptr;
    }

    event->accept();
  }
  else
  {
    event->ignore();
  }
}
