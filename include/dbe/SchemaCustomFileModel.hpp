#ifndef CUSTOMFILEMODEL_H
#define CUSTOMFILEMODEL_H

/// Including QT Headers
#include <QAbstractTableModel>
#include <QStringList>

namespace dbse
{

class CustomFileModel: public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit CustomFileModel ( QStringList & Headers, QObject * parent = nullptr );
  ~CustomFileModel();
  int rowCount ( const QModelIndex & parent ) const;
  int columnCount ( const QModelIndex & parent ) const;
  // Qt::ItemFlags flags ( const QModelIndex & index ) const;
  QVariant headerData ( int section, Qt::Orientation orientation, int role ) const;
  QVariant data ( const QModelIndex & index, int role ) const;
  void setupModel();
  QStringList getRowFromIndex ( QModelIndex & index );
private:
  QStringList HeaderList;
  QList<QStringList> Data;
};

}  // namespace dbse
#endif // CUSTOMFILEMODEL_H
