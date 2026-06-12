/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

#include "dbe/config_ui_info.hpp"
#include "dbe/ObjectCreator.hpp"
#include "dbe/GraphView.hpp"
#include "dbe/MainWindow.hpp"
#include "dbe/config_api_info.hpp"
#include "dbe/config_api_commands.hpp"
#include "dbe/treenode.hpp"

#include <QLabel>
#include <QDialog>
#include <QComboBox>
#include <QMenu>
#include <QContextMenuEvent>


//------------------------------------------------------------------------------------------
dbe::GraphView::~GraphView() = default;

dbe::GraphView::GraphView ( QWidget * parent )
  : QWidget ( parent ),
    ContextMenu ( nullptr ),
    ClickedItem ( nullptr ),
    uuid ( QUuid::createUuid() )
{
  setupUi ( this );
  /// Connecting Actions
  ConnectActions();
  /// Initial Settings
  setWindowTitle ( "Graphical View" );
  /// Scene setting
  QGraphicsScene * scene = new QGraphicsScene ( this );
  scene->setItemIndexMethod ( QGraphicsScene::NoIndex );
  scene->setSceneRect ( 0, 0, 10000, 10000 );
  GraphicalView->setScene ( scene );
  GraphicalView->setCacheMode ( QGraphicsView::CacheBackground );
  GraphicalView->setViewportUpdateMode ( QGraphicsView::BoundingRectViewportUpdate );
  GraphicalView->setRenderHint ( QPainter::Antialiasing );
  GraphicalView->setTransformationAnchor ( QGraphicsView::AnchorUnderMouse );
  GraphicalView->setMinimumSize ( 400, 400 );
  GraphicalView->centerOn ( scene->sceneRect().topLeft() );
  GraphicalView->setAcceptDrops ( true );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::ConnectActions()
{
  connect ( LoadButton, SIGNAL ( clicked() ), this, SLOT ( GetWindowConfiguration() ),
            Qt::UniqueConnection );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::SetupView()
{
  /// Cleaning the scene
  GraphicalView->scene()->clear();

  /// WindowSettings
  double dy = 0;
  setWindowTitle ( WindowConfiguration.Title + " View" );

  /// Iterating over the classes that have to be displayed
  for ( QString & ClassName : WindowConfiguration.GraphicalClassesList )
  {
    GraphicalClass GraphicalClassInfo = confaccessor::guiconfig()->graphical (
                                          ClassName.toStdString() );
    /// Getting Class and SubClasses to initialize
    // QStringList ClassesToInitialize = GraphicalClassInfo.DerivedClasses;
    // ClassesToInitialize.append(GraphicalClassInfo.DatabaseClassName);

    // for(QString& InitializeClass : ClassesToInitialize)
    // {
    double dx = 0;
    double ChildNodeHeight = 0;

    // Node* ClassNode = ConfigWrapper::GetInstance().GetDataHandler()->GetClassNode(InitializeClass);
    treenode * ClassNode = confaccessor::gethandler()->getnode (
                             GraphicalClassInfo.DatabaseClassName );

    if ( ClassNode )
    {
      /// Loadind Data
      confaccessor::gethandler()->FetchMore ( ClassNode );

      for ( treenode * Object : ClassNode->GetChildren() )
      {
        bool Used = true;
        GraphicalObject * ObjectNodeGraphical = new GraphicalObject (
          Used, Object->GetData ( 0 ).toString(), GraphicalClassInfo );
        GraphicalView->scene()->addItem ( ObjectNodeGraphical );
        ObjectNodeGraphical->setPos (
          GraphicalView->scene()->sceneRect().topLeft() + QPointF ( dx, dy ) );
        dx += ObjectNodeGraphical->boundingRect().width() + 30;

        if ( ( dx + ObjectNodeGraphical->boundingRect().width() ) > GraphicalView->scene()
             ->sceneRect().width() )
        {
          dx = 0;
          dy += ChildNodeHeight + 30;
        }

        ChildNodeHeight = ObjectNodeGraphical->boundingRect().height();
      }

      dy += ChildNodeHeight + 30;
    }

    // }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::contextMenuEvent ( QContextMenuEvent * Event )
{
  if ( ContextMenu == nullptr )
  {
    ContextMenu = new QMenu ( GraphicalView );
    /// CreaContextMenu(nullptr)ting Actions
    CreateActions();
  }

  QGraphicsItem * Index = GraphicalView->itemAt ( Event->pos() );

  if ( Index )
  {
    ClickedItem = dynamic_cast<GraphicalObject *> ( Index );

    if ( ClickedItem )
    {
      ContextMenu->exec ( Event->globalPos() );
    }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::CreateActions()
{
  editObject = new QAction ( tr ( "&Edit Object" ), this );
  editObject->setShortcut ( tr ( "Ctrl+E" ) );
  editObject->setShortcutContext ( Qt::WidgetShortcut );
  connect ( editObject, SIGNAL ( triggered() ), this, SLOT ( editThisObject() ),
            Qt::UniqueConnection );
  ContextMenu->addAction ( editObject );

  deleteObjectAc = new QAction ( tr ( "&Delete Object" ), this );
  deleteObjectAc->setShortcut ( tr ( "Ctrl+D" ) );
  deleteObjectAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( deleteObjectAc, SIGNAL ( triggered() ), this, SLOT ( deleteThisObject() ),
            Qt::UniqueConnection );
  ContextMenu->addAction ( deleteObjectAc );

  refByAc = new QAction ( tr ( "Referenced B&y (All objects)" ), this );
  refByAc->setShortcut ( tr ( "Ctrl+Y" ) );
  refByAc->setShortcutContext ( Qt::WidgetShortcut );
  refByAc->setToolTip ( "Find all objects which reference the selecetd object" );
  refByAc->setStatusTip ( refByAc->toolTip() );
  connect ( refByAc, SIGNAL ( triggered() ), this, SLOT ( referencedBy_All() ),
            Qt::UniqueConnection );
  ContextMenu->addAction ( refByAc );

  refByAcOnlyComp = new QAction ( tr ( "Referenced B&y (Only Composite)" ), this );
  refByAcOnlyComp->setShortcut ( tr ( "Ctrl+Shift+Y" ) );
  refByAcOnlyComp->setShortcutContext ( Qt::WidgetShortcut );
  refByAcOnlyComp->setToolTip (
    "Find objects (ONLY Composite ones) which reference the selecetd object" );
  refByAcOnlyComp->setStatusTip ( refByAcOnlyComp->toolTip() );
  connect ( refByAcOnlyComp, SIGNAL ( triggered() ), this,
            SLOT ( referencedBy_OnlyComposite() ),
            Qt::UniqueConnection );
  ContextMenu->addAction ( refByAcOnlyComp );

  copyObjectAc = new QAction ( tr ( "Copy This Object Into A &New One" ), this );
  copyObjectAc->setShortcut ( tr ( "Ctrl+Shift+N" ) );
  copyObjectAc->setShortcutContext ( Qt::WidgetShortcut );
  connect ( copyObjectAc, SIGNAL ( triggered() ), this, SLOT ( copyObject() ),
            Qt::UniqueConnection );
  ContextMenu->addAction ( copyObjectAc );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::editThisObject()
{
  tref Object = inner::dbcontroller::get (
  {
    ClickedItem->GetDatabaseUidName().toStdString(), ClickedItem->GetDatabaseClassName()
    .toStdString()
  } );

  bool WidgetFound = false;
  QString ObjectEditorName = QString ( "%1@%2" ).arg ( Object.UID().c_str() ).arg (
                               Object.class_name().c_str() );

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
    ( new ObjectEditor ( Object ) )->show();
  }
}

void dbe::GraphView::deleteThisObject()
{

  try
  {
    tref obj = inner::dbcontroller::get (
    {
      ClickedItem->GetDatabaseUidName().toStdString(),
      ClickedItem->GetDatabaseClassName().toStdString()
    } );

    dbe::config::api::commands::delobj ( obj, uuid );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    // Nothing needed to do here , if the lookup failed that is ok
  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::copyObject()
{

  try
  {
    tref obj = inner::dbcontroller::get (
    {
      ClickedItem->GetDatabaseUidName().toStdString(),
      ClickedItem->GetDatabaseClassName().toStdString()
    } );

    ( new ObjectCreator ( obj ) )->show();
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
  {

  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::referencedBy_All()
{

  try
  {
    tref obj = inner::dbcontroller::get (
    {
      ClickedItem->GetDatabaseUidName().toStdString(),
      ClickedItem->GetDatabaseClassName().toStdString()
    } );

    MainWindow::findthis()->get_view()->referencedBy ( false, obj );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    // Nothing needed to do here , it seems that the object has not been found ,
  }

}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::referencedBy_OnlyComposite()
{
  try
  {
    tref obj = inner::dbcontroller::get (
    {
      ClickedItem->GetDatabaseUidName().toStdString(),
      ClickedItem->GetDatabaseClassName().toStdString()
    } );

    MainWindow::findthis()->get_view()->referencedBy ( true, obj );
  }
  catch ( daq::dbe::config_object_retrieval_result_is_null const & ex )
  {
    // Nothing needed to do here , it seems that the object has not been found ,
    // which is weird in this case
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::GetWindowConfiguration()
{
  QString ChoosenWindow;

  QDialog * ChooseDialog = new QDialog ( this );
  QComboBox * Combo = new QComboBox ( ChooseDialog );
  QPushButton * GoButton = new QPushButton ( "Choose" );
  QHBoxLayout * Layout = new QHBoxLayout ( ChooseDialog );
  QLabel * Label = new QLabel ( "Window Configuration : ", ChooseDialog );

  connect ( GoButton, SIGNAL ( clicked() ), ChooseDialog, SLOT ( accept() ),
            Qt::UniqueConnection );

  std::vector<Window> Configurations = confaccessor::guiconfig()->windows();

  for ( Window const & WindowConfig : Configurations )
  {
    Combo->addItem ( WindowConfig.Title );
  }

  Layout->addWidget ( Label );
  Layout->addWidget ( Combo );
  Layout->addWidget ( GoButton );

  ChooseDialog->setLayout ( Layout );
  ChooseDialog->setWindowTitle ( "Choose window configuration" );

  if ( ChooseDialog->exec() )
  {
    ChoosenWindow = Combo->currentText();
  }

  for ( Window & Setting : Configurations )
  {
    if ( ChoosenWindow == Setting.Title )
    {
      WindowConfiguration = Setting;
      SetupView();
      break;
    }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::RedrawObject ( tref Object )
{
  Q_UNUSED ( Object )
  SetupView();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphView::RedrawObject()
{
  SetupView();
}
//------------------------------------------------------------------------------------------
