// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

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


  m_instructions->setText ( "Select schema files and define new database file. Database must contain at least one schema file, other includes may be added later via a File info window." );
  CreateDatabaseButton->setDisabled ( true );
  CreateNoLoadDatabaseButton->setDisabled ( true );

  DatabaseName->setReadOnly ( true );
}

void dbe::CreateDatabaseWidget::DefineSchema()
{
  auto fd = new QFileDialog ( this, tr ( "Select Schema File" ),
                              FileInfo::get_schema_path(),
                              tr ( "XML schema files (*.schema.xml)" ) );
  fd->setFileMode ( QFileDialog::ExistingFile );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptOpen );
  fd->setSidebarUrls(FileInfo::get_path_urls());
  if (!(fd->exec() == QDialog::Accepted)) {
    return;
  }
  FileInfo::set_schema_path(fd->directory().path());

  auto files = fd->selectedFiles();
  for (auto file: files) {
    QFileInfo SchemaFile = QFileInfo (file);
    if ( !SchemaFile.isFile() ) {
      m_instructions->setPalette ( StyleUtility::AlertStatusBarPallete );
      m_instructions->setText ( QString ( "The file is not accessible. Check before usage" ));
    }
    else {
      if (SchemaFile.fileName().contains("schema")) {
        schema_list->addItem(FileInfo::prune_path(file));
      }
      else {
        m_instructions->setPalette ( StyleUtility::AlertStatusBarPallete );
        m_instructions->setText (
          QString ( "The file %1 is not a schema file" ).arg ( SchemaFile.absoluteFilePath() ));
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

  QString path = FileInfo::get_data_path().append ( "/NewDatabaseFile.data.xml" );

  auto fd = new QFileDialog ( this, tr("Select new DB File"),
                              path,
                              tr("XML data files (*.data.xml)"));
  fd->setFileMode ( QFileDialog::AnyFile );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptSave );
  fd->setSidebarUrls(FileInfo::get_path_urls());
  if (!(fd->exec() == QDialog::Accepted)) {
    return;
  }
  FileInfo::set_data_path(fd->directory().path());
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


bool dbe::CreateDatabaseWidget::create_database_file(std::string extra_text) {
  Configuration db ( "oksconflibs" );
  try {
    const std::string DatabaseName = DatabaseFile.absoluteFilePath().toStdString();
    db.create (DatabaseName, get_includes());
    db.commit();
  }
  catch ( dunedaq::conffwk::Exception const & ex )
  {
    FAIL ( "Database creation failure", dbe::config::errors::parse ( ex ).c_str() );
    return false;
  }

  QMessageBox::information (0, tr ( "DBE" ),
                            QString::fromStdString ("The Database was created.\n"+extra_text),
                            QMessageBox::Ok );
  return true;
}
void dbe::CreateDatabaseWidget::CreateDatabaseFileLoad()
{
  if (create_database_file("Now the DB will be loaded into the Editor!\n")) {
    emit CanLoadDatabase ( DatabaseFile.absoluteFilePath() );
    close();
  }
}

void dbe::CreateDatabaseWidget::CreateDatabaseFileNoLoad()
{
  if (create_database_file("")) {
    close();
  }
}

void dbe::CreateDatabaseWidget::CreateDatabaseFileInclude()
{
  if (create_database_file("")) {
    emit CanIncludeDatabase ( DatabaseFile.absoluteFilePath() );
    close();
  }
}
