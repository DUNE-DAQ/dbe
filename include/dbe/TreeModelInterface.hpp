// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/TreeModelInterface.h to include/dbe/TreeModelInterface.hpp).

#ifndef TREEMODELINTERFACE_H
#define TREEMODELINTERFACE_H

#include "dbe/treenode.hpp"
#include "dbe/Exceptions.hpp"

#include "conffwk/Configuration.hpp"
#include "conffwk/ConfigObject.hpp"
#include "conffwk/Schema.hpp"

#include <QModelIndex>
#include <vector>
#include <algorithm>

namespace dbe
{

class TreeModelInterface
{
public:
  virtual treenode * getnode ( const QModelIndex & index ) const = 0;

  virtual ~TreeModelInterface() = default;
};

}  // namespace dbe

#define TREEMODEL_REMOVE_ROWS_DEF(classname) \
bool classname::removeRows(int row, int count, QModelIndex const & parent) \
{\
  \
  assert(count > 0);\
  \
  int const last = row + count - 1;\
  emit beginRemoveRows(parent, row, last);\
  for (int current = last; current != row - 1; --current)\
  {\
    QModelIndex src_index = this->mapToSource(index(current, parent.column(), parent));\
    treenode * element = this->getnode(src_index);\
    \
    if (element != nullptr)\
    {\
      treenode * element_parent = element->GetParent();\
      if (element_parent != nullptr)\
      {\
        element_parent->RemoveChild(element);\
      }\
    }\
    else\
    {\
      return false;\
    }\
  }\
  emit endRemoveRows();\
  return true;\
}\

#endif // TREEMODELINTERFACE_H
