/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/// Including QT Headers
#include "dbe/subtreeproxy.hpp"
#include "dbe/tree.hpp"

#include "dbe/treenode.hpp"
#include "dbe/treeselection.hpp"

#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

/// Including DBE
#include "dbe/CustomTreeView.hpp"
#include "dbe/TreeModelInterface.hpp"
#include "dbe/ObjectCreator.hpp"
#include "dbe/ObjectEditor.hpp"
#include "dbe/messenger.hpp"

//------------------------------------------------------------------------------------------
dbe::CustomTreeView::CustomTreeView ( QWidget * Parent )
  : QTreeView ( Parent ),
    contextMenu ( nullptr )
{
  setDragEnabled ( true );
  setSortingEnabled ( true );
  setExpandsOnDoubleClick ( false );
  CreateActions();
  setSelectionMode ( QAbstractItemView::ExtendedSelection );
  setUniformRowHeights(true);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::contextMenuEvent ( QContextMenuEvent * Event )
{
  if ( contextMenu == nullptr )
  {
    contextMenu = new QMenu ( this );
    contextMenu->addAction ( editObjectAc );
    contextMenu->addAction ( deleteObjectAc );
    contextMenu->addAction ( createObjectAc );
    contextMenu->addAction ( deleteObjectWidgetAc );
    contextMenu->addAction ( copyObjectAc );
    contextMenu->addAction ( buildTableFromClassAc );
    contextMenu->addAction ( expandAllAc );
    contextMenu->addAction ( collapseAllAc );
    contextMenu->addAction ( refByAc );
    contextMenu->addAction ( refByAcOnlyComp );
  }

  model_common_interface * modelInterface =
    dynamic_cast<model_common_interface *> ( this->model() );
  QModelIndex index = this->currentIndex();

  if ( index.isValid() )
  {
    if ( modelInterface )
    {

      try
      {
        tref obj = modelInterface->getobject ( index );
        ( contextMenu->actions() ).at ( 0 )->setVisible ( true );
        ( contextMenu->actions() ).at ( 1 )->setVisible ( true );
        ( contextMenu->actions() ).at ( 2 )->setVisible ( true );
        ( contextMenu->actions() ).at ( 3 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 4 )->setVisible ( true );
        ( contextMenu->actions() ).at ( 5 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 6 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 7 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 8 )->setVisible ( true );
        ( contextMenu->actions() ).at ( 9 )->setVisible ( true );
      }
      catch ( daq::dbe::cannot_handle_invalid_qmodelindex const & e )
      {
        ( contextMenu->actions() ).at ( 0 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 1 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 2 )->setVisible ( true );
        ( contextMenu->actions() ).at ( 3 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 4 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 5 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 6 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 7 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 8 )->setVisible ( false );
        ( contextMenu->actions() ).at ( 9 )->setVisible ( false );
      }
    }

    contextMenu->exec ( Event->globalPos() );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::CreateActions()
{

  editObjectAc = new QAction ( tr ( "&Edit Object" ), this );
  editObjectAc->setShortcut ( tr ( "Ctrl+E" ) );
  editObjectAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( editObjectAc, SIGNAL ( triggered() ), this, SLOT ( slot_edit_object() ) );
  addAction ( editObjectAc );

  deleteObjectAc = new QAction ( tr ( "&Delete Object" ), this );
  deleteObjectAc->setShortcut ( tr ( "Ctrl+D" ) );
  deleteObjectAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( deleteObjectAc, SIGNAL ( triggered() ), this, SLOT ( slot_delete_objects() ) );
  addAction ( deleteObjectAc );

  createObjectAc = new QAction ( tr ( "Create &New Object" ), this );
  createObjectAc->setShortcut ( tr ( "Ctrl+N" ) );
  createObjectAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( createObjectAc, SIGNAL ( triggered() ), this, SLOT ( slot_create_object() ) );
  addAction ( createObjectAc );

  copyObjectAc = new QAction ( tr ( "&Copy This Object Into A New One" ), this );
  copyObjectAc->setShortcut ( tr ( "Ctrl+Shift+N" ) );
  copyObjectAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( copyObjectAc, SIGNAL ( triggered() ), this, SLOT ( slot_copy_object() ) );
  addAction ( copyObjectAc );

  deleteObjectWidgetAc = new QAction ( tr ( "Delete Object &Widget" ), this );
  deleteObjectWidgetAc->setShortcut ( tr ( "Ctrl+W" ) );
  deleteObjectWidgetAc->setShortcutContext ( Qt::WidgetShortcut );
  addAction ( deleteObjectWidgetAc );

  buildTableFromClassAc = new QAction ( tr ( "Build Tab&le From Class" ), this );
  buildTableFromClassAc->setShortcut ( tr ( "Ctrl+L" ) );
  buildTableFromClassAc->setShortcutContext ( Qt::WidgetShortcut );
  addAction ( buildTableFromClassAc );

  expandAllAc = new QAction ( tr ( "Expand &All" ), this );
  expandAllAc->setShortcut ( tr ( "Ctrl+A" ) );
  expandAllAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( expandAllAc, SIGNAL ( triggered() ), this,
            SLOT ( expandAll() ) ); // QTreeView slot
  addAction ( expandAllAc );

  collapseAllAc = new QAction ( tr ( "C&ollapse All" ), this );
  collapseAllAc->setShortcut ( tr ( "Ctrl+Shift+C" ) );
  collapseAllAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( collapseAllAc, SIGNAL ( triggered() ), this,
            SLOT ( collapseAll() ) ); // QTreeView slot
  addAction ( collapseAllAc );

  refByAc = new QAction ( tr ( "&Referenced By (All objects)" ), this );
  refByAc->setShortcut ( tr ( "Ctrl+Y" ) );
  refByAc->setShortcutContext ( Qt::WidgetShortcut );
  refByAc->setToolTip ( "Find all objects which reference the selecetd object" );
  refByAc->setStatusTip ( refByAc->toolTip() );
  connect ( refByAc, SIGNAL ( triggered() ), this, SLOT ( referencedByAll() ) );
  addAction ( refByAc );

  refByAcOnlyComp = new QAction ( tr ( "Referenced B&y (Only Composite)" ), this );
  refByAcOnlyComp->setShortcut ( tr ( "Ctrl+Shift+Y" ) );
  refByAcOnlyComp->setShortcutContext ( Qt::WidgetShortcut );
  refByAcOnlyComp->setToolTip (
    "Find objects (ONLY Composite ones) which reference the selecetd object" );
  refByAcOnlyComp->setStatusTip ( refByAcOnlyComp->toolTip() );
  connect ( refByAcOnlyComp, SIGNAL ( triggered() ), this,
            SLOT ( referencedbyOnlycomposite() ) );
  addAction ( refByAcOnlyComp );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::slot_create_object()
{
  // To create an object we only need to extract the information of the class that the
  // user had clicked onto.
  model_common_interface * common_model = dynamic_cast<model_common_interface *> ( model() );
  QModelIndex const & treeindex = currentIndex();
  treenode * cnode = nullptr;

  if ( common_model && treeindex.isValid() )
  {
    dunedaq::conffwk::class_t classinfo = common_model->getclass ( treeindex );

    if ( dynamic_cast<models::treeselection *> ( model() ) )
    {
      models::treeselection * selectionmodel = dynamic_cast<models::treeselection *> ( model() );

      if ( selectionmodel )
      {
        QModelIndex const & srcindex = selectionmodel->mapToSource ( treeindex );

        if ( dbe::models::tree * tree_model = dynamic_cast<dbe::models::tree *>
                                              ( selectionmodel->sourceModel() )
           )
        {
          cnode = tree_model->getnode ( srcindex );
        }
      }
    }
    else if ( dbe::models::subtree_proxy * subtree_model =
                dynamic_cast<dbe::models::subtree_proxy *> ( model() ) )
    {
      cnode = subtree_model->getnode ( treeindex );
    }

    if ( dynamic_cast<RelationshipNode *> ( cnode ) )
    {
      treenode * parent = cnode->GetParent();
      ObjectNode * NodeObject = dynamic_cast<ObjectNode *> ( parent );
      RelationshipNode * RelationshipTreeNode = dynamic_cast<RelationshipNode *> ( cnode );

      ( new ObjectCreator ( NodeObject->GetObject(),
                            RelationshipTreeNode->relation_t() ) )->show();
    }
    else
    {
      ( new ObjectCreator ( classinfo ) )->show();
    }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::slot_delete_objects()
{
  if ( this->model() != nullptr )
  {
    QModelIndexList qindices = this->selectedIndexes();
    std::vector<QModelIndex> indices;

    for ( QModelIndex const & q : qindices )
    {
      if ( q.isValid() and q.column() == 0 )
      {
        indices.push_back ( q );
      }
    }

    if ( models::treeselection * casted_model = dynamic_cast<models::treeselection *> ( this
                                                                                        ->model() )
       )
    {
      casted_model->delete_objects ( indices.begin(), indices.end() );
    }
    else if ( dbe::models::subtree_proxy * casted_model =
                dynamic_cast<dbe::models::subtree_proxy *>
                ( this->model() ) )
    {
      casted_model->delete_objects ( indices.begin(), indices.end() );
    }
    else
    {
      WARN ( "Object Deleting",
             "Object deletion failed due to the internal model being in invalid state",
             "You are advised to restart the application before proceeding any further" );
    }

  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::slot_edit_object()
{
  if ( this->model() != nullptr )
  {
    QModelIndexList indices = this->selectedIndexes();

    for ( QModelIndex const & q : indices )
    {
      edit_object ( q );
    }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::edit_object ( QModelIndex const & index )
{
  model_common_interface * filtermodel = dynamic_cast<model_common_interface *> ( model() );

  if ( !index.isValid() )
  {
    return;
  }

  if ( filtermodel )
  {
    try
    {
      tref obj = filtermodel->getobject ( index );
      bool WidgetFound = false;
      QString ObjectEditorName = QString ( "%1@%2" ).arg ( obj.UID().c_str() ).arg (
                                   obj.class_name().c_str() );

      for ( QWidget * Editor : QApplication::allWidgets() )
      {
        ObjectEditor * Widget = dynamic_cast<ObjectEditor *> ( Editor );

        if ( Widget != nullptr )
        {
          if ( ( Widget->objectName() ).compare ( ObjectEditorName ) == 0 )
          {
            Widget->raise();
            Widget->setVisible ( true );
            WidgetFound = true;
          }
        }
      }

      if ( !WidgetFound )
      {
        ( new ObjectEditor ( obj ) )->show();
      }
    }
    catch ( daq::dbe::cannot_handle_invalid_qmodelindex const & e )
    {
      // nothing to do , maybe the user just clicked the wrong place
    }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::slot_copy_object()
{
  model_common_interface * filtermodel = dynamic_cast<model_common_interface *> ( model() );
  QModelIndex i = currentIndex();

  if ( i.isValid() and filtermodel )
  {
    try
    {
      ( new ObjectCreator ( filtermodel->getobject ( i ) ) )->show();
    }
    catch ( daq::dbe::cannot_handle_invalid_qmodelindex const & e )
    {
      //TODO the index can be invalid in this case
    }
  }
}
//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::referencedbyOnlycomposite()
{
  referencedBy ( true );
}

void dbe::CustomTreeView::referencedByAll()
{
  referencedBy ( false );
}
//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::closeEvent(QCloseEvent * event) {
    // Reset the model when the widget is closed
    // If not done, reference to removed objects may create issues
    models::treeselection* filtermodel = dynamic_cast<models::treeselection*>(model());

    setModel(nullptr);

    if(filtermodel != nullptr) {
        filtermodel->ResetQueryObjects();
        filtermodel->deleteLater();
    }

    event->accept();
}

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::referencedBy ( bool All )
{
  model_common_interface * filtermodel = dynamic_cast<model_common_interface *> ( model() );
  QModelIndex Index = currentIndex();

  if ( !Index.isValid() )
  {
    return;
  }

  if ( filtermodel )
  {

    try
    {
      tref obj = filtermodel->getobject ( Index );
      std::vector<tref> objects;

      if ( not obj.is_null() )
      {
        objects = obj.referenced_by ( "*", All );
      }

      if ( objects.size() > 0 )
      {
        dbe::models::tree * Source = dynamic_cast<dbe::models::tree *> ( filtermodel
                                                                         ->ReturnSourceModel() );
        models::treeselection * Selection = new models::treeselection();
        Selection->SetFilterType ( models::treeselection::ObjectFilterType );
        Selection->SetQueryObjects ( objects );
        Selection->setFilterRegExp ( "Dummy" );
        Selection->setSourceModel ( Source );

        CustomTreeView * tView = new CustomTreeView ( 0 );
        tView->setModel ( Selection );
        tView->setWindowTitle ( QString ( "Objects referencing '%1'" ).arg ( obj.UID().c_str() ) );
        tView->setSortingEnabled ( true );
        tView->setAlternatingRowColors ( true );
        tView->resizeColumnToContents ( 0 );
        tView->resizeColumnToContents ( 1 );
        tView->setAttribute(Qt::WA_DeleteOnClose);

        connect ( tView, SIGNAL ( doubleClicked ( QModelIndex ) ),
                  tView, SLOT ( slot_edit_object () ) );

        tView->show();
      }
      else
      {
        if ( All )
        {
          WARN ( "No references of only composite objects have been found for object", " ",
                 "with UID:", obj.UID(), "of class", obj.class_name() );
        }
        else
        {
          WARN ( "No references of objects have been found for object", " ", "with UID:",
                 obj.UID(), "of class", obj.class_name() );
        }
      }
    }
    catch ( daq::dbe::cannot_handle_invalid_qmodelindex const & e )
    {}
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::CustomTreeView::referencedBy ( bool All, tref obj )
{
  model_common_interface * filtermodel = dynamic_cast<model_common_interface *> ( model() );

  if ( filtermodel )
  {
    std::vector<tref> objects;

    if ( !obj.is_null() )
    {
      objects = obj.referenced_by ( "*", All );
    }

    if ( objects.size() > 0 )
    {
      dbe::models::tree * Source = dynamic_cast<dbe::models::tree *> ( filtermodel
                                                                       ->ReturnSourceModel() );
      models::treeselection * Selection = new models::treeselection();
      Selection->SetFilterType ( models::treeselection::ObjectFilterType );
      Selection->SetQueryObjects ( objects );
      Selection->setFilterRegExp ( "Dummy" );
      Selection->setSourceModel ( Source );

      CustomTreeView * tView = new CustomTreeView ( 0 );
      tView->setModel ( Selection );
      tView->setSortingEnabled ( true );
      tView->setAlternatingRowColors ( true );
      tView->resizeColumnToContents ( 0 );
      tView->hideColumn ( 1 );
      //tView->resizeColumnToContents(1);
      tView->setWindowTitle ( QString ( "Objects referencing '%1'" ).arg ( obj.UID().c_str() ) );
      tView->setAttribute(Qt::WA_DeleteOnClose);

      connect ( tView, SIGNAL ( doubleClicked ( QModelIndex ) ), tView,
                SLOT ( slot_edit_object () ) );

      tView->show();
    }
    else
    {
      if ( All )
      {
        WARN ( "No references for object", "Only Composite Objects - Not found", "with UID:",
               obj.UID() );
      }
      else
      {
        WARN ( "No references for object", "All Objects - Not found", "with UID:", obj.UID() );
      }
    }
  }
}
//------------------------------------------------------------------------------------------
