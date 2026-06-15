// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from test/dbe_test_defs.hpp to unittest/dbe_test_defs.hpp).

/*
 * dbe_test_defs.hpp
 *
 *  Created on: 25 May 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_TEST_DEFS_HPP_
#define DBE_TEST_DEFS_HPP_

#include "dbe/dbinfo.hpp"
#include <unistd.h>
#include <string>
#include <cstdlib>

namespace dbe {
namespace test {

/**
 * Base class for test fixes defines oks database location for using in unit tests
 */
struct oksfix {
        const ::dbe::dbinfo dbtype = ::dbe::dbinfo::oks;
        const std::string fn = "dbe_unittest.data.xml";
        const std::string cdbpath = std::string(getenv("DBE_SHARE")) + "/schema/dbe/";
};

}
}

#endif /* DBE_TEST_DEFS_HPP_ */
