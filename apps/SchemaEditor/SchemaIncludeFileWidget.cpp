#include "dbe/SchemaIncludeFileWidget.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/StyleUtility.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/config_api_get.hpp"
#include "dbe/config_api_commands.hpp"
#include "ui_SchemaIncludeFileWidget.h"
#include "dbe/messenger.hpp"
#include "dbe/SchemaKernelWrapper.hpp"

#include "dbe/SchemaCustomFileModel.hpp"
#include "dbe/SchemaCustomTableModel.hpp"


#include <QUrl>
#include <QApplication>
#include <QMessageBox>

#include <boost/scope_exit.hpp>
#include <cstdlib>

using namespace dunedaq;
using namespace dunedaq::oks;

dbse::SchemaIncludeFileWidget::~SchemaIncludeFileWidget() = default;

dbse::SchemaIncludeFileWidget::SchemaIncludeFileWidget ( QString FilePath, QWidget * parent )
  : QWidget ( parent ),
    ui ( new dbse::Ui::SchemaIncludeFileWidget ),
    // CreateWidget ( nullptr ),
    CurrentFile ( FilePath ),
    Removed ( false ),
    StatusBar ( nullptr ),
    SelectFile ( nullptr )
{
  ui->setupUi ( this );

  /// Visual
  // ui->AddLabel->setStyleSheet (
  //   "QLabel{background-color:#8DAA2E; color:#facd64; font:bold14px; border-style:outset; border-width:2px; border-radius:10px;"
  //   "border-color:beige; font:bold14px; min-width:10em; padding: 6px;}" );
  // ui->RemoveLabel->setStyleSheet (
  //   "QLabel{background-color:#b5351b; color:#facd64; font:bold14px; border-style:outset;"
  //   "border-width:2px; border-radius:10px; border-color:beige; font:bold14px; min-width:10em; padding:6px;}" );
  setWindowTitle (
    QString ( "Edit Included Files for schema file : %1" ).arg ( QFileInfo (
                                                                FilePath ).fileName() ) );
  StatusBar = new QStatusBar ( ui->StatusFrame );
  StatusBar->setSizeGripEnabled ( false );
  ui->StatusFrame->setFrameStyle ( QFrame::NoFrame );
  ui->StatusLayout->addWidget ( StatusBar );

  StatusBar->setAutoFillBackground ( true );
  StatusBar->showMessage ( "Select files to include or to remove from include!" );

  SelectFile = new QFileDialog ( this, tr ( "Open File" ), ".", tr ( "XML files (*.xml)" ) );
  SelectFile->setFileMode ( QFileDialog::ExistingFiles );
  SelectFile->setViewMode ( QFileDialog::Detail );
  SelectFile->setAcceptMode ( QFileDialog::AcceptOpen );

  QString DUNEDAQ_DB_PATH = getenv ( "DUNEDAQ_DB_PATH" );
  FolderPathList = DUNEDAQ_DB_PATH.split ( QLatin1Char(':'), Qt::SkipEmptyParts );


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
  SelectFile->setOption(QFileDialog::DontResolveSymlinks, true);
  ui->AddToIncludeButton->setDisabled ( true );
  // ui->RemoveButton->setDisabled ( true );
  ui->SaveButton->setDisabled ( true );

  ui->AddFileLine->setSelectionMode(QAbstractItemView::NoSelection);

  SetCurrentIncludeList();

  SetController();
}

void dbse::SchemaIncludeFileWidget::SetCurrentIncludeList()
{
  std::set<std::string> includes;
  dbse::KernelWrapper::GetInstance().GetIncludedList ( CurrentFile.toStdString(),
                                                       includes );
  QStringList IncludeList;
  for (auto file: includes) {
    std::cout << "Include file = <" << file << ">\n";
    QString File = QString::fromStdString ( file );
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
      if (!RemovedFileList.contains(File)) {
        IncludeList.append ( File );
      }
    }
  }
  if ( !IncludeList.isEmpty() )
  {
    if ( IncludeList.contains ( CurrentFile ) )
    {
      IncludeList.removeOne ( CurrentFile );
    }

    ui->CurrentIncludeList->clear();
    ui->CurrentIncludeList->addItems ( IncludeList );
  }
}


void dbse::SchemaIncludeFileWidget::SetController()
{
  connect ( ui->SelectFileButton, SIGNAL ( clicked() ), this, SLOT ( SelectFileToInclude() ),
            Qt::UniqueConnection );
  connect ( ui->CreateFileButton, SIGNAL ( clicked() ), this, SLOT ( CreateFileToInclude() ),
            Qt::UniqueConnection );
  connect ( ui->AddToIncludeButton, SIGNAL ( clicked() ), this, SLOT ( AddFileToInclude() ),
            Qt::UniqueConnection );
  connect ( ui->RemoveButton, SIGNAL ( clicked() ), this, SLOT ( RemoveFileFromInclude() ),
            Qt::UniqueConnection );
  connect ( ui->DirectoryCombo, SIGNAL ( activated ( const QString & ) ), this,
            SLOT ( SetDirectory ( const QString & ) ), Qt::UniqueConnection );
  connect ( ui->SaveButton, SIGNAL ( clicked() ), this, SLOT ( SaveSchema() ) );
  connect ( ui->ExitButton, SIGNAL ( clicked() ), this, SLOT ( close() ) );
}

void dbse::SchemaIncludeFileWidget::SaveSchema()
{
  KernelWrapper::GetInstance().SaveSchema( CurrentFile.toStdString() );

  StatusBar->showMessage (
    QString ( "Schema file %1 saved" ).arg ( CurrentFile ) );
  ui->SaveButton->setEnabled ( false );
  RemovedFileList.clear();
  SetCurrentIncludeList();

  emit files_updated();
}
void dbse::SchemaIncludeFileWidget::SelectFileToInclude()
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

void dbse::SchemaIncludeFileWidget::AddFileToInclude()
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

            QString message;
            try {
              KernelWrapper::GetInstance().AddInclude(
                CurrentFile.toStdString(), File.toStdString() );
              message = QString (
                "The file %1 was added to the included files" ).arg ( File );
            }
            catch (std::exception& exc) {
              message = QString (
                "Failed to add %1 to included files, %2" ).arg(File).arg(exc.what());
            }
            StatusBar->setPalette ( QApplication::palette ( this ) );
            StatusBar->showMessage (
              QString ( "The file %1 was added to the included files" ).arg ( File ) );
        }
    }



    ui->AddFileLine->clear();
    ui->AddToIncludeButton->setDisabled ( true );
    ui->SaveButton->setEnabled ( true );

    SetCurrentIncludeList();

  emit files_updated();
}

void dbse::SchemaIncludeFileWidget::AddNewFileToInclude ( const QString & File )
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

    emit files_updated();
  }
}

void dbse::SchemaIncludeFileWidget::RemoveFileFromInclude()
{
  auto selected = ui->CurrentIncludeList->selectedItems();
  if ( selected.size() != 0 )
  {
    BOOST_SCOPE_EXIT(void)
    {
        QApplication::restoreOverrideCursor();
    }
    BOOST_SCOPE_EXIT_END

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString removedFiles;
    QString verb = "was";
    for ( auto Item : selected) {
      QString File = Item->text();
      if (removedFiles.size() >0) {
        removedFiles.append(", ");
        verb = "were";
      }
      std::cout << "File " << File.toStdString() << " being removed\n";
      KernelWrapper::GetInstance().RemoveInclude( CurrentFile.toStdString(),
                                                  File.toStdString() );
      removedFiles.append(File);
      RemovedFileList.append(File);
    }
    Removed = true;
    StatusBar->setPalette ( QApplication::palette ( this ) );
    StatusBar->showMessage (
      QString ( "The files %1 %2 removed from the included files" ).arg ( removedFiles ).arg(verb) );
    ui->SaveButton->setEnabled ( true );
    SetCurrentIncludeList();

    emit files_updated();
  }
  else
  {
    StatusBar->setPalette ( StyleUtility::AlertStatusBarPallete );
    StatusBar->showMessage (
      QString ( "There is NO file selected. Use file list to select one." ) );
  }
}

void dbse::SchemaIncludeFileWidget::RemoveFileFromInclude ( int )
{
  auto selected = ui->CurrentIncludeList->selectedItems();
  if ( selected.size() != 0 )
  {
    QString files;
    for ( auto Item : selected) {
      if (files.size() >0) {
        files.append(", ");
      }
      files.append(Item->text());
    }
    ui->RemoveButton->setEnabled ( true );
    StatusBar->setPalette ( QApplication::palette ( this ) );
    StatusBar->showMessage (
      QString ( "The file(s) %1 will be removed from the included files of %2" ).arg (
        files ).arg ( QFileInfo ( CurrentFile ).fileName() ) );
  }
}

void dbse::SchemaIncludeFileWidget::SetDirectory ( const QString & Dir )
{
  if(dbPath.contains(Dir, Qt::CaseSensitivity::CaseSensitive) && QDir(Dir).exists()) {
    SelectFile->setDirectory ( Dir );
  } else {
    Directory.clear();
    SelectFile->setDirectory(QDir("."));
  }
}

void dbse::SchemaIncludeFileWidget::CheckInclude ()
{

  if ( ui->AddFileLine->count() == 0 )
  {
    StatusBar->setPalette ( StyleUtility::AlertStatusBarPallete );
    StatusBar->showMessage ( QString ( "Select existing file!" ) );
    ui->AddToIncludeButton->setDisabled ( true );

    return;
  }


  ui->AddToIncludeButton->setDisabled((ui->AddFileLine->count() == 0) ? true : false);
}

void dbse::SchemaIncludeFileWidget::CreateFileToInclude()
{
  QString FileName = QFileDialog::getSaveFileName ( this, tr ( "New schema File" ) );

  if ( FileName.isEmpty() )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Please provide a name for the schema !" ) );
    return;
  }

  if ( !FileName.endsWith ( ".schema.xml" ) )
  {
    FileName.append ( ".schema.xml" );
  }

  QFile FileInfo ( FileName );
  std::string FileNameStd = FileInfo.fileName().toStdString();

  try
  {
    auto save_active = KernelWrapper::GetInstance().GetActiveSchema();
    KernelWrapper::GetInstance().CreateNewSchema ( FileNameStd );
    KernelWrapper::GetInstance().SaveSchema ( FileNameStd );
    KernelWrapper::GetInstance().SetActiveSchema(save_active);
    KernelWrapper::GetInstance().AddInclude(
      CurrentFile.toStdString(), FileNameStd );
    StatusBar->showMessage (
      QString ( "The file %1 was added to the included files" ).arg ( FileInfo.fileName())
      );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning (
      0,
      "Schema editor",
      QString ( "Could not create file : %1.\n\n%2" ).arg ( QString::fromStdString (
                                                              FileNameStd ) ).arg (
        QString ( Ex.what() ) ) );
  }

  ui->SaveButton->setEnabled ( true );

  SetCurrentIncludeList();

    emit files_updated();


}
