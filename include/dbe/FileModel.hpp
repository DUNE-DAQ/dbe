#ifndef FILEMODEL_H
#define FILEMODEL_H

/// Including QT Headers
#include <QAbstractTableModel>
#include <QStringList>

namespace dbe
{

class FileModel: public QAbstractTableModel
{
  Q_OBJECT
public:
  ~FileModel();

  explicit FileModel ( QObject * parent = nullptr );

  explicit FileModel ( QList<QStringList> const & FileList, QObject * parent = nullptr );

  int rowCount ( const QModelIndex & parent ) const;

  int columnCount ( const QModelIndex & parent ) const;

  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

  QVariant headerData ( int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole ) const;

  Qt::ItemFlags flags ( const QModelIndex & index ) const;

  QString GetFullFileName ( QString & FileName );

  QList<QStringList> GetFilesInfo() const;

private:
  void initpaths();
  void initmodel();
  void initconnections();

  QList<QStringList> IncludedFiles;
  QStringList FolderPathList;
  QStringList Headers;

signals:
  void FileCacheReady ( QList<QStringList> & Files );
};

}  // namespace dbe
#endif // FILEMODEL_H
