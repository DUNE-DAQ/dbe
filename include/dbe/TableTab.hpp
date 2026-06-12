/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/TableTab.h to include/dbe/TableTab.hpp).
 */

#ifndef TABLETAB_H
#define TABLETAB_H

/// Include Qt
#include "dbe/table.hpp"
#include "dbe/tableselection.hpp"

#include <QString>
#include <QTabWidget>
/// Including dbe
#include "dbe/CustomDelegate.hpp"
#include "dbe/CustomTableView.hpp"

namespace dbe
{

class TableTab: public QWidget
{
  Q_OBJECT
public:
  TableTab ( QWidget * parent = 0 );
  ~TableTab();
  dbe::models::table * GetTableModel() const;
  dbe::models::tableselection * GetTableFilter() const;
  CustomDelegate * GetTableDelegate() const;
  CustomTableView * GetTableView() const;
  void CreateModels();
  void DisconnectView();
  void ResetTableView();
  void ResizeHeaders();

protected:
  void dropEvent ( QDropEvent * event ) override;
  void dragEnterEvent ( QDragEnterEvent * event ) override;

private:
  dbe::models::table * this_model;
  dbe::models::tableselection * this_filter;
  CustomDelegate * this_delegate;
  CustomTableView * this_view;
  QTabWidget * this_holder;

  void create_models();
  void reset_models();


public slots:
  void ResetTabView();

signals:
  void sig_data_dropped ( QMimeData const & data, Qt::DropAction );

};

}  // namespace dbe
#endif // TABLETAB_H
