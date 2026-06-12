/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from src/graphtool/gtool.cpp to apps/graphtool/gtool.cpp).
 */

/************************************************************
 * gtool.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/

#include "dbe/graphtool.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/gtool.hpp"
#include "dbe/messenger.hpp"
#include "dbe/config_api_info.hpp"
#include "dbe/config_reference.hpp"
#include "dbe/config_api.hpp"

#include <QFileInfo>

#include <boost/graph/graphviz.hpp>

#include <vector>
#include <string>
#include <algorithm>

namespace dbe
{
namespace tool
{
namespace graph
{

//------------------------------------------------------------------------------------------
gtool::gtool ( std::string const & rl, dbinfo dbtype )
  :
  this_src ( rl )
{
  QFileInfo DatabaseFile ( QString::fromStdString ( rl ) );

  if ( DatabaseFile.exists() )
  {
    QString path_to_database = QString ( DatabaseFile.absoluteFilePath() );
    {
      confaccessor::setdbinfo ( path_to_database, dbtype );
      INFO ( "Database location set", "Program execution control success" );

      if ( confaccessor::load() )
      {
        INFO ( "Database initialized", "User request" );
      }
      else
      {
        ERROR ( "Could not load database", "OKS error ",
                " Actions : check environment variables e.g. DUNEDAQ_DB_PATH" );
        throw std::string ( "Could not load database" );
      }
    }
  }
  else
  {
    ERROR ( "Cannot open database", "File error", "for file", rl );
  }
}

gtool::t_graph const & gtool::getgraph() const
{
  return this_graph;
}

void gtool::load_all()
{
  INFO ( "Start load objects into internal cache", "Program execution control" );

  std::vector<std::string> all_classes =
  { config::api::info::onclass::allnames<std::vector<std::string>>() };

  for ( auto const & x : all_classes )
  {
    load_all_class_objects ( x );
  }

  INFO ( "All objects loaded into internal cache", "Program execution control success",
         "#objects:", std::to_string ( this_all_objects.size() ) );
}

void gtool::load_all_class_objects ( std::string const & cname )
{
  std::vector<dbe::tref> all_objects = config::api::info::onclass::objects<false> ( cname,
                                                                                    false );

  INFO ( "Class to load: ", "Program execution control", cname );

  for ( dbe::tref const & x : all_objects )
  {
    this_all_objects.push_back ( x );
  }

  INFO ( "Class load success: ", "Program execution control", cname,
         "#objects", std::to_string ( all_objects.size() ) );
}

void gtool::create_graph()
{
  INFO ( "Database graph generation initiated", "Program execution control" );
  already_processed.clear();

  for ( auto const & x : this_all_objects )
  {
    add_object_and_friends ( this_graph, x, already_processed );
  }

  INFO ( "Graph generation completed successfully", "Program execution control success" );
}

gtool::t_vertex gtool::add_object ( gtool::t_graph & g, tref const & x, bool uniqueness )
{
  vertex_label xl
  { x.UID(), x.class_name(), x.full_name() };

  if ( not uniqueness )
  {
    // Multiple vertices with the same properties can be added to this graph
    return boost::add_vertex ( xl, g );
  }
  else
  {
    bool result;
    boost::graph_traits<t_graph>::vertex_iterator viter;
    std::tie ( result, viter ) = lookup ( g, xl );

    if ( result )
    {
      return *viter;
    }
    else
    {
      return boost::add_vertex ( xl, g );

    }
  }
}

gtool::t_vertex gtool::add_object ( gtool::t_graph & g, tref const & x,
                                    t_registry & registry )
{
  t_registry::iterator vat = registry.find ( x.full_name() );

  if ( std::end ( registry ) == vat )
  {
    FULLDEBUG ( "Adding object", "Program execution control", x.full_name() );
    // not in the registry implies it is not in the graph , i.e. its user responsibility
    t_registry::iterator v;
    std::tie ( v, std::ignore ) = registry.emplace ( x.full_name(), add_object ( g, x,
                                                                                 false ) );
    return v->second;
  }
  else
  {
    return vat->second;
  }
}

gtool::t_vertex gtool::add_object_and_friends ( t_graph & g, tref const & o,
                                                t_registry & registry )
{
  DEBUG ( "Processing object", "Program execution control", o.full_name() );
  t_vertex ov = add_object ( g, o, registry );

  std::vector<tref> friends
  { config::api::graph::linked::by::object<tref> ( o ) }; // get all neighbors
  DEBUG ( "Processing object", "Program execution control", o.full_name(), " # friends: ",
          std::to_string ( friends.size() ) );

  for ( tref const & x : friends )
  {
    t_vertex xv = add_object ( g, x, registry );
    boost::add_edge ( ov, xv, g );
    FULLDEBUG ( "Add edge ", "Program excecution control", o.full_name() , "->",
                x.full_name() );
  }

  return ov;
}

gtool::t_lookup_ret gtool::lookup ( t_graph const & g, vertex_label const & l )
{
  auto const & vertices = boost::vertices ( g );

  for ( auto v = vertices.first; v != vertices.second; ++v )
  {
    if ( l.label == boost::get ( &vertex_label::label, g, *v ) )
    {
      return t_lookup_ret ( true, v );
    }
  }

  return t_lookup_ret ( false, vertices.second );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
writegraph::writegraph ( std::string const & s )
  :
  this_dest ( s )
{
}

int writegraph::operator() ( gtool const & x ) const
{
  INFO ( "Saving result", "Program execution control", this_dest );
  write ( x.getgraph() );
  INFO ( "Result sent to output", "Program execution control success", this_dest );
  return EXIT_SUCCESS;
}

void writegraph::write ( gtool::t_graph const & g ) const
{
  graph::write ( g, this_dest );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void write ( gtool::t_graph const & g, std::string const & ofn )
{
  if ( ofn.empty() )
  {
    write_to_cout ( g );
  }
  else
  {
    write_to_file ( g, ofn );
  }
}

void write_to_cout ( gtool::t_graph const & g )
{
  INFO ( "Sending output to stdout", "Program execution control" )
  boost::write_graphviz (
    std::cout, g, boost::make_label_writer ( boost::get ( &gtool::vertex_label::label, g ) ) );
}

void write_to_file ( gtool::t_graph const & g, std::string const & ofn )
{
  std::ofstream of;
  of.open ( ofn );

  if ( of.is_open() )
  {
    INFO ( "Sending output to file", "Program execution control", "File name:", ofn );
    boost::write_graphviz (
      of, g, boost::make_label_writer ( boost::get ( &gtool::vertex_label::label, g ) ) );

    of.close();

    if ( of.fail() )
    {
      ERROR ( "Could not close file", "Program execution control", "File name:", ofn );
    }
    else
    {
      NOTE ( "Output written to file", "Program execution control success", "File name:", ofn );
    }
  }
  else
  {
    ERROR (
      "Output could not be written file", "Stream could not be opened", "File name:", ofn );
    write_to_cout ( g );
  }
}
//------------------------------------------------------------------------------------------

} /* namespace graph */
} /* namespace tool */
} /* namespace dbe */
