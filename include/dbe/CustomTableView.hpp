// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/CustomTableView.h to include/dbe/CustomTableView.hpp).

#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include "dbe/table.hpp"
#include "dbe/view_common_interface.hpp"
#include "conffwk/ConfigObject.hpp"

#include <QTableView>
#include <QPushButton>
#include <QUuid>
#include <QPainter>
#include <QStyleOptionViewItem>

namespace dbe
{

class CustomTableView: public QTableView, public view_common_interface
{
  Q_OBJECT

signals:
  void OpenEditor ( tref Object );

public:
  explicit CustomTableView ( QWidget * parent = 0 );
  void contextMenuEvent ( QContextMenuEvent * Event );

private slots:
  void slot_delete_objects();
  void slot_create_object();
  void slot_edit_object();
  void slot_copy_object();

  void FindObjectSlot();

  void slot_go_to_object();
  void GoToNext();

  void EditedSearchString ( QString Text );

  void referencedBy_OnlyComposite();
  void referencedBy_All();

private:
  void CreateActions();

  void referencedBy ( tref obj, bool onlyComposite );

  QMenu * m_context_menu;
  QAction * FindObject;
  QAction * editObject;
  QAction * deleteObjectAc;
  QAction * refByAc;
  QAction * refByAcOnlyComp;
  QAction * copyObjectAc;

  /// File Dialog
  QDialog * m_find_object_dialog;
  QLineEdit * LineEdit;
  QPushButton * NextButton;
  QPushButton * GoButton;

  /// Match Variables
  int ListIndex;
  QModelIndexList ListOfMatch;

  int m_last_object_item;
};
} // end namespace dbe
#endif // CUSTOMTABLEVIEW_H
