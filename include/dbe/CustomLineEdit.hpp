/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/CustomLineEdit.h to include/dbe/CustomLineEdit.hpp).
 */

#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>
#include <QValidator>

namespace dbe
{
class CustomLineEdit: public QLineEdit
{
  Q_OBJECT
public:
  explicit CustomLineEdit ( QWidget * parent = 0, bool IsNull = true, bool IsMulti = false );
  ~CustomLineEdit();
  void SetNullCheck ( bool IsNull );
  void SetMultiCheck ( bool IsMulti );
  bool IsNullCheck();
  bool IsValid();
  void SetCheckDefault ( bool Default );
  void SetDefaultValue ( QString ValueDefault );
  void ValidateText();
  void SetLoadedDefaultFlag ( bool Loaded );
  void SetValidator ( QValidator * ValidatorSet );
  void SetPopupMenu();
  void CreateActions();
signals:
  void StringValidated();
  void ValueChanged();
  void DecChange();
  void HexChange();
  void OctChange();
public slots:
  void TryValidate ( QString Dummy );
  void EmitDecSlot();
  void EmitOctSlot();
  void EmitHexSlot();
protected:
  void contextMenuEvent ( QContextMenuEvent * Event );
private:
  bool Valid;
  bool NullCheck;
  bool CheckDefault;
  bool PopupMenu;
  bool IsMultiValue;
  QMenu * ContextMenu;
  QAction * Dec;
  QAction * Oct;
  QAction * Hex;
  QValidator * Validator;
  QString DefaultValue;
};
} // end namespace dbe

#endif // CUSTOMLINEEDIT_H
