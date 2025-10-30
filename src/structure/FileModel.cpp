/// Including QT Headers
#include "dbe/confaccessor.hpp"
#include "dbe/ui_constants.hpp"
#include "dbe/StyleUtility.hpp"

#include <QFileInfo>
#include <QDir>
/// Including DBE Headers
#include "dbe/FileModel.hpp"
#include "dbe/config_api_get.hpp"
#include "dbe/MainWindow.hpp"

dbe::FileModel::FileModel ( QObject * parent )
  : QAbstractTableModel ( parent ),
    Headers
{ tr ( "Name" ), tr ( "Folder" ), tr ( "Access" ), tr ( "Status" ) }
{
  initpaths();
  initconnections();
  initmodel();
}

dbe::FileModel::FileModel ( QList<QStringList> const & FileList, QObject * parent )
  : QAbstractTableModel ( parent ),
    IncludedFiles ( FileList ),
    Headers
{ tr ( "Name" ), tr ( "Folder" ), tr ( "Access" ), tr ( "Status" ) }
{
  initconnections();
}

dbe::FileModel::~FileModel()
{}

int dbe::FileModel::rowCount ( const QModelIndex & parent ) const
{
  if ( !parent.isValid() )
  {
    return IncludedFiles.size();
  }

  return 0;
}

int dbe::FileModel::columnCount ( const QModelIndex & parent ) const
{
  Q_UNUSED ( parent )
  return Headers.size();
}

QVariant dbe::FileModel::data ( const QModelIndex & index, int role ) const
{
  if ( index.isValid() )
  {
    if ( role == Qt::DisplayRole ) {
      return QVariant (
               IncludedFiles.at ( index.row() ).at ( index.column() ) );
    }
    if ( role == Qt::ForegroundRole )
    {
      if ( IncludedFiles.at ( index.row() ).at (
             static_cast<int> ( tablepositions::filepermission ) ) == "RO" ) {
        return QBrush (StyleUtility::FileReadOnlyForeground );
      }
    }
    if ( role == Qt::BackgroundRole )
    {
      if ( IncludedFiles.at ( index.row() ).at (
             static_cast<int> ( tablepositions::filepermission ) ) == "RO" ) {
        return QBrush (StyleUtility::FileReadOnlyBackground );
      }
    }
  }

  return QVariant();
}

QVariant dbe::FileModel::headerData ( int section, Qt::Orientation orientation,
                                      int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Horizontal )
    {
      return Headers.at ( section );
    }

    if ( orientation == Qt::Vertical )
    {
      return section + 1;
    }
  }

  return QVariant();
}

Qt::ItemFlags dbe::FileModel::flags ( const QModelIndex & /*index*/ ) const
{
  // if ( IncludedFiles.at ( index.row() ).at ( static_cast<int>
  //                                            ( tablepositions::filepermission ) ) == "RW" )
  // {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  // }

  // return Qt::ItemIsSelectable;
}

void dbe::FileModel::initpaths()
{
  QString TDAQ_DB_REPOSITORY = getenv ( "TDAQ_DB_REPOSITORY" );
  if(TDAQ_DB_REPOSITORY.isEmpty() == false) {
      QString TDAQ_DB_USER_REPOSITORY = getenv ( "TDAQ_DB_USER_REPOSITORY" );
      FolderPathList = TDAQ_DB_USER_REPOSITORY.split ( ":", Qt::SkipEmptyParts );

      FolderPathList << dbe::MainWindow::findthis()->find_db_repository_dir();
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
}

void dbe::FileModel::initconnections()
{
  connect ( this, SIGNAL ( FileCacheReady ( QList<QStringList> & ) ), &confaccessor::ref(),
            SLOT ( GetFileCache ( QList<QStringList> & ) ) );
}

QString dbe::FileModel::GetFullFileName ( QString & FileName )
{
  QFileInfo FileInfo ( FileName );

  if ( FileInfo.isRelative() )
  {
    bool found=false;
    for ( const QString & Folder : FolderPathList )
    {
      QDir TdaqFolder ( Folder );

      if ( TdaqFolder.exists ( FileName ) )
      {
        FileName = TdaqFolder.path() + "/" + FileName;
        found=true;
        break;
      }
    }
    if (!found) {
      FileName = FileInfo.absoluteFilePath();
    }
  }

  return FileName;
}

QList<QStringList> dbe::FileModel::GetFilesInfo() const
{
  return IncludedFiles;
}

void dbe::FileModel::initmodel()
{
  QStringList sources = dbe::config::api::get::file::inclusions (
  { confaccessor::dbfullname() } );

  std::list<std::string> updated = confaccessor::uncommitted_files();
  for ( QString const & source : sources )
  {
    QFileInfo srcinfo ( source );
    QString fn = srcinfo.filePath();
    QString dn = GetFullFileName(fn).remove(QRegularExpression("/[a-zA-Z0-9_-]*.data.xml"));

    QString modified{};
    auto file = GetFullFileName(fn).toStdString();
    for (auto ufile: updated) {
      if (file == ufile) {
        modified = "Modified";
        break;
      }
    }

    IncludedFiles.append ( QStringList{ srcinfo.fileName(),
        dn,
        confaccessor::check_file_rw ( source ) ? "RW" : "RO",
        modified}
      );
  }

  if ( IncludedFiles.size() )
  {
    emit FileCacheReady ( IncludedFiles );
  }
}
