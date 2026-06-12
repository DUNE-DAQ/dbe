/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/ValidatorComboBox.h to include/dbe/ValidatorComboBox.hpp).
 */

#ifndef VALIDATORCOMBOBOX_H
#define VALIDATORCOMBOBOX_H

#include <QComboBox>

namespace dbe
{

class ValidatorComboBox: public QComboBox
{
  Q_OBJECT
public:
  explicit ValidatorComboBox ( QWidget * parent = nullptr );
  ~ValidatorComboBox();
  bool IsValid() const;
  bool IsChanged() const;
  QStringList GetDataList();
private:
  QStringList DataList;
  QString Default;
  bool Valid;
  bool ValueChanged;
  void wheelEvent ( QWheelEvent * e );
public slots:
  void TryValidate ( const QString & ValidateString );
private slots:
  void ChangeDetected ( const QString & StringChange );
  void CheckDefaults ( int Default );
  bool CompareDefaults();
signals:
  void ValueWasChanged();
};

}  // namespace dbe
#endif // VALIDATORCOMBOBOX_H
