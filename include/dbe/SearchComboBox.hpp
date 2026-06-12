/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/SearchComboBox.h to include/dbe/SearchComboBox.hpp).
 */

#ifndef SEARCHCOMBOBOX_H
#define SEARCHCOMBOBOX_H

/// Including Qt Headers
#include <QComboBox>

namespace dbe
{

class SearchComboBox: public QComboBox
{
  Q_OBJECT
public:
  explicit SearchComboBox ( QWidget * parent = 0 );
protected:
  void focusInEvent ( QFocusEvent * Event );
  void keyPressEvent ( QKeyEvent * Event );
private:
  QString UserText;
private slots:
  void ChangeToolTip ( const QString & Text );
signals:
  void ReturnPressed();
  void TextModified ( const QString & );
};

}  // namespace dbe

#endif // SEARCHCOMBOBOX_H
