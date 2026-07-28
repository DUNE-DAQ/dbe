// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/IncludeFileWidget.h to include/dbe/IncludeFileWidget.hpp).

#ifndef INCLUDEFILEWIDGET_H
#define INCLUDEFILEWIDGET_H

#include <memory>
/// Including QT Headers
#include <QFileDialog>
#include <QWidget>
/// Including DBE
#include "dbe/CreateDatabaseWidget.hpp"

namespace dbe
{
namespace Ui
{
class IncludeFileWidget;
}  // namespace Ui

class IncludeFileWidget: public QWidget
{
  Q_OBJECT
public:
  ~IncludeFileWidget();
  explicit IncludeFileWidget ( QString FilePath, QWidget * parent = 0 );

private:
  void SetRemoveComboBox();
  void SetController();
  void keyPressEvent(QKeyEvent* event) override;

  std::unique_ptr<dbe::Ui::IncludeFileWidget> ui;

  CreateDatabaseWidget * CreateWidget;
  QString DatabasePath;
  QString Directory;
  bool Removed;
  QStatusBar * StatusBar;
  QFileDialog * SelectFile;
  QStringList FolderPathList;
  QStringList dbPath;

private slots:
  void SelectFileToInclude();
  void AddFileToInclude();
  void AddNewFileToInclude ( const QString & File );
  void RemoveFileFromInclude();
  void RemoveFileFromInclude ( int );
  void SetDirectory ( const QString & Dir );
  void CheckInclude ();
  void CreateFileToInclude();
};
}  // namespace dbe

#endif // INCLUDEFILEWIDGET_H
