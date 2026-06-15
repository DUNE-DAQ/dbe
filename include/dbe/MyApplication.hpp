// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/MyApplication.h to include/dbe/MyApplication.hpp).

#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>
#include <QMessageBox>

extern char const * const dbe_lib_core_version;

namespace dbe
{
class MyApplication: public QApplication
{
  Q_OBJECT
public:
  MyApplication ( int & argc, char ** argv )
    : QApplication ( argc, argv )
  {
  }
  virtual ~MyApplication()
  {
  }
  virtual bool notify ( QObject * rec, QEvent * ev );
};
}  // namespace dbe

#endif // MYAPPLICATION_H
