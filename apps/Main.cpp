/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from src/Main.cpp to apps/Main.cpp).
 */

/// Including DBE
#include "dbe/MainWindow.hpp"
#include "dbe/MyApplication.hpp"
/// Including QT Headers
#include <QApplication>
/// Including BOOST
#include <boost/program_options.hpp>

#include <memory>

#include "dbe/msghandler.hpp"
#include "dbe/messenger.hpp"
#include "dbe/types.hpp"
#include "dbe/confaccessor.hpp"

template class ::lutils::program::msghandler<dbe::interface::messenger::qt>;

namespace bop = boost::program_options;

int main(int argc, char *argv[])
{
	/// Setting language variable to english(otherwise , is interpreted as . in numbers)
	setenv("LC_ALL", "C", 1);

	std::unique_ptr<dbe::MyApplication> myapp(new dbe::MyApplication(argc, argv));

	dbe::confaccessor::init();

	std::string configv;
	std::string oksfn;
	std::string rdbrl;
	std::string roksrl;
	QMap<QString, QString> argmap;

	bop::options_description options_description("Allowed options", 128);

	options_description.add_options()
			("help,h", "Provide help message")

            // ("version,v", bop::value<std::string>(&configv)->default_value(configv),
            // "OKS version to load (when the GIT back-end is used - valid only in file mode)")

			("file,f", bop::value<std::string>(&oksfn)->default_value(oksfn),
			"OKS database file name")

			// ("rdb,r", bop::value<std::string>(&rdbrl)->default_value(rdbrl),
			// "RDB resource locator (e.g. rdbServerName@partitionName)")

			// ("roksrl,o",bop::value<std::string>(&roksrl)->default_value(roksrl),
			// "ROKS resource locator (e.g. oracle://atlas_oks/r:atlas_oks_archive:<schema version>:<data version>)")
    ;

	bop::variables_map options_map;

    try {
        bop::parsed_options parsed = bop::command_line_parser(argc, argv).options(options_description).allow_unregistered().run();

        // This is to properly catch any positional argument (not supported)
        const auto& unknown = bop::collect_unrecognized(parsed.options, bop::include_positional);
        if(unknown.size() != 0) {
            std::cerr << "Incorrect command line argument, unrecognized options: ";
            for(const auto& o : unknown) {
                std::cerr << o << " ";
            }
            std::cerr << std::endl;

            return EXIT_FAILURE;
        }

        bop::store(parsed, options_map);
        bop::notify(options_map);

        auto display_help_message = [&options_description]()
        {
            std::cout
            << "DBE: TDAQ online configuration database editor"
            << std::endl
            << std::endl
            << "Usage: dbe [options]"
            << std::endl
            << std::endl
            << options_description
            << std::endl;
        };

        if(options_map.count("help")) {
            display_help_message();
            return EXIT_SUCCESS;
        }

        argmap.insert("f", QString::fromStdString(oksfn));
        // argmap.insert("r", QString::fromStdString(rdbrl));
        // argmap.insert("o", QString::fromStdString(roksrl));
        // argmap.insert("v", QString::fromStdString(configv));

    }
    catch(std::exception const & e) {
        std::cerr << "Incorrect command line argument: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

	std::unique_ptr<dbe::MainWindow> window(new dbe::MainWindow(argmap));
	try
	{
		window->show();
		return myapp->exec();
	}
	catch (std::exception const & e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown Exception" << std::endl;
	}

	return EXIT_FAILURE;
}
