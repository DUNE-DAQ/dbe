/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/OracleWidget.h to include/dbe/OracleWidget.hpp).
 */

#ifndef ORACLEWIDGET_H
#define ORACLEWIDGET_H

#include <memory>
#include <QWidget>

namespace dbe
{
namespace Ui
{
class OracleWidget;
}  // namespace Ui

class OracleWidget: public QWidget
{
  Q_OBJECT
public:
  ~OracleWidget();
  explicit OracleWidget ( QWidget * parent = 0 );

private:
  void SetController();

  std::unique_ptr<dbe::Ui::OracleWidget> ui;

private slots:
  void ProcessOracleCommand();

signals:
  void OpenOracleConfig ( const QString & ConfigStream );
};

}  // namespace dbe
#endif // ORACLEWIDGET_H
