// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/version.h to include/dbe/version.hpp).

/************************************************************
 * version.h
 *
 *  Created on: Jun 9, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/

#ifndef DBE_VERSION_H_
#define DBE_VERSION_H_

#define xstrexp(s) strexp(s)
#define strexp(s) #s

#ifdef DBE__VERSION_INFO
#define dbe_compiled_version xstrexp(DBE__VERSION_INFO)
#else
#define dbe_compiled_version "compiled without version information"
#endif

#ifdef DBE__COMMIT_INFO
#define dbe_compiled_commit xstrexp(DBE__COMMIT_INFO)
#else
#define dbe_compiled_commit "compiled without commit information"
#endif


#endif /* DBE_VERSION_H_ */
