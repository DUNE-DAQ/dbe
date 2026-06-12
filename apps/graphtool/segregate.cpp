/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from src/graphtool/segregate.cpp to apps/graphtool/segregate.cpp).
 */

/*
 * segregate.cpp
 *
 *  Created on: 14 Jun 2016
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/segregate.hpp"
#include "dbe/dbcontroller.hpp"

#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>

#include <array>
#include <future>

#define GTOOL_MAX_THREADS 10

namespace dbe
{
namespace tool
{
namespace graph
{

segregated_graph_write::segregated_graph_write ( std::string const & oprfx,
                                                 size_t const minc,
                                                 size_t const maxc )
  :
  this_dest_prefix ( oprfx ),
  this_min_component_size ( minc ),
  this_max_component_size ( maxc )
{
}

int segregated_graph_write::operator () ( gtool const & tool ) const
{
  typedef long unsigned int t_int;
  gtool::t_graph const & G = tool.getgraph();
  std::vector<t_int> compindices ( boost::num_vertices ( G ) );
  t_int total_connected = boost::connected_components ( G, &compindices[0] );

  std::vector<gtool::t_graph> components ( total_connected );
  std::vector<gtool::t_registry> component_registries ( total_connected );

  // Build the components as seperate graphs
  for ( t_int vector_i = 0; vector_i != compindices.size(); ++vector_i )
  {
    t_int component_j = compindices[vector_i];
    std::string oname = boost::get ( &gtool::vertex_label::uid, G ) [vector_i];
    std::string cname = boost::get ( &gtool::vertex_label::cname, G ) [vector_i];
    gtool::add_object_and_friends (
      components[component_j],
      dbe::inner::dbcontroller::get (
    { oname, cname } ),
    component_registries[component_j] );
  }

  // from the graphs built output only those that are within the range specified

  int max_punits = GTOOL_MAX_THREADS;

  std::vector<std::future<void>> punits;

  unsigned long long int c = 0;

  for ( auto const & g : components )
  {
    size_t const component_size = g.vertex_set().size();

    if ( component_size > this_min_component_size
         and component_size < this_max_component_size )
    {
      if ( --max_punits )
      {
        std::string output_file
        { this_dest_prefix + std::string ( "_" ) + std::to_string ( ++c ) + std::string ( ".dot" ) };

        try
        {
          punits.push_back ( std::async ( std::launch::async, graph::write, g, output_file ) );
        }
        catch ( std::system_error const & e )
        {
          if ( e.code() == std::errc::resource_unavailable_try_again )
          {

            ERROR (
              "Could not launch another thread", e.what(), "for file:", output_file,
              "try to launch in deferred" );

            punits.push_back (
              std::async ( std::launch::deferred, graph::write, g, output_file ) );

            WARN (
              "File write policy change", "Program execution correction", " for file:",
              output_file, " launched in deferred" );

          }
          else
          {
            throw e;
          }
        }
      }
      else
      {
        // The standard dictates ( C++11 ) that wait is called on the destructor
        // and the futures launched are either async or defered, which puts them
        // always in a valid state. All that is needed to do is clear the vector
        // of futures to initiate calling the destructors.
        punits.clear();
        max_punits = GTOOL_MAX_THREADS;
      }
    }
  }

  return EXIT_SUCCESS;
}

} /* namespace graph */
} /* namespace tool */
} /* namespace dbe */
