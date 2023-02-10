/*
 * confaccessor_test.cpp
 *
 *  Created on: 24 May 2016
 *      Author: Leonidas Georgopoulos
 */

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include "dbe/dbe_test_defs.hpp"
#include "dbe/confaccessor.hpp"

#include <boost/test/unit_test.hpp>
namespace dbe
{
namespace config
{

namespace test
{
struct okstestfix:
										dbe::test::oksfix
{
		okstestfix()
		{
			::dbe::confaccessor::setdbinfo(QString::fromStdString(cdbpath + fn), dbtype);
		}
};

/*
 * Check that the database can be loaded and unloaded
 */
BOOST_FIXTURE_TEST_CASE(load_test,okstestfix)
{
	BOOST_TEST_MESSAGE(std::string("Database location:")+cdbpath+fn);

	BOOST_CHECK(confaccessor::is_database_loaded()==false);
	BOOST_CHECK(confaccessor::load());
	BOOST_CHECK(confaccessor::is_database_loaded());
}

} /* namespace test */

} /* namespace config */
} /* namespace dbe */

