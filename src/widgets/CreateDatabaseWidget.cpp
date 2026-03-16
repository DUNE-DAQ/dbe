/// Including QT Headers
#include<QFileDialog>
#include<QMessageBox>
/// Including DBE
#include"dbe/CreateDatabaseWidget.hpp"
#include"dbe/StyleUtility.hpp"
/// Including config headers
#include "conffwk/ConfigObject.hpp"
#include "conffwk/Configuration.hpp"
#include "conffwk/Schema.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/messenger.hpp"
#include "dbe/FileInfo.hpp"

using namespace dunedaq::conffwk;

dbe::CreateDatabaseWidget::CreateDatabaseWidget ( QWidget * parent, bool Include,
                                                  const QString & CreateDir )
  : QWidget ( parent ),
    StatusBar ( nullptr ),
    DirToCreate ( CreateDir ),
    CreateToInclude ( Include )
{
  setupUi ( this );

  connect ( SchemaButton, SIGNAL ( clicked() ), this, SLOT ( DefineSchema() ),
            Qt::UniqueConnection );
  connect ( SelectButton, SIGNAL ( clicked() ), this, SLOT ( DefineDatabaseFile() ),
            Qt::UniqueConnection );

  if ( !CreateToInclude )
  {
    connect ( CreateDatabaseButton, SIGNAL ( clicked() ), this,
              SLOT ( CreateDatabaseFileLoad() ),
              Qt::UniqueConnection );
    connect ( CreateNoLoadDatabaseButton, SIGNAL ( clicked() ), this,
              SLOT ( CreateDatabaseFileNoLoad() ), Qt::UniqueConnection );
  }
  else
  {
    connect ( CreateDatabaseButton, SIGNAL ( clicked() ), this,
              SLOT ( CreateDatabaseFileInclude() ), Qt::UniqueConnection );
    CreateDatabaseButton->setText ( "Create Database File" );
    CreateNoLoadDatabaseButton->hide();
  }

  StatusBar = new QStatusBar ( StatusFrame );
  StatusBar->setSizeGripEnabled ( false );
  StatusFrame->setFrameStyle ( QFrame::NoFrame );
  StatusLayout->addWidget ( StatusBar );

  StatusBar->setAutoFillBackground ( true );
  StatusBar->showMessage ( "Select schema files and define new database file!" );
  CreateDatabaseButton->setDisabled ( true );
  CreateNoLoadDatabaseButton->setDisabled ( true );

  DatabaseName->setReadOnly ( true );
}

void dbe::CreateDatabaseWidget::DefineSchema()
{
  auto fd = new QFileDialog ( this, tr ( "Select Schema File" ),
                              DirToCreate.append("./"),
                              tr ( "XML schema files (*.schema.xml)" ) );
  fd->setFileMode ( QFileDialog::ExistingFile );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptOpen );
  fd->setSidebarUrls(FileInfo::get_path_urls());
  if (!(fd->exec() == QDialog::Accepted)) {
    return;
  }
  auto files = fd->selectedFiles();
  for (auto file: files) {
    QFileInfo SchemaFile = QFileInfo (file);
    if ( !SchemaFile.isFile() ) {
      StatusBar->setPalette ( StyleUtility::AlertStatusBarPallete );
      StatusBar->showMessage ( QString ( "The file is not accessible. Check before usage" ) );
    }
    else {
      if (SchemaFile.fileName().contains("schema")) {
        schema_list->addItem(FileInfo::prune_path(file));
      }
      else {
        StatusBar->setPalette ( StyleUtility::AlertStatusBarPallete );
        StatusBar->showMessage (
          QString ( "The file %1 is not a schema file" ).arg ( SchemaFile.absoluteFilePath() ) );
      }
    }
  }

  if (schema_list->count()>0 && !DatabaseName->text().isEmpty()) {
    CreateDatabaseButton->setEnabled ( true );
    CreateNoLoadDatabaseButton->setEnabled ( true );
  }
}

void dbe::CreateDatabaseWidget::DefineDatabaseFile()
{
  QString Dir;

  if ( DirToCreate.isEmpty() )
  {
    Dir = QString ( "./NewDatabaseFile.data.xml" );
  }
  else
  {
    Dir = DirToCreate.append ( "/NewDatabaseFile.data.xml" );
  }

  auto fd = new QFileDialog ( this, tr("Select new DB File"),
                              Dir,
                              tr("XML data files (*.data.xml)"));
  fd->setFileMode ( QFileDialog::AnyFile );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptSave );
  fd->setSidebarUrls(FileInfo::get_path_urls());
  if (!(fd->exec() == QDialog::Accepted)) {
    return;
  }
  auto files = fd->selectedFiles();

  DatabaseFile = QFileInfo (files.at(0));

  DatabaseName->setText ( DatabaseFile.absoluteFilePath() );

  if (schema_list->count()>0) {
    CreateDatabaseButton->setEnabled ( true );
    CreateNoLoadDatabaseButton->setEnabled ( true );
  }
}

std::list<std::string> dbe::CreateDatabaseWidget::get_includes() {
  std::list<std::string> includes;

  for (int row=0; row<schema_list->count(); ++row) {
    auto item = schema_list->item(row)->text();
    includes.push_back(item.toStdString());
  }
  return includes;
}


void dbe::CreateDatabaseWidget::CreateDatabaseFileLoad()
{
  Configuration db ( "oksconflibs" );
  try {
    const std::string DatabaseName = DatabaseFile.absoluteFilePath().toStdString();
    db.create ( DatabaseName, get_includes());
    db.commit();

    QMessageBox::information (
      0, tr ( "DBE" ),
      QString ( "Database was created.\nNow the DB will be loaded into the Editor!\n" ),
      QMessageBox::Ok );
    emit CanLoadDatabase ( DatabaseFile.absoluteFilePath() );
    close();
  }
  catch ( dunedaq::conffwk::Exception const & ex )
  {
    FAIL ( "Database creation failure", dbe::config::errors::parse ( ex ).c_str() );
  }
}

void dbe::CreateDatabaseWidget::CreateDatabaseFileNoLoad()
{
  Configuration db ( "oksconflibs" );
  try {
    const std::string DatabaseName = DatabaseFile.absoluteFilePath().toStdString();
    db.create (DatabaseName, get_includes());
    db.commit();

    QMessageBox::information ( 0, tr ( "DBE" ), QString ( "The Database was created.\n" ),
                               QMessageBox::Ok );
    close();
  }
  catch ( dunedaq::conffwk::Exception const & ex )
  {
    FAIL ( "Database creation failure", dbe::config::errors::parse ( ex ).c_str() );
  }
}

void dbe::CreateDatabaseWidget::CreateDatabaseFileInclude()
{
  Configuration db ( "oksconflibs" );
  try {
    const std::string DatabaseName = DatabaseFile.absoluteFilePath().toStdString();
    db.create (DatabaseName, get_includes());
    db.commit();

    QMessageBox::information ( 0, tr ( "DBE" ), QString ( "The Database was created.\n" ),
                               QMessageBox::Ok );
    emit CanIncludeDatabase ( DatabaseFile.absoluteFilePath() );
    close();
  }
  catch ( dunedaq::conffwk::Exception const & ex )
  {
    FAIL ( "Database creation error", dbe::config::errors::parse ( ex ).c_str() );
  }
}
