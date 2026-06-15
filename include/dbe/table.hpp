// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/table.h to include/dbe/table.hpp).

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include "dbe/dbcontroller.hpp"
#include "dbe/TableNode.hpp"
#include "dbe/model_common_interface.hpp"

#include "conffwk/ConfigObject.hpp"

#include <QAbstractTableModel>

#include <memory>

namespace dbe
{
namespace models
{
class table:
  public QAbstractTableModel,
  public model_common_impl<table>,
  public model_common_async_operations<table>
{
  Q_OBJECT

  MODEL_COMMON_IMPL_REQ_DEF ( table )

public :

  typedef TableNode type_datum;

  explicit table ( QObject * parent = nullptr );
  ~table();

  int rowCount ( type_index const & parent ) const override;
  int columnCount ( type_index const & parent ) const override;

  QVariant data ( type_index const & index, int role ) const override;

  bool setData ( type_index const & index, const QVariant & value, int role ) override;

  QVariant headerData ( int section, Qt::Orientation orientation, int role ) const override;

  Qt::ItemFlags flags ( type_index const & index ) const override;

  TableNode * getnode ( const QModelIndex & Index ) const;

  QStringList mimeTypes() const override;

  Qt::DropActions supportedDropActions() const override;

  bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column,
                      type_index const & parent ) override;

  void ResetModel();

  bool BuildTableFromClass ( const QString & ClassName, bool BuildSubClasses = false );

  bool BuildTableFromObject ( QList<QStringList> BuildList );

  bool is_built() const;
  QString get_class_name() const;

  tref GetTableObject ( int ObjectIndex ) const;

  QList<dbe::dref> * GetTableObjects();

  void objectsUpdated(const std::vector<dbe::dref>& objects);

private:
  bool enabled;
  QString this_class_name;
  dunedaq::conffwk::class_t class_type_info;

  QStringList this_headers;
  QList<dref> this_objects;
  QList<QList<TableNode * >> this_structure;

  /**
   * Clear internal structure and set the name to the given class
   * @param the class name to set
   */
  void reset ( QString const & );

  /**
   * Set headers as designate by the given class
   *
   * @param the dunedaq::conffwk::class_t information for the class
   */
  void setheader ( dunedaq::conffwk::class_t const & );

  QList<type_datum *> createrow ( treenode const * );

public slots:
  void slot_data_dropped ( QMimeData const &, Qt::DropAction );

private slots:
  void slot_create_object ( QString const & src, dref const & obj );
  void slot_remove_object ( QString const & src, dref const & obj );
  void slot_update_object ( QString const & src, dref const & obj );
  void slot_rename_object ( QString const & src, dref const & obj );

signals:
  void ResetTab();
};

}
// namespace models
}// namespace dbe
#endif // TABLEMODEL_H
