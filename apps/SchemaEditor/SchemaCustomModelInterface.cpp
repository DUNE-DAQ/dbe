/// Including Schema Editor
#include "dbe/SchemaCustomModelInterface.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include "dbe/SchemaStyle.hpp"

dbse::CustomModelInterface::CustomModelInterface ( QStringList Headers, QObject * parent )
  : QAbstractTableModel ( parent ),
    HeaderList ( Headers )
{
}

dbse::CustomModelInterface::~CustomModelInterface()
{
}

int dbse::CustomModelInterface::rowCount ( const QModelIndex & parent ) const
{
  Q_UNUSED ( parent )
  return Data.size();
}

int dbse::CustomModelInterface::columnCount ( const QModelIndex & parent ) const
{
  Q_UNUSED ( parent )
  return HeaderList.size();
}

Qt::ItemFlags dbse::CustomModelInterface::flags ( const QModelIndex & index ) const
{
  Q_UNUSED ( index )
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant dbse::CustomModelInterface::headerData ( int section, Qt::Orientation orientation,
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

QVariant dbse::CustomModelInterface::data ( const QModelIndex & index, int role ) const
{
  if ( role == Qt::DisplayRole ) {
    return Data.value ( index.row() ).value ( index.column() );
  }
  else if (role == Qt::ForegroundRole) {
    auto row = Data.at(index.row());
    auto flag = row.at(row.size()-1);
    if (flag == "I") {
      return SchemaStyle::get_color("foreground", "inherited");
    }
  }
  else if (role == Qt::BackgroundRole) {
    auto row = Data.at(index.row());
    auto flag = row.at(row.size()-1);
    if (flag == "I") {
      return SchemaStyle::get_color("background", "inherited");
    }
  }
  return QVariant();
}

QStringList dbse::CustomModelInterface::getRowFromIndex ( QModelIndex & index )
{
  if ( !index.isValid() )
  {
    return QStringList();
  }

  return Data.at ( index.row() );
}
