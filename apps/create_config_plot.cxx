/************************************************************
 * create_config_plot.cpp
 *
 * JCF, May-7-2024
 *
 * Main file of create_config_plot used to generate GraphViz dot
 * files of DUNE DAQ configurations. The latter can
 * be used to generate graphs that visualize the database
 * patterns
 *
 ************************************************************/
#include "GraphBuilder.hpp"
#include "dbe/confaccessor.hpp"

#include "logging/Logging.hpp"

#include <boost/program_options.hpp>

#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace bop = boost::program_options;

int main ( int argc, char * argv[] )
{
  // Setting language variable to English (otherwise "," is interpreted as "." in numbers)
  setenv ( "LC_ALL", "C", 1 );

  std::string oksfilename, outputfilename;
  std::string level;

  bop::options_description options_description (
    "Allowed options", 128 );

  options_description.add_options()

  ( "help,h", "Provide help message" )

  ( "file,f", bop::value<std::string> ( &oksfilename ), "OKS database file name" )

  ( "level,l", bop::value<std::string>(&level), "base level (session, segment, application, module)")

  ( "output,o", bop::value<std::string> ( &outputfilename ),
    "Output DOT file which can be used as input to GraphViz" );

  bop::variables_map args;

  auto display_help_message = [&options_description]()
  {
    TLOG() 
        << "DBE create_config_plot : Generate dot graphs from database files"
        << std::endl
        << std::endl
        << "Usage: create_config_plot -f/--file <input OKS file> -l/--level <session, segment, application or module> (-o/--output <output DOT file>)"
        << "\nIf no output file is specified the result is sent to stdout"
        << std::endl
        << std::endl
        << options_description
        << std::endl;
  };

  try
  {
    bop::store ( bop::command_line_parser ( argc, argv ).options ( options_description ).run(),
                 args );
    bop::notify ( args );

    const std::map<std::string, dbe::GraphBuilder::TopGraphLevel> level_as_enum {
      {"session", dbe::GraphBuilder::TopGraphLevel::kSession },
      {"segment", dbe::GraphBuilder::TopGraphLevel::kSegment },
      {"application", dbe::GraphBuilder::TopGraphLevel::kApplication },
      {"module", dbe::GraphBuilder::TopGraphLevel::kModule }
    };
    
    if ( args.count ( "help" ) || ! args.count ( "file" ) || ! args.count("level") || ! level_as_enum.count(level) )
    {
      display_help_message();
      return EXIT_FAILURE;
    }

    // Initialize access to configuration backend
    dbe::confaccessor::init();

    
    dbe::GraphBuilder graphbuilder(oksfilename); 
    graphbuilder.construct_graph( level_as_enum.at( level ) );
    
  } catch ( std::string const & e ) {
    ers::error(dbe::GeneralGraphToolError(ERS_HERE, "Program execution failure"));
    return EXIT_FAILURE;
  } catch ( std::exception const & e ) {
    display_help_message();
    
    std::stringstream errmsgstr;
    errmsgstr << "Incorrect command line argument: " << e.what();
    ers::error(dbe::GeneralGraphToolError(ERS_HERE, errmsgstr.str()));
    return EXIT_FAILURE;
  }

  return 0;

}

