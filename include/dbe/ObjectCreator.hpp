/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/ObjectCreator.h to include/dbe/ObjectCreator.hpp).
 */

#ifndef OBJECTCREATOR_H
#define OBJECTCREATOR_H

#include "dbe/FileModel.hpp"
#include "dbe/ObjectEditor.hpp"

#include "conffwk/Schema.hpp"
#include "conffwk/ConfigObject.hpp"

#include <QWidget>
#include <QStatusBar>
#include <QUuid>

#include <memory>

namespace dbe
{

namespace Ui
{
class ObjectCreator;
}  // namespace Ui

class ObjectCreator: public QWidget
{

  Q_OBJECT

public:
  ~ObjectCreator();
  /**
   * Create a new object of class
   *
   * @param classinfo describes the class for which an object is to be created
   * @param parent of this widget
   */
  ObjectCreator ( dunedaq::conffwk::class_t const & classinfo, QWidget * parent = 0 );

  /**
   * Clone an object of a given relation type
   *
   * @param anobject is a reference to another object
   * @param relation is a relation to set
   * @param parent of this widget
   */
  ObjectCreator ( tref const & clonefrom,
                  dunedaq::conffwk::relationship_t const & relation, QWidget * parent = 0 );

  /**
   * Clone from a given object
   *
   * @param copyfrom is the object to copy from
   * @param parent
   */
  ObjectCreator ( tref const & clonefrom, QWidget * parent = 0 );

  bool CanClose();

private slots:
  void AddInclude();
  void UpdateActions();
  void UpdateActions ( QString );
  /**
   * Called from ui when the create button has been pressed
   */
  void CreateObject(bool openEditor = false);
  void CreateOpenObject();
  void SetObjectChanged();
  void ActiveFileChanged ( QString const & );
  void MustPressReturn ( QString const & );
  void SetUID();

signals:
  void stateChanged();

private:
  void setup_editor();
  void setup_copy_editor();

  void SetComboClass();
  void SetController();
  void BuildFileModel();
  void SetStatusBar();
  QString GetMessage();

  bool GetState ( int Flags );
  void BuildContextMenu();
  void FillUidComboBox();
  void closeEvent ( QCloseEvent * event );

  std::unique_ptr<dbe::Ui::ObjectCreator> ui;

  dunedaq::conffwk::class_t this_object_class;

  std::unique_ptr<tref> this_src_object;
  std::unique_ptr<tref> this_target_object;
  dunedaq::conffwk::relationship_t this_relation;

  QSortFilterProxyModel this_sort;
  FileModel * this_files;

  QStatusBar * this_status_bar;

  int this_state;

  bool UidSet;
  QString this_newuid;
  QString this_file_for_new_object;
  ObjectEditor * this_associated_editor;
  QMenu * ContextMenu;

  bool this_is_temporary;
  bool this_object_changed;
  bool this_create_copy;

  QUuid const uuid;
};
}  //end namespace dbe

#endif // OBJECTCREATOR_H
