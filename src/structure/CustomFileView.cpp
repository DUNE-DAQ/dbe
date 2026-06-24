// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/// Including QT Headers
#include "dbe/FileInfo.hpp"
#include "dbe/ui_constants.hpp"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QShortcut>
/// Including DBE
#include "dbe/CustomFileView.hpp"
#include "dbe/IncludeFileWidget.hpp"

dbe::CustomFileView::CustomFileView ( QWidget * parent )
  : QTableView ( parent ),
    ContextMenu ( nullptr ),
    LaunchIncludeEditor ( nullptr ),
    HideReadOnlyFiles ( nullptr ),
    FindFile ( nullptr ),
    FindFileDialog ( nullptr ),
    LineEdit ( nullptr ),
    NextButton ( nullptr ),
    GoButton ( nullptr )
{
  ConnectActions();
  CreateContextMenu();
  setSelectionBehavior ( SelectionBehavior::SelectRows );
  horizontalHeader()->setSectionResizeMode ( QHeaderView::ResizeMode::Stretch );
  setSortingEnabled ( true );

  connect ( this, SIGNAL(activated(QModelIndex)),
            this, SLOT(file_info_slot(QModelIndex)) );
}

void dbe::CustomFileView::CreateActions()
{
  LaunchIncludeEditor = new QAction ( tr ( "Add/Remove Files" ), this );
  LaunchIncludeEditor->setShortcutContext ( Qt::WidgetShortcut );
  connect ( LaunchIncludeEditor, SIGNAL ( triggered() ), this,
            SLOT ( LaunchIncludeEditorSlot() ) );
  ContextMenu->addAction ( LaunchIncludeEditor );

  m_file_info_action = new QAction (tr ("File &information"));
  connect ( m_file_info_action, SIGNAL(triggered()),
            this, SLOT(file_info_slot()) );
  ContextMenu->addAction ( m_file_info_action );

  HideReadOnlyFiles = new QAction ( tr ( "Hide Read Only Files" ), this );
  HideReadOnlyFiles->setShortcutContext ( Qt::WidgetShortcut );
  HideReadOnlyFiles->setCheckable ( true );
  HideReadOnlyFiles->setChecked ( false );
  connect ( HideReadOnlyFiles, SIGNAL ( triggered ( bool ) ), this,
            SLOT ( HideReadOnlyFilesSlot ( bool ) ) );
  ContextMenu->addAction ( HideReadOnlyFiles );


  FindFile = new QAction ( tr ( "Find File" ), this );
  FindFile->setShortcut ( QKeySequence ( tr ( "Ctrl+F" ) ) );
  FindFile->setShortcutContext ( Qt::WidgetShortcut );
  connect ( FindFile, SIGNAL ( triggered() ), this, SLOT ( FindFileSlot() ) );
//  connect ( Shortcut, SIGNAL ( activated() ), this, SLOT ( FindFileSlot() ) );
  ContextMenu->addAction ( FindFile );
}

void dbe::CustomFileView::ConnectActions()
{
  connect ( this, SIGNAL ( clicked ( QModelIndex ) ), this,
            SLOT ( ChangeSelection ( QModelIndex ) ) );
}

void dbe::CustomFileView::CreateContextMenu()
{
  if ( ContextMenu == nullptr )
  {
    ContextMenu = new QMenu ( this );
    CreateActions();
  }
}

void dbe::CustomFileView::contextMenuEvent ( QContextMenuEvent * Event )
{
  QModelIndex index = indexAt ( Event->pos() );

  if ( index.isValid() )
  {
    if ( model()->data ( model()->index ( index.row(), 2 ) ).toString() == "RO" ) {
      ContextMenu->actions().at ( 0 )->setVisible ( false );
    }
    else {
      ContextMenu->actions().at ( 0 )->setVisible ( true );
    }
    ContextMenu->actions().at ( 1 )->setVisible ( true );

    setCurrentIndex ( index );
    selectionModel()->setCurrentIndex ( index, QItemSelectionModel::NoUpdate );
  }
  else {
      ContextMenu->actions().at(0)->setVisible(false);
      ContextMenu->actions().at(1)->setVisible(false);
  }
  ContextMenu->exec ( Event->globalPos() );
}

void dbe::CustomFileView::GoToFile()
{
  ListIndex = 0;
  ListOfMatch.clear();

  QString UserType = LineEdit->text();

  if ( UserType.isEmpty() )
  {
    return;
  }

  QAbstractItemModel * Model = model();

  if ( Model != nullptr )
  {
    QVariant StringCriterium = QVariant ( UserType );

    QModelIndex find_by_name = Model->index ( 0, 0 );
    QModelIndex find_by_path = Model->index ( 0, 1 );

    Qt::MatchFlags options = Qt::MatchContains;

    if ( CaseSensitiveCheckBox->isChecked() )
    {
      options = options | Qt::MatchCaseSensitive;
    }

    if ( WholeWordCheckBox->isChecked() )
    {
      options = Qt::MatchExactly;
    }

    ListOfMatch = Model->match ( find_by_name, Qt::DisplayRole, StringCriterium, 1000,
                                 options );

    ListOfMatch.append (
      Model->match ( find_by_path, Qt::DisplayRole, StringCriterium, 1000, options ) );

    if ( ListOfMatch.size() > 0 )
    {
      ListIndex = 0;
      scrollTo ( ListOfMatch.value ( ListIndex ), QAbstractItemView::EnsureVisible );
      selectRow ( ListOfMatch.value ( ListIndex ).row() );
      resizeColumnToContents ( ListIndex );

      GoButton->setDisabled ( true );
      NextButton->setEnabled ( true );

      disconnect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( GoToFile() ) );
      connect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( GoToNext() ) );
      ChangeSelection ( ListOfMatch.at ( ListIndex ) );
    }
  }
}

void dbe::CustomFileView::GoToNext()
{
  if ( ( LineEdit->text() ).isEmpty() )
  {
    ListIndex = 0;
    ListOfMatch.clear();
    return;
  }

  if ( ListOfMatch.size() > 0 )
  {
    if ( ( ++ListIndex ) < ListOfMatch.size() )
    {
      scrollTo ( ListOfMatch.value ( ListIndex ), QAbstractItemView::EnsureVisible );
      selectRow ( ListOfMatch.value ( ListIndex ).row() );
      resizeColumnToContents ( ListIndex );
      ChangeSelection ( ListOfMatch.at ( ListIndex ) );
    }
    else
    {
      ListIndex = 0;
      scrollTo ( ListOfMatch.value ( ListIndex ), QAbstractItemView::EnsureVisible );
      selectRow ( ListOfMatch.value ( ListIndex ).row() );
      resizeColumnToContents ( ListIndex );
      ChangeSelection ( ListOfMatch.at ( ListIndex ) );
    }
  }
}

void dbe::CustomFileView::FindFileSlot()
{
  if ( FindFileDialog != nullptr )
  {
    delete FindFileDialog;
    FindFileDialog = nullptr;
  }

  FindFileDialog = new QDialog ( this );
  FindFileDialog->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
  FindFileDialog->setToolTip ( "Type string to edit line and press Enter." );
  FindFileDialog->setWindowTitle ( "Search for a file name in the table" );

  QHBoxLayout * FileLayout = new QHBoxLayout();
  QLabel * Label = new QLabel ( QString ( "Find File:" ) );
  NextButton = new QPushButton ( "Next" );
  GoButton = new QPushButton ( "Go !" );
  LineEdit = new QLineEdit();
  LineEdit->setToolTip ( "Type string and press Enter" );

  FileLayout->addWidget ( Label );
  FileLayout->addWidget ( LineEdit );
  FileLayout->addWidget ( GoButton );
  FileLayout->addWidget ( NextButton );

  QHBoxLayout * ButtonsLayout = new QHBoxLayout();
  WholeWordCheckBox = new QCheckBox ( "Whole String" );
  CaseSensitiveCheckBox = new QCheckBox ( "Case Sensitive" );

  ButtonsLayout->addWidget ( WholeWordCheckBox );
  ButtonsLayout->addWidget ( CaseSensitiveCheckBox );

  QVBoxLayout * FindDialogLayout = new QVBoxLayout();
  FindDialogLayout->addItem ( FileLayout );
  FindDialogLayout->addItem ( ButtonsLayout );

  FindFileDialog->setLayout ( FindDialogLayout );
  FindFileDialog->show();
  //NextButton->setDisabled(true);

  connect ( LineEdit, SIGNAL ( textEdited ( QString ) ), this,
            SLOT ( EditedSearchString ( QString ) ) );
  connect ( WholeWordCheckBox, SIGNAL ( clicked() ), this, SLOT ( EditedSearchString() ) );
  connect ( CaseSensitiveCheckBox, SIGNAL ( clicked() ), this,
            SLOT ( EditedSearchString() ) );

  connect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( GoToFile() ) );
  connect ( GoButton, SIGNAL ( clicked() ), this, SLOT ( GoToFile() ) );
  connect ( NextButton, SIGNAL ( clicked() ), this, SLOT ( GoToNext() ) );
}

void dbe::CustomFileView::file_info_slot() {
  file_info_slot(currentIndex());
}

void dbe::CustomFileView::file_info_slot(QModelIndex index) {
  QString file = model()->data ( model()->index ( index.row(), 0,
                                                  index.parent() ) ).toString();
  QString path = model()->data ( model()->index ( index.row(), 1,
                                                  index.parent() ) ).toString();
  auto full_name = path + "/" + file;

  FileInfoWidget::show_file_info(full_name);
}

void dbe::CustomFileView::LaunchIncludeEditorSlot()
{
  QModelIndex index = currentIndex();
  QString file = model()->data ( model()->index ( index.row(), 0,
                                                  index.parent() ) ).toString();
  QString path = model()->data ( model()->index ( index.row(), 1,
                                                  index.parent() ) ).toString();
  auto full_name = path + "/" + file;
  auto * file_widget = new IncludeFileWidget ( full_name );

  file_widget->show();
}

void dbe::CustomFileView::HideReadOnlyFilesSlot ( bool Hide )
{
  for ( int i = 0; i < model()->rowCount(); ++i )
  {
    if ( model()->data ( model()->index ( i, 2 ) ).toString() == "RO" )
    {
      if ( Hide )
      {
        hideRow ( i );
      }
      else
      {
        showRow ( i );
      }
    }
  }
}

void dbe::CustomFileView::EditedSearchString ( QString Text )
{
  Q_UNUSED ( Text )

  connect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( GoToFile() ) );
  disconnect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( GoToNext() ) );

  GoButton->setEnabled ( true );
  NextButton->setDisabled ( true );
}

void dbe::CustomFileView::EditedSearchString()
{
  QString Dummy ( "Dummy" );
  EditedSearchString ( Dummy );
}

void dbe::CustomFileView::ChangeSelection ( QModelIndex index )
{
  QString file = model()->data ( model()->index ( index.row(), 0,
                                                  index.parent() ) ).toString();
  QString path = model()->data ( model()->index ( index.row(), 1,
                                                  index.parent() ) ).toString();
  auto full_name = path + "/" + file;
  emit stateChanged ( full_name );
}
