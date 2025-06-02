#ifndef SCHEMAINCLUDEFILEWIDGET_H
#define SCHEMAINCLUDEFILEWIDGET_H

#include <memory>
/// Including QT Headers
#include <QFileDialog>
#include <QStatusBar>
#include <QWidget>
/// Including DBE
//#include "dbe/CreateDatabaseWidget.hpp"
#include "dbe/StyleUtility.hpp"

using dbe::StyleUtility;


namespace dbse
{
  namespace Ui
  {
    class SchemaIncludeFileWidget;
  }  // namespace Ui

class SchemaIncludeFileWidget: public QWidget
{
  Q_OBJECT
public:
  ~SchemaIncludeFileWidget();
  explicit SchemaIncludeFileWidget ( QString FilePath, QWidget * parent = 0 );

signals:
  void files_updated();
  
private:
  void SetCurrentIncludeList();
  void SetController();
  std::unique_ptr<dbse::Ui::SchemaIncludeFileWidget> ui;

  // CreateDatabaseWidget * CreateWidget;
  QString CurrentFile;
  QString Directory;
  bool Removed;
  QStatusBar * StatusBar;
  QFileDialog * SelectFile;
  QStringList FolderPathList;
  QStringList dbPath;
  QStringList RemovedFileList;

private slots:
  void SelectFileToInclude();
  void AddFileToInclude();
  void AddNewFileToInclude ( const QString & File );
  void RemoveFileFromInclude();
  void RemoveFileFromInclude ( int );
  void SetDirectory ( const QString & Dir );
  void CheckInclude ();
  void SaveSchema();
  void CreateFileToInclude();
};
}  // namespace dbse

#endif // SCHEMAINCLUDEFILEWIDGET_H
