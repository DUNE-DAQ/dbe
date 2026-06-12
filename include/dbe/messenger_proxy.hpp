/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/messenger_proxy.h to include/dbe/messenger_proxy.hpp).
 */

/*
 * messenger_proxy.h
 *
 *  Created on: Nov 4, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_MESSENGER_PROXY_H_
#define DBE_MESSENGER_PROXY_H_

#include <QObject>
#include <QString>
#include <QMessageBox>

namespace dbe
{
namespace interface
{

/**
 * messenger_proxy is an class that permit in messenger class to receive qt signals
 */
class messenger_proxy: public QObject
{
  Q_OBJECT
public:
  typedef std::string t_extstr;

  static messenger_proxy & ref();

  void debug ( t_extstr const &, t_extstr const & );
  void info ( t_extstr const &, t_extstr const & );
  void note ( t_extstr const &, t_extstr const & );
  void warn ( t_extstr const &, t_extstr const & );
  void error ( t_extstr const &, t_extstr const & );
  void fail ( t_extstr const &, t_extstr const & );

  /*
   * Signals defined below are used to call across the thread boundary, e.g. from an std::thread to
   * a QThread , in order to guarantee the the strings leave in the receiver thread's memory space
   * copies have to actually be passed as an argument. Otherwise, in case of context switch the
   * value may die, as it is going to be unaware of it being held from the receiver.
   */
signals:
  void signal_debug ( QString , QString );
  void signal_info ( QString , QString );
  void signal_note ( QString , QString );
  void signal_warn ( QString , QString );
  void signal_error ( QString , QString );
  void signal_fail ( QString , QString );

private:
  messenger_proxy();
  messenger_proxy ( messenger_proxy const & ) = delete;
  messenger_proxy operator= ( messenger_proxy const & ) = delete;
};

}
}

#endif /* DBE_MESSENGER_PROXY_H_ */
