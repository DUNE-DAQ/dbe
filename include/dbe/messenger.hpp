/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/messenger.h to include/dbe/messenger.hpp).
 */

/*
 * messenger.h
 *
 *  Created on: Nov 3, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_MESSENGER_H_
#define DBE_MESSENGER_H_

#include "dbe/macro.hpp"
#include "dbe/msghandler.hpp"

#include <string>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <array>
#include <atomic>

#ifndef t_messenger
#define t_messenger dbe::interface::messenger::qt
#endif

#ifndef t_msghandler
#define t_msghandler  ::lutils::program::msghandler<t_messenger>
#endif

#define MESSAGE(...) VA_SELECT(MESSAGE,__VA_ARGS__)

#define MSEP " "

#ifndef MESSAGE_STATICS
#define MESSAGE_STATICS(msg) \
    static t_msghandler::t_str sms(msg); \
    static t_msghandler::t_str smsr("\n\nReason: "); \

#endif

#define MESSAGE_3(level,msg,reason) \
{\
  MESSAGE_STATICS(msg) \
      t_msghandler::ref().message( level, sms + smsr + MSEP +reason); \
}\

#define MESSAGE_4(level,msg,reason,_1) \
{\
  MESSAGE_STATICS(msg) \
  t_msghandler::ref().message( level, sms + MSEP + _1 + smsr + MSEP + reason); \
}\

#define MESSAGE_5(level,msg,reason,_1,_2) \
{\
  MESSAGE_STATICS(msg) \
  t_msghandler::ref().message( level, \
                               sms + MSEP +  _1 + MSEP + _2 + smsr +MSEP+ reason); \
}\

#define MESSAGE_6(level,msg,reason,_1,_2,_3) \
{\
  MESSAGE_STATICS(msg) \
  t_msghandler::ref().message( level, \
                               sms + MSEP + _1 + MSEP + _2 + MSEP + _3 + smsr +MSEP + reason); \
}\

#define MESSAGE_7(level,msg,reason,_1,_2,_3,_4) \
{\
  MESSAGE_STATICS(msg) \
  t_msghandler::ref().message( level,\
                               sms + MSEP+_1 +MSEP+ _2 + MSEP +_3 + MSEP + _4 + smsr + MSEP + reason); \
}\

#define MSGHNDL_DEBUG_LEVEL t_messenger::debug
#define MSGHNDL_INFO_LEVEL t_messenger::info
#define MSGHNDL_NOTE_LEVEL t_messenger::note
#define MSGHNDL_WARN_LEVEL t_messenger::warn
#define MSGHNDL_ERROR_LEVEL t_messenger::error
#define MSGHNDL_FAIL_LEVEL t_messenger::fail

#define WARN(...) VA_SELECT(WARN,__VA_ARGS__)

#define WARN_2(msg,reason) MESSAGE_3(MSGHNDL_WARN_LEVEL,msg,reason)
#define WARN_3(msg,reason,var) MESSAGE_4(MSGHNDL_WARN_LEVEL,msg,reason,var)
#define WARN_4(msg,reason,var,_4) MESSAGE_5(MSGHNDL_WARN_LEVEL,msg,reason,var,_4)
#define WARN_5(msg,reason,var,_4,_5) MESSAGE_6(MSGHNDL_WARN_LEVEL,msg,reason,var,_4,_5)
#define WARN_6(msg,reason,var,_4,_5,_6) MESSAGE_7(MSGHNDL_WARN_LEVEL,msg,reason,var,_4,_5,_6)

#define ERROR(...) VA_SELECT(ERROR,__VA_ARGS__)

#define ERROR_2(msg,reason) MESSAGE_3(MSGHNDL_ERROR_LEVEL,msg,reason)
#define ERROR_3(msg,reason,var) MESSAGE_4(MSGHNDL_ERROR_LEVEL,msg,reason,var)
#define ERROR_4(msg,reason,var,_4) MESSAGE_5(MSGHNDL_ERROR_LEVEL,msg,reason,var,_4)
#define ERROR_5(msg,reason,var,_4,_5) MESSAGE_6(MSGHNDL_ERROR_LEVEL,msg,reason,var,_4,_5)
#define ERROR_6(msg,reason,var,_4,_5,_6) MESSAGE_7(MSGHNDL_ERROR_LEVEL,msg,reason,var,_4,_5,_6)

#define INFO(...) VA_SELECT(INFO,__VA_ARGS__)

#define INFO_2(msg,reason) MESSAGE_3(MSGHNDL_INFO_LEVEL,msg,reason)
#define INFO_3(msg,reason,var) MESSAGE_4(MSGHNDL_INFO_LEVEL,msg,reason,var)
#define INFO_4(msg,reason,var,_4) MESSAGE_5(MSGHNDL_INFO_LEVEL,msg,reason,var,_4)
#define INFO_5(msg,reason,var,_4,_5) MESSAGE_6(MSGHNDL_INFO_LEVEL,msg,reason,var,_4,_5)

#define NOTE(...) VA_SELECT(NOTE,__VA_ARGS__)

#define NOTE_2(msg,reason) MESSAGE_3(MSGHNDL_NOTE_LEVEL,msg,reason)
#define NOTE_3(msg,reason,var) MESSAGE_4(MSGHNDL_NOTE_LEVEL,msg,reason,var)
#define NOTE_4(msg,reason,var,_4) MESSAGE_5(MSGHNDL_NOTE_LEVEL,msg,reason,var,_4)
#define NOTE_5(msg,reason,var,_4,_5) MESSAGE_6(MSGHNDL_NOTE_LEVEL,msg,reason,var,_4,_5)


#ifdef MSGHNDL_FULLDEBUG_INFORMATION
#define FULLDEBUG(...) VA_SELECT(DEBUG,__VA_ARGS__)
#else
#define FULLDEBUG(...)
#endif

#ifdef MSGHNDL_FULLDEBUG_INFORMATION
#define DEBUG(...) VA_SELECT(DEBUG,__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#define DEBUG_2(msg,reason) MESSAGE_3(MSGHNDL_DEBUG_LEVEL,msg,reason)
#define DEBUG_3(msg,reason,var) MESSAGE_4(MSGHNDL_DEBUG_LEVEL,msg,reason,var)
#define DEBUG_4(msg,reason,var,_4) MESSAGE_5(MSGHNDL_DEBUG_LEVEL,msg,reason,var,_4)
#define DEBUG_5(msg,reason,var,_4,_5) MESSAGE_6(MSGHNDL_DEBUG_LEVEL,msg,reason,var,_4,_5)

#define FAIL(...) VA_SELECT(FAIL,__VA_ARGS__)

#define FAIL_2(msg,reason) MESSAGE_3(MSGHNDL_FAIL_LEVEL,msg,reason)
#define FAIL_3(msg,reason,var) MESSAGE_4(MSGHNDL_FAIL_LEVEL,msg,reason,var)
#define FAIL_4(msg,reason,var,_4) MESSAGE_5(MSGHNDL_FAIL_LEVEL,msg,reason,var,_4)
#define FAIL_5(msg,reason,var,_4,_5) MESSAGE_6(MSGHNDL_FAIL_LEVEL,msg,reason,var,_4,_5)

namespace dbe
{
namespace interface
{
namespace messenger
{
enum class messages
{
  DEBUG = 0,
  INFO,
  NOTE,
  WARN,
  ERROR,
  FAIL,
  sizeme
};

class qt;

struct batch_guard
{
  friend class qt;
  ~batch_guard();
private:
  batch_guard();
};

struct msglevels
{
  typedef std::string t_str;
  typedef std::vector<t_str> t_levels;
  static t_str const debug, info, note, warn, error, fail;
  static t_levels const all_levels;
};

/**
 * Messenger handles posting of messages and delegates them to the main window
 * according to their level to be displayed as needed
 */
class qt:
  public msglevels
{
  friend class batch_guard;
public:
  typedef std::string t_str;

  // used to inform at compile time msg handler that this class provides a post function
  HAS_POST

  /**
   * This method provides posting facilities for messages
   *
   * @param the level of the message to be treated
   * @param A string message to be treatead
   * @return post_ret_type is void
   */
  static post_ret_type post ( t_str const &, t_str const & );

  /**
   * Set returns a batch_guard object that sets m_batch_mode, which resets it
   * batch object when it is destroyed.
   *
   * @return a batch_guard object bound to this
   */
  static std::unique_ptr<batch_guard> batchmode();

private:
  qt() = delete;
  qt ( qt const & ) = delete;
  qt operator= ( qt const & ) = delete;

  static post_ret_type direct_post ( t_str const & m, t_str const & l );
  static post_ret_type merge_post ( t_str const & m, t_str const & l );

  /**
   * Takes all messages and forms a message for each groups,  that is then resent for direct_post
   */
  static void purge();

  static std::mutex m_block;
  static std::atomic<bool> m_batch_mode;

  typedef std::unordered_multiset<t_str> t_message_bucket;
  typedef std::array<t_message_bucket, static_cast<int> ( messages::sizeme ) > t_batches;
  static t_batches m_batches;
};

class console:
  public msglevels
{
public:
  DEFAULT_POST
  typedef std::string t_str;
};

} /* namespace messenger */
} /* namespace interface */
} /* namespace dbe */

#endif /* DBE_MESSENGER_H_ */
