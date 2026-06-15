// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/tree.h to include/dbe/tree.hpp).

#ifndef TREEMODEL_H
#define TREEMODEL_H

/// Including QT Headers
#include <QAbstractItemModel>
/// Including TDAq
#include "conffwk/ConfigObject.hpp"
/// Including DBE
#include "dbe/TreeModelInterface.hpp"
#include "dbe/model_common_interface.hpp"

extern char const * const dbe_lib_structure_version;

namespace dbe
{
class treenode;

namespace models
{

class tree:
  public QAbstractItemModel,
  public TreeModelInterface,
  public model_common_impl<tree>,
  public model_common_async_operations<tree>
{
  Q_OBJECT

  MODEL_COMMON_IMPL_REQ_DEF ( tree )

public :
  typedef dbe::treenode type_datum;

  ~tree();

  explicit tree ( const QStringList & Headers, QObject * parent = nullptr );


  type_index index ( int row, int column, const type_index & parent ) const override;

  type_index parent ( const type_index & child ) const override;

  type_index mapToSource ( type_index const & i )
  {
    return i;
  }

  int rowCount ( const type_index & parent ) const override;

  int columnCount ( const type_index & parent ) const override;

  Qt::ItemFlags flags ( const type_index & index ) const override;

  QVariant data ( const type_index & index, int role = Qt::DisplayRole ) const override;

  QVariant headerData ( int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole ) const override;

  bool insertRows ( int position, int rows, const type_index & parent ) override;

  void fetchMore ( const type_index & parent ) override;

  bool canFetchMore ( const type_index & parent ) const override;

  bool hasChildren ( const type_index & parent ) const override;

  bool setData ( const type_index & index, const QVariant & value, int role ) override;

  QStringList mimeTypes() const override;

  QMimeData * mimeData ( const QModelIndexList & indexes ) const override;

  type_datum * getnode ( const type_index & index ) const override;

  type_index getindex ( treenode * NodeItem,
                        type_index const & RootIndex = QModelIndex() ) const;

  void ResetModel();

  void objectsUpdated(const std::vector<dbe::dref>& objects);

signals:
  void ObjectFile ( QString );

private:
  bool abstract_classes_selectable;

private slots:
  void ToggleAbstractClassesSelectable ( bool );

  void slot_create_object ( QString const & src, dref const & obj );
  void slot_remove_object ( QString const & src, dref const & obj );
  void slot_update_object ( QString const & src, dref const & obj );
  void slot_rename_object ( QString const & src, dref const & obj );
};

}
// namespace models
}// namespace dbe
#endif // TREEMODEL_H
