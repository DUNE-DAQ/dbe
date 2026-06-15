// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/gtool.h to include/dbe/gtool.hpp).

/************************************************************
 * gtool.h
 *
 *  Created on: Jun 10, 2016
 *      Author: Leonidas Georgopoulos
 ************************************************************/

#ifndef SRC_GTOOL_H_
#define SRC_GTOOL_H_

#include "dbe/config_reference.hpp"
#include "dbe/messenger.hpp"
#include "dbe/dbinfo.hpp"
#include "dbe/tref.hpp"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/labeled_graph.hpp>

#include <string>
#include <vector>
#include <set>

namespace dbe
{
namespace tool
{
namespace graph
{

//------------------------------------------------------------------------------------------
class gtool
{
public:

//    struct edge_label
//    {
//        std::string uid;
//    };

  struct vertex_label
  {
    std::string uid;
    std::string cname;
    std::string label;
  };

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
          vertex_label> t_graph;

  typedef boost::graph_traits<t_graph>::edge_descriptor t_edge;
  typedef boost::graph_traits<t_graph>::vertex_descriptor t_vertex;

  typedef std::map<std::string, t_vertex> t_registry;

  /**
   * Construct gtool object from string a resource location and db information type
   * Resulting object loads a database from the resource location of the designated type
   *
   * @param fn is a string designating the resource location of the database
   * @param dbtype is the type (dbinfo) of the database access method (e.g. oks, rdb ... )
   */
  gtool ( std::string const & rl, dbinfo dbtype );

  template<typename ALGO> inline int load_and_run ( ALGO const & );

  /**
   * Retrieve the graph as it currently is
   *
   * @return a const reference to the t_graph
   */
  t_graph const & getgraph() const;

  /**
   * Add an object to a graph along with all connected objects through any relation
   *
   * @param a reference to the graph
   * @param a registry of tags of objects already present in the graph
   */
  static t_vertex add_object_and_friends ( t_graph &, tref const &, t_registry & registry );

  /**
   * Add an object to a graph without its relations
   *
   * @param g is a reference to the graph
   * @param o is a reference to the object
   * @param uniqueness only add the object if there is no vertex already abstracting it
   *
   * @return the newly added vertex or an already existent vertex if uniqueness is set and
   *          the object has already been added as a vertex
   */
  static t_vertex add_object ( t_graph & g, tref const & o, bool uniqueness = true );

  /**
   * Add an object to a graph without its relations if it is not found in
   * the provided registry
   *
   * @param g is a reference to the graph
   * @param o is a reference to the object
   * @param registry is a set of tags already present , and implies adding the object only
   *        if its tag is not in the registry ( speed up lookups )
   *
   * @return the newly added vertex or an already existent vertex if uniqueness is set and
   *          the object has already been added as a vertex
   */
  static t_vertex add_object ( t_graph & g, tref const & o, t_registry & registry );

  typedef std::pair<bool, boost::graph_traits<gtool::t_graph>::vertex_iterator> t_lookup_ret;
  /**
   * Lookup a vector in the graph , runs in O(N) worst case
   * @param g is the graph
   * @param l is the graph label
   * @return a pair <true, valid vertex iterator > if found , otherwise <false, end iter>
   */
  static t_lookup_ret lookup ( t_graph const & g, vertex_label const & l );
private:

  void load_all();
  void load_all_class_objects ( std::string const & cname );

  void create_graph();

  typedef std::vector<tref> t_objects;

  t_graph this_graph;
  t_objects this_all_objects;
  t_registry already_processed;

  std::string this_src;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Permits to write a graph generated in a gtool as a dot file or to standard output if
 * an epmty string has been specified as argument. Another case if the output file cannot
 * be overwritten in which case.
 */
class writegraph
{
public:
  writegraph ( std::string const & );
  int operator() ( gtool const & ) const;
private:
  void write ( gtool::t_graph const & ) const;
  std::string this_dest;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Send a graph to output or a file. The file is used if the second parameter is not an
 * empty string.
 *
 * @param The graph to send to file
 * @param The file name to write to
 */
void write ( gtool::t_graph const &, std::string const & );
/**
 * Send a graph to standard output
 *
 * @param the graph to write
 */
void write_to_cout ( gtool::t_graph const & );
/**
 * Send a graph to file. If the operation could not be completed EXIT_FAIL is returned
 *
 * @param The graph to write
 * @param The filename to write to
 * @return EXIT_SUCCESS in case of success
 */
void write_to_file ( gtool::t_graph const &, std::string const & );
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename ALGO> inline int gtool::load_and_run ( ALGO const & algo )
{
  load_all();
  create_graph();
  return algo ( static_cast<gtool const &> ( *this ) );
}
//------------------------------------------------------------------------------------------
} /* namespace graph */
} /* namespace tool */
} /* namespace dbe */

#endif /* SRC_GTOOL_H_ */
