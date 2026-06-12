/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/*
 * messenger.cpp
 *
 *  Created on: Nov 3, 2015
 *      Author: Leonidas Georgopoulos
 */

#include "logging/Logging.hpp"

#include "dbe/messenger.hpp"
#include "dbe/messenger_proxy.hpp"

#include <map>
#include <cassert>

namespace dbe
{
namespace interface
{
namespace messenger
{

//------------------------------------------------------------------------------------------
msglevels::t_str const msglevels::debug = "DEBUG";
msglevels::t_str const msglevels::info = "INFO";
msglevels::t_str const msglevels::note = "NOTE";
msglevels::t_str const msglevels::warn = "WARN";
msglevels::t_str const msglevels::error = "ERROR";
msglevels::t_str const msglevels::fail = "FAIL";

msglevels::t_levels const msglevels::all_levels = {debug, info, note, warn, error, fail};
// provides codes from message level type ( level -> code )
static std::map<qt::t_str const, messages const> codes =
{
  { msglevels::info, messages::DEBUG},
  { msglevels::info, messages::INFO },
  { msglevels::note, messages::NOTE },
  { msglevels::warn, messages::WARN },
  { msglevels::error, messages::ERROR },
  { msglevels::fail, messages::FAIL }
};

// provides level types from code ( code->level)
static std::map<messages const, qt::t_str const> levels =
{
  { messages::DEBUG, msglevels::debug },
  { messages::INFO, msglevels::info },
  { messages::NOTE, msglevels::note },
  { messages::WARN, msglevels::warn },
  { messages::ERROR, msglevels::error },
  { messages::FAIL, msglevels::fail }
};

// provides titles from level types
static std::map<qt::t_str const, qt::t_str const> titles =
{
  { msglevels::debug, "Debug info" },
  { msglevels::info, "Information" },
  { msglevels::note, "Notice" },
  { msglevels::warn, "Warning" },
  { msglevels::error, "Error" },
  { msglevels::fail, "Failure" }
};

std::mutex qt::m_block;
std::atomic<bool> qt::m_batch_mode ( false );
qt::t_batches qt::m_batches;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
qt::post_ret_type qt::post ( t_str const & m, t_str const & l )
{
  std::lock_guard<std::mutex> lock ( m_block );

  if ( m_batch_mode )
  {
    merge_post ( m, l );
  }
  else
  {
    direct_post ( m, l );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
qt::post_ret_type qt::direct_post ( t_str const & m, t_str const & l )
{
  if ( warn == l )
  {
    TLOG_DEBUG(0) <<  static_cast<int> ( messages::WARN ) <<  m ;
    messenger_proxy::ref().warn ( titles[warn], m );
  }
  else if ( error == l )
  {
    TLOG_DEBUG(0) << static_cast<int> ( messages::ERROR ) << m ;
    messenger_proxy::ref().error ( titles[error], m );
  }
  else if ( info == l )
  {
    TLOG_DEBUG(0) << static_cast<int> ( messages::INFO ) << m ;
    messenger_proxy::ref().info ( titles[info], m );
  }
  else if ( fail == l )
  {
    TLOG_DEBUG(0) << static_cast<int> ( messages::FAIL ) << m ;
    messenger_proxy::ref().fail ( titles[fail], m );
  }
  else if ( note == l )
  {
    TLOG_DEBUG(0) << static_cast<int> ( messages::NOTE ) << m ;
    messenger_proxy::ref().fail ( titles[note], m );
  }
  else if ( debug == l )
  {
    TLOG_DEBUG(0) << static_cast<int> ( messages::DEBUG ) << m ;
    messenger_proxy::ref().fail ( titles[debug], m );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
qt::post_ret_type qt::merge_post ( t_str const & m, t_str const & l )
{
  auto codeit = codes.find ( l );
  assert ( codeit != codes.end() );
  m_batches[static_cast<int> ( codeit->second )].insert ( m );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void qt::purge()
{
  for ( auto clevel = m_batches.begin(); clevel != m_batches.end(); ++clevel )
  {
    if ( not clevel->empty() )
    {
        t_str merged ( "Summary of messages during batch mode" );

        for(auto it = clevel->begin(); it != clevel->end(); ) {
            auto cnt = clevel->count(*it);

            t_str cmessage = *it;
            merged += t_str ( "\n\n" ) + cmessage + t_str ( "\n\n #Occurances=" ) + std::to_string(cnt);

            std::advance(it, cnt);
        }

        direct_post ( merged, levels[static_cast<messages> ( clevel - m_batches.begin() )] );
        clevel->clear();
    }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
std::unique_ptr<batch_guard> qt::batchmode()
{
  return std::unique_ptr<batch_guard> ( new batch_guard() );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
batch_guard::~batch_guard()
{
  qt::m_batch_mode = false;
  qt::purge();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
batch_guard::batch_guard()
{
  qt::m_batch_mode = true;
}
//------------------------------------------------------------------------------------------

}// namespace messenger
} /* namespace interface */
} /* namespace dbe */
