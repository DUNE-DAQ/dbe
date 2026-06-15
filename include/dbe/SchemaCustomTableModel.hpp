// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/SchemaCustomTableModel.h to include/dbe/SchemaCustomTableModel.hpp).

#ifndef CUSTOMTABLEMODEL_H
#define CUSTOMTABLEMODEL_H

/// Including QT Headers
#include <QAbstractTableModel>
#include <QBrush>
#include <QColor>
#include <QStringList>
#include <QMimeData>

#include <vector>

namespace dbse
{

class CustomTableModel: public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit CustomTableModel ( QStringList Headers, QObject * parent = nullptr );
  ~CustomTableModel();
  int rowCount ( const QModelIndex & parent ) const;
  int columnCount ( const QModelIndex & parent ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  QVariant headerData ( int section, Qt::Orientation orientation, int role ) const;
  QVariant data ( const QModelIndex & index, int role ) const;
  QStringList getRowFromIndex ( QModelIndex & index );
  void setupModel();
  /// Drag/Drop Handlers
  QStringList mimeTypes() const;
  QMimeData * mimeData ( const QModelIndexList & indexes ) const;
private:
  QStringList HeaderList;
  QList<QList<QString>> m_data;
  QList<QList<QString>> m_tooltips;
  std::vector<QBrush> m_brushes;
  std::vector<QColor> m_backgrounds;
};

}  // namespace dbse
#endif // CUSTOMTABLEMODEL_H
