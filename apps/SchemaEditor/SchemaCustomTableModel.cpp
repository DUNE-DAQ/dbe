#include "dbe/SchemaCustomTableModel.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include <QIODevice>
#include <QDataStream>

using namespace dunedaq::oks;

dbse::CustomTableModel::CustomTableModel ( QStringList Headers, QObject * parent )
  : QAbstractTableModel ( parent ),
    HeaderList ( Headers )
{
  setupModel();
}

dbse::CustomTableModel::~CustomTableModel()
{
}

int dbse::CustomTableModel::rowCount ( const QModelIndex & parent ) const
{
  Q_UNUSED ( parent )
  return m_data.size();
}

int dbse::CustomTableModel::columnCount ( const QModelIndex & parent ) const
{
  Q_UNUSED ( parent )
  return HeaderList.size();
}

Qt::ItemFlags dbse::CustomTableModel::flags ( const QModelIndex & index ) const
{
  Q_UNUSED ( index )
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QVariant dbse::CustomTableModel::headerData ( int section, Qt::Orientation orientation,
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

QVariant dbse::CustomTableModel::data ( const QModelIndex & index, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    return m_data.value ( index.row() ).value ( index.column() );
  }
  if ( role == Qt::ToolTipRole )
  {
    return m_tooltips.value ( index.row() ).value ( index.column() );
  }


  return QVariant();
}

QStringList dbse::CustomTableModel::getRowFromIndex ( QModelIndex & index )
{
  if ( !index.isValid() )
  {
    return QStringList();
  }

  return m_data.at ( index.row() );
}

void dbse::CustomTableModel::setupModel()
{
  std::vector<OksClass *> ClassList;
  KernelWrapper::GetInstance().GetClassList ( ClassList );

  for ( unsigned int i = 0; i < ClassList.size(); ++i )
  {
    QList<QString> Row;
    OksClass * Class = ClassList.at ( i );
    Row.append ( QString ( Class->get_name().c_str() ) );
    m_data.append ( Row );
    m_tooltips.append (QStringList {QString ( Class->get_description().c_str() )});
  }
}

QStringList dbse::CustomTableModel::mimeTypes() const
{
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

QMimeData * dbse::CustomTableModel::mimeData ( const QModelIndexList & indexes ) const
{
  QMimeData * mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream ( &encodedData, QIODevice::WriteOnly );

  foreach ( QModelIndex index, indexes )
  {
    if ( index.isValid() && index.column() == 0 )
    {
      QString ClassName = data ( index, Qt::DisplayRole ).toString();
      stream << ClassName;
    }
  }

  mimeData->setData ( "application/vnd.text.list", encodedData );
  return mimeData;
}
