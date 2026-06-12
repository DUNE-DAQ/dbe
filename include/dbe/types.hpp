/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/types.h to include/dbe/types.hpp).
 */

/*
 * types.h
 *
 *  Created on: Nov 3, 2015
 *      Author: lgeorgop
 */

#ifndef DBE_TYPES_H_
#define DBE_TYPES_H_

#include <mutex>

namespace dbe
{
namespace types
{
namespace common
{

typedef std::mutex type_mutex;
typedef std::lock_guard<type_mutex> type_lock;

}
}
}

#endif /* DBE_TYPES_H_ */
