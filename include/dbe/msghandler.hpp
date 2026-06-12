/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/msghandler.h to include/dbe/msghandler.hpp).
 */

/************************************************************
 * msghandler.h
 *
 *  Created on: Dec 3, 2014
 *      Author: Leonidas Georgopoulos
 *
 *  Copyright (C) 2014 Leonidas Georgopoulos
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or
 *  copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 ************************************************************/

#ifndef LUTILS_MSGHANDLER_H_
#define LUTILS_MSGHANDLER_H_

#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <iostream>
#include <vector>

#include "dbe/macro.hpp"

namespace lutils
{
namespace program
{

/**
 * Class providing message handling capabilities to multiple sources
 */
template<typename UI, typename M = std::mutex, typename T = std::thread, typename B = bool>
class msghandler
{
public:
  typedef typename UI::t_str t_str;
  typedef std::vector<t_str> t_levels;

  /**
   * Creates or retrieves the msghandler
   *
   * @return a reference to a singleton msghandler
   */
  static msghandler & ref();

  /**
   * Sets the logfile.
   * All remaining messages will be appended to that file
   *
   * @param logfile
   * @return a reference to a singleton msghandler
   */
  void set ( t_str const & logfile );

  /**
   * Sets the minimum level of messages that the message handler will
   * consider for handling. The rest will be discarded
   *
   * @param level is a string that defines the minimum level
   * @param levels is a vector of strings in order of imporance
   */
  void setlevel ( t_str const & level, t_levels levels );

  /**
   * Add a message to the queue to be served
   *
   * @param a string as a message
   */
  void message ( t_str const & messagein );

  /**
   * Add a message to the queue to be served
   *
   * @param messagein is the message content to be displayed
   * @param msglevel is a tag representing the level of the message
   */
  void message ( t_str const & msglevel, t_str const & messagein );

private:
  typedef B t_bool;

  typedef T t_thread;
  typedef M t_mut;
  typedef std::lock_guard<t_mut> t_lock;
  typedef std::shared_ptr<t_str> t_str_ptr;

  typedef std::ofstream t_file;
  typedef std::unique_ptr<t_file> t_file_ptr;

  template<typename E> using t_container = std::queue<E>;
  typedef t_container<std::pair<t_str, t_str>> t_messages;

  t_messages m_messages;
  t_levels m_levels;
  t_str m_level;

  std::condition_variable m_condition;
  std::mutex m_cond_lock;


  t_str_ptr m_logfile;
  t_file_ptr m_file;

  t_thread * m_handler;

  t_bool m_running;
  t_bool m_have_messages;

  t_mut m_loock;

  /**
   * Sets the object in ready to process state
   */
  void on();

  /**
   * Handles incoming messages
   */
  void handle();

  /**
   * Sends message to standard output or the provided UI::post method
   * @param m is the message to send
   */
  template<typename TR = UI> typename TR::default_post_ret_type post ( t_str const & m,
                                                                       t_str const & l );
  template<typename TR = UI> typename TR::post_ret_type post ( t_str const & m,
                                                               t_str const & l );

  /**
   * Send message to file
   * @param the filename to write to
   */
  void file ( t_str const & );

  /**
   * Checks the level of a message and return true if the level of the
   * message is appropriate to be displayed
   *
   * @param level of a message
   * @return true or false
   */
  bool levelcheck ( t_str const & level ) const;

  /**
   * Opens a designated file
   * @param the filename to open
   */
  void reopen ( t_str const & );

  ~msghandler();
  msghandler();
  msghandler ( msghandler const & ) = delete;
  msghandler & operator= ( msghandler const & ) = delete;
};

}
/* namespace program */
} /* namespace lutils */

#include "detail/msghandler.hxx"

#endif /* LUTILS_MSGHANDLER_H_ */
