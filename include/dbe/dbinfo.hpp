// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/dbinfo.h to include/dbe/dbinfo.hpp).

/*
 * dbinfo.h
 *
 *  Created on: 25 May 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_DBINFO_H_
#define DBE_DBINFO_H_

namespace dbe
{
//------------------------------------------------------------------------------------------
enum class dbinfo
{
  oks,
  roks,
  rdb
};
//------------------------------------------------------------------------------------------
}

#endif /* DBE_DBINFO_H_ */
