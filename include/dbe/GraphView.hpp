// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/GraphView.h to include/dbe/GraphView.hpp).

#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

/// Including QT Headers
#include <QWidget>
#include <QUuid>

/// Including DBE
#include "dbe/GraphicalClass.hpp"
/// Including Ui Files
#include "ui_GraphView.h"

#include "dbe/confaccessor.hpp"

namespace dbe
{
namespace Ui
{
class GraphView;
}  // namespace Ui

class GraphView: public QWidget, private Ui::GraphView
{
  Q_OBJECT
public:
  ~GraphView();
  explicit GraphView ( QWidget * parent = nullptr );

  void ConnectActions();
  void SetupView();
  void contextMenuEvent ( QContextMenuEvent * Event );

private slots:
  void GetWindowConfiguration();
  void RedrawObject ( tref Object );
  void RedrawObject();
  void CreateActions();
  void editThisObject();
  void deleteThisObject();
  void copyObject();
  void referencedBy_All();
  void referencedBy_OnlyComposite();

private:
  Window WindowConfiguration;
  //Qt::DropActions supportedDropActions() const;
  QMenu * ContextMenu;
  QAction * editObject;
  QAction * deleteObjectAc;
  QAction * refByAc;
  QAction * refByAcOnlyComp;
  QAction * copyObjectAc;
  GraphicalObject * ClickedItem;
  QUuid const uuid;
};
} // end namespace dbe
#endif // GRAPHVIEW_H
