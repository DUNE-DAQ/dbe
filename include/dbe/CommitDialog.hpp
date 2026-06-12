/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/CommitDialog.h to include/dbe/CommitDialog.hpp).
 */

#ifndef COMMITDIALOG_H
#define COMMITDIALOG_H

/// Include QT Headers
#include <QDialog>
/// Including Ui Files
#include "ui_CommitDialog.h"

#include <memory>

namespace dbe
{

namespace Ui
{
class CommitDialog;
}  // namespace Ui

class CommitDialog: public QDialog, private Ui::CommitDialog
{
  Q_OBJECT
public:
  ~CommitDialog();
  CommitDialog ( QWidget * parent = 0 );

  void SetController();
  QString GetCommitMessage() const;

private:
  std::unique_ptr<dbe::Ui::CommitDialog> ui;

private slots:
  void CommitMessageEdited ();
  void OkCommitMessage();
  void CancelCommitMessage();
};

}  // namespace dbe
#endif // COMMITDIALOG_H
