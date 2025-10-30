/// Including Schema Editor
#include "dbe/SchemaCustomFileModel.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include "dbe/SchemaStyle.hpp"
/// Including C++ Headers
#include <vector>

#include <QBrush>
#include <QColor>
#include <QSize>

dbse::CustomFileModel::CustomFileModel ( QStringList & Headers, QObject * parent )
  : QAbstractTableModel ( parent ),
    HeaderList ( Headers )
{
  setupModel();
}

dbse::CustomFileModel::~CustomFileModel()
{
}

int dbse::CustomFileModel::rowCount ( const QModelIndex & parent ) const
{
  Q_UNUSED ( parent );
  return Data.size();
}

int dbse::CustomFileModel::columnCount ( const QModelIndex & parent ) const
{
  Q_UNUSED ( parent );
  return HeaderList.size();
}

// Qt::ItemFlags dbse::CustomFileModel::flags ( const QModelIndex & index ) const
// {
//   // if ( Data.value (index.row() ).value ( 1) == "RW" ) {
//     return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
//   // }
//   // else {
//   //   return Qt::NoItemFlags;
//   // }
// }

QVariant dbse::CustomFileModel::headerData ( int section, Qt::Orientation orientation,
                                             int role ) const
{
  if ( role != Qt::DisplayRole )
  {
    return QVariant();
  }

  if ( orientation == Qt::Horizontal )
  {
    return HeaderList.at ( section );
  }

  return QVariant();
}

QVariant dbse::CustomFileModel::data ( const QModelIndex & index, int role ) const
{
  if ( role == Qt::DisplayRole ) {
    return Data.value(index.row()).value(index.column());
  }
  if (role == Qt::ForegroundRole) {
    if (Data.value(index.row()).value(2).contains("Active")) {
      return QBrush(QColor (SchemaStyle::get_color("foreground", "active_file")));
    }
    if (Data.value(index.row()).value(1) == "RW" ) {
      return QBrush(QColor (SchemaStyle::get_color("foreground", "default")));
    }
    return QBrush(SchemaStyle::get_color("foreground", "readonly"));
  }
  if (role == Qt::BackgroundRole) {
    if (Data.value(index.row()).value(2).contains("Active")) {
      return QBrush(QColor (SchemaStyle::get_color("background", "active_file")));
    }
    if (Data.value(index.row()).value(1) == "RW" ) {
      return QBrush(QColor (SchemaStyle::get_color("background", "default")));
    }
    return QBrush(SchemaStyle::get_color("background", "readonly"));
  }
  if (role == Qt::ToolTipRole) {
    return Data.value(index.row()).value(0);
  }

  return QVariant();
}

void dbse::CustomFileModel::setupModel()
{
  std::vector<std::string> SchemaFiles;
  KernelWrapper::GetInstance().GetSchemaFiles ( SchemaFiles );
  std::string ActiveSchema = KernelWrapper::GetInstance().GetActiveSchema();
  std::string Modified = KernelWrapper::GetInstance().ModifiedSchemaFiles();
  for ( std::string & FileName : SchemaFiles )
  {
    QStringList Row;
    Row.append ( QString::fromStdString ( FileName ) );

    if ( KernelWrapper::GetInstance().IsFileWritable ( FileName ) )
    {
      Row.append ( "RW" );
    }
    else
    {
      Row.append ( "RO" );
    }

    std::string Status = "";
    if ( Modified.find( FileName ) != std::string::npos) {
      Status = "Modified";
    }
    if ( FileName == ActiveSchema) {
      if ( !Status.empty() ) {
        Status += "  ";
      }
      Status += "Active";
    }

    Row.append ( QString::fromStdString ( Status ) );
    Data.append ( Row );
  }
}

QStringList dbse::CustomFileModel::getRowFromIndex ( QModelIndex & index )
{
  if ( !index.isValid() )
  {
    return QStringList();
  }

  return Data.at ( index.row() );
}
