/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/segregate.h to include/dbe/segregate.hpp).
 */

/*
 * segregate.h
 *
 *  Created on: 14 Jun 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef SRC_GRAPHTOOL_SEGREGATE_H_
#define SRC_GRAPHTOOL_SEGREGATE_H_

#include "dbe/graphtool.hpp"
#include "dbe/gtool.hpp"

namespace dbe
{
namespace tool
{
namespace graph
{

class segregated_graph_write
{
public:
  /**
   * Construct an object from the filename prefix ,the mininimum and maximum size
   * components to be included in the output. Separate files are generated for it component
   *
   * @param ofn is an std::string of the filename prefix
   * @param minc is the minimum size for a component to be considered
   * @param maxc is the maximum sized component to be included in the output
   */
  segregated_graph_write ( std::string const & ofn_prefix, size_t const minc = 0,
                           size_t const maxc = 0 );

  int operator() ( gtool const & ) const;

private:
  std::string const this_dest_prefix;
  size_t const this_min_component_size;
  size_t const this_max_component_size;
};

} /* namespace graph */
} /* namespace tool */
} /* namespace dbe */

#endif /* SRC_GRAPHTOOL_SEGREGATE_H_ */
