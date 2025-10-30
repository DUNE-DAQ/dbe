#ifndef CUSTOMTREEVIEW_H
#define CUSTOMTREEVIEW_H

#include "dbe/view_common_interface.hpp"
#include "dbe/config_reference.hpp"
#include "dbe/dbcontroller.hpp"

#include "conffwk/ConfigObject.hpp"

#include "ers/ers.hpp"

#include <QTreeView>
#include<QUuid>


namespace dbe
{
class CustomTreeView: public QTreeView , public view_common_interface
{
  Q_OBJECT
public:
  explicit CustomTreeView ( QWidget * Parent = nullptr );
  void contextMenuEvent ( QContextMenuEvent * Event );

protected:
  void closeEvent(QCloseEvent * event) override;

private slots:
  void slot_delete_objects();
  void slot_create_object();
  void slot_edit_object();
  void slot_copy_object();

  void referencedbyOnlycomposite();
  void referencedByAll();

public slots:
  void referencedBy ( bool All );
  void referencedBy ( bool All, tref Object );
signals:
  void OpenEditor ( tref Object );

private:
  void edit_object ( QModelIndex const & );
  void CreateActions();

  QMenu * contextMenu;
  QAction * editObjectAc;
  QAction * deleteObjectAc;
  QAction * createObjectAc;
  QAction * copyObjectAc;
  QAction * deleteObjectWidgetAc;
  QAction * hideShowAc;
  QAction * buildTableFromClassAc;
  QAction * expandAllAc;
  QAction * collapseAllAc;
  QAction * refByAc;
  QAction * refByAcOnlyComp;
};
} // end namespace dbe
#endif // CUSTOMTREEVIEW_H
