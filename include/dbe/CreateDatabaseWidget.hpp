#ifndef CREATEDATABASEWIDGET_H
#define CREATEDATABASEWIDGET_H

/// Including QT Headers
#include <QWidget>
#include <QStatusBar>
#include <QFileInfo>
/// Including Ui Files
#include "ui_CreateDatabaseWidget.h"

namespace dbe
{

namespace Ui
{
class CreateDatabaseWidget;
}  // namespace Ui

class CreateDatabaseWidget: public QWidget, private dbe::Ui::CreateDatabaseWidget
{
  Q_OBJECT
public:
  CreateDatabaseWidget ( QWidget * parent = nullptr, bool Include = false,
                         const QString & CreateDir = QString() );

signals:
  void CanLoadDatabase ( const QString & DatabasePath );
  void CanIncludeDatabase ( const QString & DatabasePath );

private:
  QStatusBar * StatusBar;
  QFileInfo DatabaseFile;
  QFileInfo SchemaFile;
  QString DirToCreate;
  bool CreateToInclude;
  std::list<std::string> get_includes();
private slots:
  void DefineSchema();
  void DefineDatabaseFile();
  void CreateDatabaseFileLoad();
  void CreateDatabaseFileNoLoad();
  void CreateDatabaseFileInclude();
};

} // end namespace dbe
#endif // CREATEDATABASEWIDGET_H
