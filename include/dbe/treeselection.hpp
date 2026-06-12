/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/treeselection.h to include/dbe/treeselection.hpp).
 */

#ifndef TREESELECTIONMODEL_H
#define TREESELECTIONMODEL_H

#include "dbe/treenode.hpp"
#include "dbe/TreeModelInterface.hpp"
#include "dbe/model_common_interface.hpp"
#include "dbe/dbcontroller.hpp"

#include "conffwk/ConfigObject.hpp"

#include <QSortFilterProxyModel>
#include <QUuid>

namespace dbe
{
namespace models
{

class treeselection:
  public QSortFilterProxyModel,
  public TreeModelInterface,
  public model_common_impl<treeselection>,
  public model_common_async_operations<treeselection>
{
  Q_OBJECT

  MODEL_COMMON_IMPL_REQ_DEF ( treeselection )

public:

  typedef treenode type_datum;

  enum FilterType
  {
    RegExpFilterType = 1, ObjectFilterType = 2
  };

  ~treeselection();

  explicit treeselection ( QObject * parent = 0 );


  type_datum * getnode ( type_index const & index ) const override;

  void fetchMore ( type_index const & index );

  bool hasChildren ( type_index const & index ) const;

  bool canFetchMore ( type_index const & index ) const;

  void SetFilterType ( FilterType Filter );
  void SetFilterRestrictionLevel ( int Levels );
  void SetQueryObjects ( std::vector<tref> Objects );

  std::vector<dbe::tref> GetQueryObjects();

  void ResetQueryObjects();
  void ResetModel();

protected:
  bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;

  bool lessThan ( const QModelIndex & left, const QModelIndex & right ) const;

private slots:
  void ToggleEmptyClasses ( bool HideLocal );

private:
  bool AcceptItem ( const QModelIndex & SourceIndex, int LevelRestriction ) const;

  bool RegexpFilter ( int sourceRow, const QModelIndex & sourceParent ) const;

  bool ObjectFilter ( int sourceRow, const QModelIndex & sourceParent ) const;

  int AtDepth ( const QModelIndex & SourceParent ) const;

  FilterType Type;
  int LevelRestriction;

  bool Hide;

  std::vector<tref> QueryObjects;

private slots:
  void slot_create_object ( QString const & src, dref const & obj );
  void slot_remove_object ( QString const & src, dref const & obj );
  void slot_update_object ( QString const & src, dref const & obj );
  void slot_rename_object ( QString const & src, dref const & obj );
};

}
// namespace models
}// namespace dbe
#endif // TREESELECTIONMODEL_H
