// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from test/config_api_info_test.cpp to unittest/config_api_info_test.cxx).

/*
 * config_api_test.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: Leonidas Georgopoulos
 */

#define BOOST_TEST_MAIN

#include "dbe_test_defs.hpp"
#include "dbe/config_api_info.hpp"
#include "dbe/config_api.hpp"
#include "dbe/confaccessor.hpp"

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <iostream>

namespace dbe
{
namespace config
{
namespace test
{
//------------------------------------------------------------------------------------------
struct class_facilities_fix:
															dbe::test::oksfix
{
		std::string const class_not_existent = "foo-man-tsu";

		std::string const class_abstract = "Application";

		std::string const class_non_abstract = "Detector";
		std::string const object_in_non_abstract_class = "ALFA";

		class_facilities_fix()
		{
			confaccessor::setdbinfo(QString::fromStdString(cdbpath + fn), dbtype);
			confaccessor::load();
		}
};

BOOST_FIXTURE_TEST_SUITE(info_class_facilities_suite, class_facilities_fix)

BOOST_AUTO_TEST_CASE(get_allclasses_check)
{
	BOOST_CHECK(confaccessor::is_database_loaded());
	{
		std::vector<std::string> allclasses =
		{ config::api::info::onclass::allnames<std::vector<std::string>>() };

		BOOST_CHECK(not allclasses.empty());
	}
	{
		// can be loaded as a qstringlist
		QStringList allclasses =
		{ config::api::info::onclass::allnames<QStringList>() };

		BOOST_CHECK(not allclasses.empty());
	}
}

BOOST_AUTO_TEST_CASE(on_not_existent_class)
{
	BOOST_CHECK(confaccessor::is_database_loaded());
	{
		// catches the underlying NotFound exception
		BOOST_CHECK_NO_THROW(api::info::onclass::definition(class_not_existent, false));

		// Returns an empty object
		dunedaq::conffwk::class_t definition =
		{ api::info::onclass::definition(class_not_existent, false) };

		BOOST_CHECK(definition.p_name.empty());
		BOOST_CHECK(not definition.p_abstract);
		BOOST_CHECK(definition.p_description.empty());
		BOOST_CHECK(definition.p_relationships.empty());
		BOOST_CHECK(definition.p_attributes.empty());
		BOOST_CHECK(definition.p_superclasses.empty());
		BOOST_CHECK(definition.p_subclasses.empty());
	}
}

BOOST_AUTO_TEST_CASE(on_abstract_class_definition)
{
	BOOST_CHECK(confaccessor::is_database_loaded());
	{
		BOOST_CHECK_NO_THROW(api::info::onclass::definition(class_abstract, false));

		dunedaq::conffwk::class_t definition =
		{ api::info::onclass::definition(class_abstract, false) };

		BOOST_CHECK(definition.p_abstract);
	}

	{
		BOOST_CHECK_NO_THROW(api::info::onclass::definition(class_non_abstract, false));
		dunedaq::conffwk::class_t definition =
		{ api::info::onclass::definition(class_non_abstract, false) };

		BOOST_CHECK_EQUAL(definition.p_abstract, false);
	}

}

BOOST_AUTO_TEST_CASE(on_class_definition)
{
	{
		// Can retrieve definition for a class and not-only direct relations
		BOOST_CHECK_NO_THROW(api::info::onclass::definition(class_non_abstract, false));
		dunedaq::conffwk::class_t definition = api::info::onclass::definition(class_non_abstract,
		false);
		BOOST_CHECK_EQUAL(definition.p_name, class_non_abstract);
		BOOST_CHECK(not definition.p_abstract);
		BOOST_CHECK(not definition.p_description.empty());
		BOOST_CHECK(not definition.p_relationships.empty());
		BOOST_CHECK(not definition.p_attributes.empty());
		BOOST_CHECK(not definition.p_superclasses.empty());
		BOOST_CHECK(definition.p_subclasses.empty());
	}
	{

	 
		// Can retrieve definition for a class and direct only relations
		BOOST_CHECK_NO_THROW(api::info::onclass::definition(class_non_abstract, true));
		dunedaq::conffwk::class_t result_class = api::info::onclass::definition(class_non_abstract,
		true);

		BOOST_CHECK_EQUAL(result_class.p_name, class_non_abstract);
		BOOST_CHECK(not result_class.p_abstract);
		BOOST_CHECK(not result_class.p_description.empty());
		BOOST_CHECK(result_class.p_relationships.empty()); // class has no direct only relations
		BOOST_CHECK(not result_class.p_attributes.empty());
		BOOST_CHECK(not result_class.p_superclasses.empty());
		BOOST_CHECK(result_class.p_subclasses.empty());
	}
}

BOOST_AUTO_TEST_SUITE_END()
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
struct object_facilities_fix:
												dbe::test::oksfix
{
		std::string const class_abstract = "ComputerProgram";
		std::string const class_non_abstract = "Binary";
		std::string const class_derived_from_non_abstract_class = "ABinaryDerivedClass";
		std::string const object_in_non_abstract_class = "rc_controller";
		std::string const object_derived_from_non_abstract_class = "InstanceOfABinaryDerivedClass";

		object_facilities_fix()
		{
			confaccessor::setdbinfo(QString::fromStdString(cdbpath + fn), dbtype);
			confaccessor::load();
		}
};

BOOST_FIXTURE_TEST_SUITE(info_object_facilities_check, object_facilities_fix)

BOOST_AUTO_TEST_CASE(object_facilities_check)
{
	BOOST_CHECK(confaccessor::is_database_loaded());
	{
		// Check that has_obj accepts std::string of class Detector for object ALFA
		BOOST_CHECK(api::info::has_obj(class_non_abstract, object_in_non_abstract_class));
	}
}

BOOST_AUTO_TEST_CASE(objects_of_class)
{
	std::vector<dbe::inner::configobject::tref> const & oinclass =
			api::info::onclass::objects(class_non_abstract, false);
	{
		auto foundpos = std::find_if(oinclass.begin(), oinclass.end(),
																	[&](dbe::inner::configobject::tref const x)
																	{	return x.UID() == object_in_non_abstract_class;});
		BOOST_CHECK(foundpos != oinclass.end());
	}

	std::vector<dbe::inner::configobject::tref> const & derivedinclass =
			api::info::onclass::objects(class_non_abstract, true);
	{
		auto foundpos = std::find_if(
				derivedinclass.begin(), derivedinclass.end(),
				[&](dbe::inner::configobject::tref const x)
				{	return x.UID() == object_derived_from_non_abstract_class;});
		BOOST_CHECK(foundpos != derivedinclass.end());
	}

	BOOST_CHECK_NE(oinclass.size(), derivedinclass.size());
}

BOOST_AUTO_TEST_CASE(derived_classes)
{
	BOOST_CHECK(api::info::onclass::derived(class_abstract, class_non_abstract));
	BOOST_CHECK(
			api::info::onclass::derived(class_abstract, class_derived_from_non_abstract_class));
	BOOST_CHECK(
			api::info::onclass::derived(class_non_abstract,
																	class_derived_from_non_abstract_class));
}

BOOST_AUTO_TEST_SUITE_END()
//------------------------------------------------------------------------------------------
}/* namespace test */
} /* namespace config */
} /* namespace dbe */
