// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/SchemaCustomModelInterface.h to include/dbe/SchemaCustomModelInterface.hpp).

#ifndef CUSTOMMODELINTERFACE_H
#define CUSTOMMODELINTERFACE_H

/// Including QT Headers
#include <QAbstractTableModel>
#include <QStringList>

namespace dbse
{

class CustomModelInterface: public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit CustomModelInterface ( QStringList Headers, QObject * parent = nullptr );
  ~CustomModelInterface();
  int rowCount ( const QModelIndex & parent ) const;
  int columnCount ( const QModelIndex & parent ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  QVariant headerData ( int section, Qt::Orientation orientation, int role ) const;
  QVariant data ( const QModelIndex & index, int role ) const;
  QStringList getRowFromIndex ( QModelIndex & index );
  virtual void setupModel() = 0;
protected:
  QStringList HeaderList;
  QList<QStringList> Data;
};

}  // namespace dbse
#endif // CUSTOMMODELINTERFACE_H
