#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QDir>
#include <QMainWindow>
#include <QModelIndex>
#include <QMenu>
#include <QSortFilterProxyModel>
#include "dbe/SchemaCustomFileModel.hpp"
#include "dbe/SchemaCustomTableModel.hpp"
#include "dbe/SchemaSettings.hpp"

class QGraphicsView;

namespace dbse
{
  class SchemaTab;
  class SchemaFileInfo;

namespace Ui
{
class SchemaMainWindow;
}  // namespace Ui

class SchemaMainWindow: public QMainWindow
{
  Q_OBJECT
public:
  ~SchemaMainWindow();
  explicit SchemaMainWindow ( QString SchemaFile, QWidget * parent = nullptr );

private:
  std::unique_ptr<dbse::Ui::SchemaMainWindow> ui;

  CustomFileModel * FileModel;
  CustomTableModel * TableModel;
  QSortFilterProxyModel * m_proxyModel;
  SchemaSettings* m_settings{nullptr};
  QMenu * ContextMenuFileView;
  QMenu * ContextMenuTableView;
  QString m_title{"DUNE DAQ Configuration Schema editor"};
  QString m_view_dir{"."};
  QString m_export_path{"."};
  QDir m_schema_directory{"."};
  QList<QUrl> m_path_urls;

  void InitialSettings();
  void InitialTab();
  void InitialTabCorner();
  void SetController();
  void BuildFileModel();
  void BuildTableModel();
  void update_window_title(QString text);
  void write_view_file(const QString& fn, SchemaTab* tab);
  [[nodiscard]] int ShouldSaveChanges() const;
  [[nodiscard]] int ShouldSaveViewChanges() const;
  bool check_schema_file(QString file);
  bool save_schema_file(QString file);
protected:
  void closeEvent ( QCloseEvent * event );
  void OpenSchemaFile( QString SchemaFile);
public slots:
  void modifiedView (bool modified);
  void update_models();
private slots:
  void OpenSchemaFile();
  void CreateNewSchema();
  void show_file_info_active_schema();
  void show_file_info(QModelIndex);
  void show_file_info(QString);
  void show_file_info();
  void connect_file_info(SchemaFileInfo* win);
  void edit_settings();
  // From main menu / shortcut
  void SaveSchema();
  // From FileView 
  void SaveSchemaFile();
  bool SaveModifiedSchema();
  void ChangeCursorRelationship ( bool State );
  void ChangeCursorInheritance ( bool State );
  void add_tab();
  void SaveView();
  void SaveViewAs();
  void LoadView();
  void NameView();
  void LaunchClassEditor ( QModelIndex Index );
  void RemoveTab ( int i );
  void close_tab();
  void CustomContextMenuFileView ( QPoint Pos );
  void CustomContextMenuTableView ( QPoint Pos );
  void AddNewClass();
  void RemoveClass();
  void editClass();
  void SetSchemaFileActive();
  void PrintCurrentView();
  void export_current_view();
  void toggle_case_sensitive ( int );
  void update_view();
};

}  // namespace dbse
#endif // MAINWINDOW_H
