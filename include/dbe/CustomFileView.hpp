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
signals:
  void stateChanged ( const QString & FileName );
};
} // end namespace dbe
#endif // CUSTOMFILEVIEW_H
