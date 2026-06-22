// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from src/SchemaEditor/SchemaMain.cpp to apps/SchemaMain.cpp).

#include "dbe/SchemaMainWindow.hpp"
#include <QApplication>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

namespace bop = boost::program_options;

int main(int argc, char *argv[]) {
    std::string oksfn;

    bop::options_description options_description("Allowed options", 128);

    options_description.add_options()("help,h", "Provide help message")
                                     ("file,f", bop::value<std::string>(&oksfn)->default_value(oksfn), "OKS schema file name");

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
            << "TDAQ online database schema editor"
            << std::endl
            << std::endl
            << "Usage: schemaeditor [options]"
            << std::endl
            << std::endl
            << options_description
            << std::endl;
        };

        if(options_map.count("help")) {
            display_help_message();
            return EXIT_SUCCESS;
        }
    }
    catch(std::exception const & e) {
        std::cerr << "Incorrect command line argument: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    QApplication a(argc, argv);

    dbse::SchemaMainWindow w(QString::fromStdString(oksfn));

    w.show();

    return a.exec();
}
