// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/graphtool.h to include/dbe/graphtool.hpp).

/************************************************************
 * graphtool.h
 *
 *  Define application wide ( graphtool executable ) constants
 *  and macros. Must be included in top of every source file
 *  which are part of the graphtool application
 *
 *  Created on: Jun 10, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/

#ifndef DBE_GRAPHTOOL_H_
#define DBE_GRAPHTOOL_H_


#define t_messenger dbe::interface::messenger::console

#define MESSAGE_STATICS(msg) \
    static t_msghandler::t_str sms(msg); \
    static t_msghandler::t_str smsr("\tReason: "); \

#endif /* DBE_GRAPHTOOL_H_ */
