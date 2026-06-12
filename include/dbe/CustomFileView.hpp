/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/CustomFileView.h to include/dbe/CustomFileView.hpp).
 */

#ifndef CUSTOMFILEVIEW_H
#define CUSTOMFILEVIEW_H

/// Including QT Headers
#include <QTableView>
#include <QPushButton>
#include <QCheckBox>

namespace dbe
{
class FindDialog;

class CustomFileView: public QTableView
{
  Q_OBJECT
public:
  CustomFileView ( QWidget * parent = nullptr );
  void CreateActions();
  void ConnectActions();
  void CreateContextMenu();
  /// Reimplemented functions
  void contextMenuEvent ( QContextMenuEvent * Event );
private:
  /// Context menu with the possible actions
  QMenu * ContextMenu;
  QAction * LaunchIncludeEditor;
  QAction * HideReadOnlyFiles;
  QAction * FindFile;
  QAction* m_file_info_action;
  /// File Dialog
  QDialog * FindFileDialog;
  QLineEdit * LineEdit;
  QPushButton * NextButton;
  QPushButton * GoButton;
  QCheckBox * WholeWordCheckBox;
  QCheckBox * CaseSensitiveCheckBox;
  /// Match Variables
  int ListIndex;
  QModelIndexList ListOfMatch;
private slots:
  void GoToFile();
  void GoToNext();
  void FindFileSlot();
  void LaunchIncludeEditorSlot();
  void HideReadOnlyFilesSlot ( bool Hide );
  void EditedSearchString ( QString Text );
  void EditedSearchString();
  void ChangeSelection ( QModelIndex Index );
  void file_info_slot ();
  void file_info_slot (QModelIndex index);
signals:
  void stateChanged ( const QString & FileName );
};
} // end namespace dbe
#endif // CUSTOMFILEVIEW_H
