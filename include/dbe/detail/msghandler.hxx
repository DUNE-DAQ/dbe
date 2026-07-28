// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/msghandler.hpp to include/dbe/detail/msghandler.hxx).

/************************************************************
 * msghandler.hpp
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
#ifndef LUTILS_MSGHANDLER_HPP_
#define LUTILS_MSGHANDLER_HPP_

#include <algorithm>

#define LUTILS_MESSAGE_LEVEL_ZERO "lutils_nolevel"

namespace lutils
{
namespace program
{

/**
 * Destruction waits to write all messages to file
 */
template<typename UI, typename M, typename T, typename B>
inline msghandler<UI, M, T, B>::~msghandler()
{
	m_running = false;
	CONDITION_STD_SET(m_condition, m_cond_lock, m_have_messages, true);
	if (m_handler != nullptr)
	{
		m_handler->join();
	}
	delete m_handler;
}

/**
 * Constructs the object
 */
template<typename UI, typename M, typename T, typename B>
inline msghandler<UI, M, T, B>::msghandler()
		: m_handler(nullptr),
			m_running(false),
			m_have_messages(false)
{
	this->on();
}

/**
 * Sets the minimum level of messages that the message handler will
 * consider for handling. The rest will be discarded
 *
 * @param level is a string that defines the minimum level
 * @param levels is a vector of strings in order of importance (least important -> most )
 */
template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::setlevel(t_str const & level, t_levels levels)
{
	t_lock l(m_loock);
	m_level = level;
	m_levels = levels;
}

/**
 * Properly sets the logfile
 * @param logfile to write to
 */
template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::set(t_str const & logfile)
{
	reopen(logfile);
}

/**
 * Properly starts the message handler
 */
template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::on()
{
	if (m_handler == nullptr)
	{
		t_lock l(m_loock);
		if (m_handler == nullptr)
		{
			m_running = true;
			m_handler = new t_thread(& msghandler<UI>::handle, this);
		}
	}
}

/**
 * Checks the queue and redirects messages either to a defined output or
 * to a file if a file has been set
 */
template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::handle()
{
	typename t_messages::value_type m;

	while (m_running == true)
	{
		CONDITION_BOOL_WAIT_TRYFUN(m_condition, m_cond_lock, m_have_messages, [&m_running]()
		{
      m_running = false;
    } );
    {
			t_lock l(m_loock);
			while (!m_messages.empty())
			{
				m = m_messages.front();
				m_messages.pop();
				if (levelcheck(m.first))
				{
					if (m_logfile == nullptr)
					{
						post(m.second, m.first);
					}
					else
					{
						file(m.second);
					}
				}
			}

			m_have_messages = false;

		}
	}
}

/**
 * Adds the message in the internal container which will be picked up later
 *
 * @param messagein is the message to be displayed
 */
template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::message(t_str const & in)
{
	message(LUTILS_MESSAGE_LEVEL_ZERO, in);
}

template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::message(t_str const & level, t_str const & in)
{
	{
		t_lock l(m_loock);
		m_messages.push(std::make_pair(level, in));
	}
	CONDITION_STD_SET(m_condition, m_cond_lock, m_have_messages, true);
}

/**
 * Default post action for users classes that define default_post_ret_type
 *
 * @param m the message to display
 */
template<typename UI, typename M, typename T, typename B>
template<typename TR >
inline  typename TR::default_post_ret_type msghandler<UI, M, T, B>::post(
		t_str const & m, t_str const & l)
{
	std::cerr << "[" << l << "]:"<< m << std::endl;
}

/**
 * Post action for users classes that define post_ret_type.
 * These have to also define the post function
 *
 * @param m the message to display
 */
template<typename UI, typename M, typename T, typename B>
template<typename TR >
inline  typename TR::post_ret_type msghandler<UI, M, T, B>::post(
		t_str const & m, t_str const & l)
{
	TR::post(m,l);
}

/**
 * Checks the level of a message and return true if the level of the
 * message is appropriate to be displayed
 *
 * @param level of a message
 * @return true or false
 */
template<typename UI, typename M, typename T, typename B>
inline bool msghandler<UI, M, T, B>::levelcheck(t_str const & level) const
{
	if (m_levels.empty())
	{
		return true;
	}

	typename t_levels::const_iterator minlevel = std::find(m_levels.begin(), m_levels.end(),
																													m_level);
	typename t_levels::const_iterator reqlevel = std::find(m_levels.begin(), m_levels.end(),
																													level);

	if (reqlevel == m_levels.end() || reqlevel < minlevel)
	{
		return false;
	}
	return true;
}

/**
 * Default write to file function
 *
 * This should not be specialized
 *
 * @param m
 */
template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::file(t_str const & m)
{
	*(m_file) << std::endl << m << std::endl;
}

/**
 * Closes the current logfile and opens a new one
 *
 * @param logfile
 */
template<typename UI, typename M, typename T, typename B>
inline void msghandler<UI, M, T, B>::reopen(t_str const & logfile)
{
	t_lock l(m_loock);

	m_logfile = t_str_ptr(new t_str(logfile));

	if (m_file != nullptr)
	{
		while (m_file->is_open())
		{
			m_file->close();
		}
	}
	m_file = t_file_ptr(new std::ofstream);

	while (!m_file->is_open())
	{
		m_file->open(logfile);
	}
}

/**
 * Creates the message handler
 * @return a reference to the created singleton
 */
template<typename UI, typename M, typename T, typename B>
inline msghandler<UI, M, T, B> & msghandler<UI, M, T, B>::ref()
{
	static msghandler<UI> This;
	return This;
}

} /* namespace program */
} /* namespace lutils */


// Facility macro to inform at compile time msg handler that this class provides a post function
#define HAS_POST typedef void post_ret_type;
#define DEFAULT_POST typedef void default_post_ret_type;

#endif /* LUTILS_MSGHANDLER_HPP_ */
