/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

#include "dbe/IncludeFileWidget.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/StyleUtility.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/config_api_get.hpp"
#include "dbe/config_api_commands.hpp"
#include "ui_IncludeFileWidget.h"
#include "dbe/messenger.hpp"
#include "dbe/MainWindow.hpp"

#include <QUrl>
#include <QApplication>

#include <boost/scope_exit.hpp>
#include <cstdlib>

dbe::IncludeFileWidget::~IncludeFileWidget() = default;

dbe::IncludeFileWidget::IncludeFileWidget ( QString FilePath, QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::IncludeFileWidget ),
    CreateWidget ( nullptr ),
    DatabasePath ( FilePath ),
    Removed ( false ),
    StatusBar ( nullptr ),
    SelectFile ( nullptr )
{
  ui->setupUi ( this );

  /// Visual
  ui->AddLabel->setStyleSheet (
    "QLabel{background-color:#8DAA2E; color:#facd64; font:bold14px; border-style:outset; border-width:2px; border-radius:10px;"
    "border-color:beige; font:bold14px; min-width:10em; padding: 6px;}" );
  ui->RemoveLabel->setStyleSheet (
    "QLabel{background-color:#b5351b; color:#facd64; font:bold14px; border-style:outset;"
    "border-width:2px; border-radius:10px; border-color:beige; font:bold14px; min-width:10em; padding:6px;}" );
  setWindowTitle (
    QString ( "Edit Included Files for Database : %1" ).arg ( QFileInfo (
                                                                FilePath ).fileName() ) );

  StatusBar = new QStatusBar ( ui->StatusFrame );
  StatusBar->setSizeGripEnabled ( false );
  ui->StatusFrame->setFrameStyle ( QFrame::NoFrame );
  ui->StatusLayout->addWidget ( StatusBar );

  StatusBar->setAutoFillBackground ( true );
  StatusBar->showMessage ( "Select files to include or to remove from include!" );

  SetRemoveComboBox();

  SelectFile = new QFileDialog ( this, tr ( "Open File" ), ".", tr ( "XML files (*.xml)" ) );
  SelectFile->setFileMode ( QFileDialog::ExistingFiles );
  SelectFile->setViewMode ( QFileDialog::Detail );
  SelectFile->setAcceptMode ( QFileDialog::AcceptOpen );

  QString TDAQ_DB_REPOSITORY = getenv ( "TDAQ_DB_REPOSITORY" );
  if(TDAQ_DB_REPOSITORY.isEmpty() == false) {
      QString TDAQ_DB_USER_REPOSITORY = getenv ( "TDAQ_DB_USER_REPOSITORY" );
      FolderPathList = TDAQ_DB_USER_REPOSITORY.split ( ":", Qt::SkipEmptyParts );

      FolderPathList.append(dbe::MainWindow::findthis()->find_db_repository_dir());
  } else {
      QString DUNEDAQ_DB_PATH = getenv ( "DUNEDAQ_DB_PATH" );
      FolderPathList = DUNEDAQ_DB_PATH.split ( ":", Qt::SkipEmptyParts );
  }

  for ( QString & PathName : FolderPathList )
  {
    if ( !PathName.endsWith ( "/" ) )
    {
      PathName.append ( "/" );
    }
  }

  QList<QUrl> List = SelectFile->sidebarUrls();

  if ( !FolderPathList.isEmpty() )
  {
    dbPath = FolderPathList;

    for ( const QString & Dir : dbPath )
    {
      ui->DirectoryCombo->addItem ( Dir );
      QString TDAqDir = QString ( "file://" ).append ( Dir );
      QUrl URL = QUrl ( TDAqDir );
      List << URL;
    }
  }

  SelectFile->setSidebarUrls ( List );
  SelectFile->setOption (QFileDialog::DontResolveSymlinks, true );
  ui->AddToIncludeButton->setDisabled ( true );
  ui->RemoveButton->setDisabled ( true );

  ui->AddFileLine->setSelectionMode(QAbstractItemView::NoSelection);

  SetController();
}


void dbe::IncludeFileWidget::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}


void dbe::IncludeFileWidget::SetRemoveComboBox()
{
  QStringList IncludeList ( dbe::config::api::get::file::inclusions_singlefile (
                              DatabasePath ) );

  if ( !IncludeList.isEmpty() )
  {
    if ( IncludeList.contains ( DatabasePath ) )
    {
      IncludeList.removeOne ( DatabasePath );
    }

    ui->RemoveCombo->clear();
    ui->RemoveCombo->addItems ( IncludeList );
  }
}

void dbe::IncludeFileWidget::SetController()
{
  connect ( ui->SelectFileButton, SIGNAL ( clicked() ), this, SLOT ( SelectFileToInclude() ),
            Qt::UniqueConnection );
  connect ( ui->CreateFileButton, SIGNAL ( clicked() ), this, SLOT ( CreateFileToInclude() ),
            Qt::UniqueConnection );
  connect ( ui->AddToIncludeButton, SIGNAL ( clicked() ), this, SLOT ( AddFileToInclude() ),
            Qt::UniqueConnection );
  connect ( ui->RemoveButton, SIGNAL ( clicked() ), this, SLOT ( RemoveFileFromInclude() ),
            Qt::UniqueConnection );
  connect ( ui->RemoveCombo, SIGNAL ( activated ( int ) ), this,
            SLOT ( RemoveFileFromInclude ( int ) ),
            Qt::UniqueConnection );
  connect ( ui->DirectoryCombo, SIGNAL ( activated ( const QString & ) ), this,
            SLOT ( SetDirectory ( const QString & ) ), Qt::UniqueConnection );
  connect ( ui->ExitButton, SIGNAL ( clicked() ), this, SLOT ( close() ) );
}

void dbe::IncludeFileWidget::SelectFileToInclude()
{
  ui->AddFileLine->clear();
  QStringList Files;

  if ( !Directory.isEmpty() )
  {
    SelectFile->setDirectory ( Directory );
  }

  if ( SelectFile->exec() )
  {
    Files = SelectFile->selectedFiles();
  }

  if(Files.isEmpty()) {
      return;
  }

  for(const QString& FileToInclude : Files) {
      QFileInfo f(FileToInclude);

      if ( Directory.isEmpty() )
      {
          ui->AddFileLine->addItem(f.absoluteFilePath());
      }
      else
      {
          ui->AddFileLine->addItem(f.absoluteFilePath().remove ( Directory + QString ( '/' ), Qt::CaseSensitive ));
      }
  }

  CheckInclude();
}

void dbe::IncludeFileWidget::AddFileToInclude()
{
    BOOST_SCOPE_EXIT(void)
    {
        QApplication::restoreOverrideCursor();
    }
    BOOST_SCOPE_EXIT_END

    QApplication::setOverrideCursor(Qt::WaitCursor);

    for(int i = 0; i < ui->AddFileLine->count(); ++i) {
        QString File = ui->AddFileLine->item(i)->text();

        if ( !File.isEmpty() )
        {
            /// Need to strip file....
            for ( QString & PathFile : FolderPathList )
            {
              if ( File.startsWith ( PathFile ) )
              {
                File = File.replace ( 0, PathFile.size(), "" );
                break;
              }
            }

            config::api::commands::file::add ( DatabasePath, File );

            StatusBar->setPalette ( QApplication::palette ( this ) );
            StatusBar->showMessage (
              QString ( "The file %1 was added to the included files" ).arg ( File ) );
        }
    }

    SetRemoveComboBox();

    ui->AddFileLine->clear();
    ui->AddToIncludeButton->setDisabled ( true );
}

void dbe::IncludeFileWidget::AddNewFileToInclude ( const QString & File )
{
  ui->AddFileLine->clear();

  QString includeFilePath;
  includeFilePath = File;

  if ( includeFilePath.isEmpty() )
  {
    return;
  }

  QFileInfo includeFile = QFileInfo ( includeFilePath );

  if ( !includeFile.isFile() )
  {
    StatusBar->setPalette ( StyleUtility::AlertStatusBarPallete );
    StatusBar->showMessage ( QString ( "The file is NOT accessible. Check before usage" ) );
  }
  else
  {
    QString fileToInclude;

    if ( Directory.isEmpty() )
    {
      fileToInclude = includeFile.absoluteFilePath();
    }
    else if ( ( includeFile.absoluteDir().canonicalPath() ).compare (
                Directory ) == 0 ) fileToInclude =
                    includeFile.absoluteFilePath().remove ( Directory + QString ( '/' ), Qt::CaseSensitive );
    else
    {
      fileToInclude = includeFile.absoluteFilePath();
    }

    ui->AddFileLine->addItem( fileToInclude );
    ui->AddToIncludeButton->setDisabled(false);
  }
}

void dbe::IncludeFileWidget::RemoveFileFromInclude()
{
  if ( ui->RemoveCombo->currentIndex() != -1 )
  {
    BOOST_SCOPE_EXIT(void)
    {
        QApplication::restoreOverrideCursor();
    }
    BOOST_SCOPE_EXIT_END

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString File = ui->RemoveCombo->currentText();

    config::api::commands::file::remove ( DatabasePath, File );
    Removed = true;
    StatusBar->setPalette ( QApplication::palette ( this ) );
    StatusBar->showMessage (
      QString ( "The file %1 was removed from the included files" ).arg ( File ) );
    SetRemoveComboBox();
  }
  else
  {
    StatusBar->setPalette ( StyleUtility::AlertStatusBarPallete );
    StatusBar->showMessage (
      QString ( "The file is NOT selected. Use combo box to select one." ) );
  }
}

void dbe::IncludeFileWidget::RemoveFileFromInclude ( int )
{
  if ( ui->RemoveCombo->currentIndex() != -1 )
  {
    ui->RemoveButton->setEnabled ( true );
    StatusBar->setPalette ( QApplication::palette ( this ) );
    StatusBar->showMessage (
      QString ( "The file %1 will be removed from the included files of %2" ).arg (
        QFileInfo ( ui->RemoveCombo->currentText() ).fileName() ).arg (
        QFileInfo ( DatabasePath ).fileName() ) );
  }
}

void dbe::IncludeFileWidget::SetDirectory ( const QString & Dir )
{
  if(dbPath.contains(Dir, Qt::CaseSensitivity::CaseSensitive) && QDir(Dir).exists()) {
    Directory = Dir;
    SelectFile->setDirectory ( Directory );
  } else {
    Directory.clear();
    SelectFile->setDirectory(QDir("."));
  }
}

void dbe::IncludeFileWidget::CheckInclude ()
{
  SetRemoveComboBox();

  if ( ui->AddFileLine->count() == 0 )
  {
    StatusBar->setPalette ( StyleUtility::AlertStatusBarPallete );
    StatusBar->showMessage ( QString ( "Select existing file!" ) );
    ui->AddToIncludeButton->setDisabled ( true );

    return;
  }

  std::vector<int> removedItems;
  for(int i = 0; i < ui->AddFileLine->count(); ++i) {
      const QString inc = ui->AddFileLine->item(i)->text();

      if(inc == DatabasePath) {
          removedItems.push_back(i);
      } else {
          for(int j = 0; j < ui->RemoveCombo->count(); ++j) {
              if(ui->RemoveCombo->itemText(j).compare(inc) == 0) {
                  removedItems.push_back(i);
              }
          }
      }
  }

  if(removedItems.empty() == false) {
      QString message = "The following files will not be included because already included:";

      for(int i : removedItems) {
          QListWidgetItem* item = ui->AddFileLine->takeItem(i);
          message += "\n- " + item->text();
          delete item;
      }

      WARN("File inclusion issue", message.toStdString());
  }

  ui->AddToIncludeButton->setDisabled((ui->AddFileLine->count() == 0) ? true : false);
}

void dbe::IncludeFileWidget::CreateFileToInclude()
{
  CreateWidget = new CreateDatabaseWidget ( nullptr, true, Directory );
  connect ( CreateWidget, SIGNAL ( CanIncludeDatabase ( const QString & ) ), this,
            SLOT ( AddNewFileToInclude ( const QString & ) ), Qt::UniqueConnection );
  CreateWidget->show();
}
