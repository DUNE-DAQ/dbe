/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/*
 * messenger_proxy.cpp
 *
 *  Created on: Nov 4, 2015
 *      Author: lgeorgop
 */

#include "dbe/messenger_proxy.hpp"

namespace dbe
{
namespace interface
{

messenger_proxy::messenger_proxy() = default;

messenger_proxy & messenger_proxy::ref()
{
  static messenger_proxy inner_messenger;
  return inner_messenger;
}

void messenger_proxy::debug ( t_extstr const & title, t_extstr const & msg )
{
  emit signal_debug ( QObject::tr ( title.c_str() ), QObject::tr ( msg.c_str() ) );
}

void messenger_proxy::info ( t_extstr const & title, t_extstr const & msg )
{
  emit signal_info ( QObject::tr ( title.c_str() ), QObject::tr ( msg.c_str() ) );
}

void messenger_proxy::note ( t_extstr const & title, t_extstr const & msg )
{
  emit signal_note ( QObject::tr ( title.c_str() ), QObject::tr ( msg.c_str() ) );
}

void messenger_proxy::warn ( t_extstr const & title, t_extstr const & msg )
{
  emit signal_warn ( QObject::tr ( title.c_str() ), QObject::tr ( msg.c_str() ) );
}

void messenger_proxy::error ( t_extstr const & title, t_extstr const & msg )
{
  emit signal_error ( QObject::tr ( title.c_str() ), QObject::tr ( msg.c_str() ) );
}

void messenger_proxy::fail ( t_extstr const & title, t_extstr const & msg )
{
  emit signal_fail ( QObject::tr ( title.c_str() ), QObject::tr ( msg.c_str() ) );
}

} /* end namespace interface */
} /* end namespace dbe */

