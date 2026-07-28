// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/tableselection.h to include/dbe/tableselection.hpp).

#ifndef TABLESELECTIONMODEL_H
#define TABLESELECTIONMODEL_H

/// Including QT Headers
#include "dbe/table.hpp"

#include <QSortFilterProxyModel>
/// Including DBE
#include "dbe/TableNode.hpp"
#include "dbe/model_common_interface.hpp"

namespace dbe
{
namespace models
{

class tableselection:
  public QSortFilterProxyModel,
  public model_common_impl<tableselection>,
  public model_common_async_operations<tableselection>
{
  Q_OBJECT

  MODEL_COMMON_IMPL_REQ_DEF ( tableselection )

public :

  typedef TableNode type_datum;

  enum FilterType
  {
    RegExpFilter = 1, ExactFilter = 2
  };

  tableselection ( QObject * parent = 0 );

  void SetFilterType ( FilterType Filter );

  TableNode * getnode ( const QModelIndex & index ) const;

  bool setData ( const QModelIndex & index, const QVariant & value, int role );

  void ResetModel();

protected:
  bool filterAcceptsRow ( int sourceRow, const QModelIndex & sourceParent ) const;

  bool lessThan ( const QModelIndex & left, const QModelIndex & right ) const;

private:
  QVariant LastSavedValue;
  FilterType Type;

private slots:
  void slot_create_object ( QString const & src, dref const & obj );
  void slot_remove_object ( QString const & src, dref const & obj );
  void slot_update_object ( QString const & src, dref const & obj );
  void slot_rename_object ( QString const & src, dref const & obj );
};

}
// namespace models
}// namespace dbe
#endif // TABLESELECTIONMODEL_H
