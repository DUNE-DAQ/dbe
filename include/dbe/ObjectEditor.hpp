#ifndef OBJECTEDITOR_H
#define OBJECTEDITOR_H

#include "dbe/confaccessor.hpp"
#include "dbe/BuildingBlockEditors.hpp"
#include "dbe/CustomFileView.hpp"
#include "dbe/FileModel.hpp"

#include <QWidget>
#include <QStatusBar>
#include <QSplitter>
#include <QFormLayout>
#include <QDialog>
#include <QTableWidget>
#include <QUuid>

#include <memory>
#include <map>

namespace dbe
{
namespace Ui
{
class ObjectEditor;
}  // namespace Ui

class ObjectEditor:
  public QWidget
{
  Q_OBJECT

public:
  explicit ObjectEditor(QWidget * parent = nullptr);
  ObjectEditor ( std::string const & classname, QWidget * parent = nullptr );

  ObjectEditor ( tref const & objref, QWidget * parent = nullptr, bool const iscopy = false );

  ~ObjectEditor();

  void HideDetailWidget ( bool Hide );

  bool CanCloseWindow();

  bool IsEditorValid() const;

  bool WasObjectChanged() const;

  bool ParseToCreate ( std::string const & name, std::string const & filename );

  void SetUsedForCopy ( bool Used );

private:
  void keyPressEvent(QKeyEvent* event) override;
  void init();
  void SetStatusBar();
  void SetController();
  void BuildWidgets();

  void set_tooltip ( dunedaq::conffwk::attribute_t const &, widgets::editors::base * );
  void set_tooltip ( dunedaq::conffwk::relationship_t const &, widgets::editors::base * );

  void set_attribute_widget ( dunedaq::conffwk::attribute_t const &, widgets::editors::base * );

  void register_attribute_widget ( QString const & name, widgets::editors::base * widget );
  void register_relation_widget ( QString const & name, widgets::editors::base * widget );

  void BuildFileInfo();

  // void closeEvent ( QCloseEvent * Event );

  std::unique_ptr<dbe::Ui::ObjectEditor> ui;

  std::string classname;
  std::unique_ptr<dref> m_object_to_edit;

  tref Object()
  {
    return m_object_to_edit->ref();
  }

  QStatusBar * StatusBar;

  std::map<QString, widgets::editors::base *> this_widgets;

  bool IsValid, this_is_in_copy_mode, this_editor_is_owned, this_is_in_creation_mode,
       this_editor_values_changed;

  int CurrentRow;

  QString FilePermission;

  QHBoxLayout * MainLayout;
  QTableWidget * WidgetTable;

  /// Rename widget
  QDialog * RenameWidget;
  QLineEdit * LineEdit;
  QPushButton * GoButton;

  /// Move widget
  QDialog * MoveWidget;
  CustomFileView * FileView;
  FileModel * IncludedFileModel;
  QPushButton * MoveGoButton;

  QString ActivateFile;
  QStringList HorizontalHeaders;

public slots:
  void ResetObjectChanged();

private slots:
  void save_and_close();
  void UpdateActions();
  void ObjectChanged();
  void ParseToSave();
  void LaunchRenameObject();
  void LaunchMoveObject();
  void RenameObject();
  void slot_external_rename_object ( QString const & src, dref const & obj );
  void MoveObject();
  void ActiveFileChanged ( const QString & File );
  void UpdateObjectEditor ( QString const &, dref );
  void UpdateObjectEditor (const QList<QPair<QString, QString>>&);
  void UpdateObjectEditor ();
  void ShouldCloseThisWindow ( QString src, dref key );

signals:
  void LoadedInitials();
  void WidgetUpdated();

private:
  QUuid const uuid;

};
}  // namespace dbe
#endif // OBJECTEDITOR_H
