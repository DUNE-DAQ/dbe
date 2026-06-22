// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/datahandler.h to include/dbe/datahandler.hpp).

#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "dbe/TableNode.hpp"
#include "dbe/GraphicalClass.hpp"
#include "dbe/dbcontroller.hpp"

#include "conffwk/ConfigObject.hpp"

#include <QModelIndex>
#include <QObject>
#include <QString>

namespace dbe
{
namespace models
{
class table;
class tree;
}

class treenode;

struct ClassViewInfo
{
  bool ShowAllAttributes;
  bool ShowAllRelationships;
  QStringList Attributes;
  QStringList Relationships;
};

struct ViewConfiguration
{
  QString Name;
  QMap<QString, ClassViewInfo> Classes;
  bool DefaultView;
};

class datahandler:
  public QObject
{
  friend class dbe::models::tree;
  friend class dbe::models::table;
  friend class CustomDelegate;
  friend class CustomTableView;

  Q_OBJECT
public:
  datahandler();
  ~datahandler();
  /**
   * Get the root node
   * @return a treenode pointer to the root node
   */
  treenode * getnode() const;
  /**
   * Get a tree node to a class
   * @param ClassName of the class to retrieve treenode to
   * @return the treenode to the class
   */
  treenode * getnode ( QString const & ClassName ) const;
  treenode * getnode ( std::string const & ClassName ) const;
  /**
   * Get a treenode to an object of a class
   * @param ClassName of the class the object belongs to
   * @param ObjectName of the object to retrieve
   * @return
   */
  treenode * getnode ( QString const & ClassName, QString const & ObjectName ) const;
  treenode * getnode ( std::string const & ClassName, std::string const & ObjectName ) const;

  static treenode * findchild ( treenode * top, QString const & name );

  void FetchMore ( const treenode * ClassNode );
  void ResetData();

private:
  /// Tree data structure
  treenode * root;

signals:
  void FetchMoreData ( const treenode * ClassNode );
};

} //end namespace dbe
#endif // DATAHANDLER_H
