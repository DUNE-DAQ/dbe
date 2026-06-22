// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from src/graphtool.cpp to apps/graphtool.cpp).

/************************************************************
 * graphtool.cpp
 *
 * Main file of graphtool ( aka gtool ) used to generate dot
 * files of the ATLAS configuration database. The latter can
 * be used to generate graphs that visualize the database
 * patterns
 *
 *  Created on: Jun 10, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/
#include "dbe/graphtool.hpp"
#include "dbe/messenger.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/gtool.hpp"
#include "dbe/segregate.hpp"

#include <boost/program_options.hpp>

#include <numeric>

namespace bop = boost::program_options;

int main ( int argc, char * argv[] )
{

  /// Setting language variable to english(otherwise , is interpreted as . in numbers)
  setenv ( "LC_ALL", "C", 1 );

  std::string oksfn, rdbrl, roksrl, outfn, sepfnbase, stats, logfile, msglevel ( "ERROR" );

  size_t min_component_size = 0;
  size_t max_component_size = std::numeric_limits<size_t>::max();

  bop::options_description options_description (
    "Allowed options ( warning : almost no sanity checks performed )", 128 );

  options_description.add_options()

  ( "help,h", "Provide help message" )

  ( "file,f", bop::value<std::string> ( &oksfn ), "OKS database file name" )

  ( "rdb,r", bop::value<std::string> ( &rdbrl ),
    "RDB String (e.g. partition::rdbServerName)" )

  ( "roks,o", bop::value<std::string> ( &roksrl ),
    "ROKS String (e.g. oracle://atlas_oksprod/r:atlas_oks_tdaq:24:77)" )

  ( "result,u", bop::value<std::string> ( &outfn ),
    "Output file which can be used as input to graphviz" )

  ( "separate,s", bop::value<std::string> ( &sepfnbase ),
    "Output pathnames to prepend to subgraph files" )

  ( "stats,S",
    bop::value<std::string> ( &stats )->default_value ( stats ),
    "The statistic to compute can be a comma sperated list \" e.g. min_dist,num_of_component \"..." )

  ( "minc,m", bop::value<size_t> ( &min_component_size )->default_value (
      min_component_size ),
    "The minimum component size to output" )

  ( "maxc,M", bop::value<size_t> ( &max_component_size )->default_value (
      max_component_size ),
    "The maximum component size to output" )

  ( "msglevel,L", bop::value<std::string> ( &msglevel )->default_value ( msglevel ),
    "The minimum level of error messages to report (INFO,WARN,ERROR,FAIL)" )

  ( "log,l", bop::value<std::string> ( &logfile ),
    "Redirect error messages to logfile instead of cerr" );

  bop::variables_map args;

  std::unique_ptr<dbe::tool::graph::gtool> proc;

  auto display_help_message = [&options_description]()
  {
    std::cout
        << "DBE gtool : Generate dot graphs from database files"
        << std::endl
        << std::endl
        << "Usage: dbe_gtool [options] , only the first option is taken into account "
        << "and the output file. If no output file is specified the result is sent to stdout"
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

    // Set the message level to output
    t_msghandler::ref().setlevel ( msglevel, t_messenger::all_levels );

    // Send output to the specified file
    if ( args.count ( "log" ) )
    {
      t_msghandler::ref().set ( logfile );
    }

    // Initialize access to configuration backend
    dbe::confaccessor::init();

    // Select input source
    if ( args.count ( "help" ) or argc == 1 )
    {
      display_help_message();
      return EXIT_FAILURE;
    }
    else if ( args.count ( "file" ) )
    {
      proc = std::unique_ptr<dbe::tool::graph::gtool> (
               new dbe::tool::graph::gtool ( oksfn, dbe::dbinfo::oks ) );
    }
    else if ( args.count ( "rdb" ) )
    {
      proc = std::unique_ptr<dbe::tool::graph::gtool> (
               new dbe::tool::graph::gtool ( rdbrl, dbe::dbinfo::rdb ) );
    }
    else if ( args.count ( "roks" ) )
    {
      proc = std::unique_ptr<dbe::tool::graph::gtool> (
               new dbe::tool::graph::gtool ( roksrl, dbe::dbinfo::roks ) );
    }
    else
    {
      display_help_message();
      return EXIT_FAILURE;
    }

    // If neither separate or result have been provided , output the help message and a warning
    if ( not args.count ( "result" ) and not args.count ( "separate" ) )
    {
      WARN ( "Output file not specified ", "User input warning",
             "Output will be sent to standard output" );
      display_help_message();
    }

  }
  catch ( std::string const & e )
  {
    ERROR ( "Program execution failure", e );
    return EXIT_FAILURE;
  }
  catch ( std::exception const & e )
  {
    ERROR ( "Incorrect command line argument", e.what() );
    display_help_message();
    return EXIT_FAILURE;
  }

  try
  {
    if ( args.count ( "separate" ) )
    {
      INFO ( "Graph will be separated to its components", "Program execution control" );
      dbe::tool::graph::segregated_graph_write sgw ( sepfnbase, min_component_size,
                                                     max_component_size );
      return proc->load_and_run ( sgw );
    }
    else
    {
      INFO ( "One large output file to be created", "Program execution control" );
      dbe::tool::graph::writegraph w ( outfn );
      return proc->load_and_run ( w );
    }
  }
  catch ( std::exception const & e )
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  catch ( ... )
  {
    std::cerr << "Unknown Exception" << std::endl;
  }

  return EXIT_FAILURE;

}
