/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from src/SchemaEditor/SchemaCustomModelInterface.cpp to apps/SchemaEditor/SchemaCustomModelInterface.cpp).
 */

/// Including Schema Editor
#include "dbe/SchemaCustomModelInterface.hpp"
#include "dbe/SchemaKernelWrapper.hpp"

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
  if ( role != Qt::DisplayRole )
  {
    return QVariant();
  }

  return Data.value ( index.row() ).value ( index.column() );
}

QStringList dbse::CustomModelInterface::getRowFromIndex ( QModelIndex & index )
{
  if ( !index.isValid() )
  {
    return QStringList();
  }

  return Data.at ( index.row() );
}
