/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

#include "dbe/config_ui_info.hpp"
#include "dbe/config_api_info.hpp"
#include "dbe/config_api_commands.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/treenode.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QBitmap>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QMimeData>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>
#include <QLabel>
#include <QDrag>

//------------------------------------------------------------------------------------------
dbe::GraphicalObject::GraphicalObject ( bool Used, QString ObjectName,
                                        GraphicalClass GraphInfo, QGraphicsObject * parent )
  : QGraphicsObject ( parent ),
    GraphicalInfo ( GraphInfo ),
    IconItem ( nullptr ),
    TextItem ( nullptr ),
    DatabaseClassName ( GraphInfo.DatabaseClassName ),
    DatabaseUidName ( ObjectName ),
    IsExpanded ( false ),
    IsFetched ( false ),
    expandedX ( 0 ),
    expandedY ( 0 )
{
  setAcceptDrops ( true );
  setFlag ( ItemIsMovable );

  QPixmap Icon;

  if ( Used )
  {
    if ( !GraphInfo.UsedPixmapFile.isEmpty() ) Icon = QPixmap (
                                                          ":/Images/" + GraphInfo.UsedPixmapFile );
    else
    {
      Icon = QPixmap ( ":/Images/" + GraphInfo.GenericPixmapFile );
    }
  }
  else
  {
    Icon = QPixmap ( ":/Images/" + GraphInfo.GenericPixmapFile );
  }

  QBitmap IconMask = Icon.createHeuristicMask();
  Icon.setMask ( IconMask );

  IconItem = new QGraphicsPixmapItem ( Icon );
  TextItem = new QGraphicsTextItem ( ObjectName );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::GraphicalObject::~GraphicalObject()
{
  delete IconItem;
  delete TextItem;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QRectF dbe::GraphicalObject::boundingRect() const
{
  QRectF Boundarie;
  Boundarie.setHeight (
    IconItem->boundingRect().height() + TextItem->boundingRect().height() );

  if ( IconItem->boundingRect().width() > TextItem->boundingRect().width() ) Boundarie
    .setWidth ( IconItem->boundingRect().width() );
  else
  {
    Boundarie.setWidth ( TextItem->boundingRect().width() );
  }

  return Boundarie;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::paint ( QPainter * painter,
                                   const QStyleOptionGraphicsItem * option,
                                   QWidget * widget )
{
  Q_UNUSED ( option )
  Q_UNUSED ( widget )

  QFontMetrics Metric ( QFont ( "Helvetica", 10 ) );
  painter->setFont ( QFont ( "Helvetica", 10 ) );
  painter->drawPixmap ( IconItem->boundingRect(), IconItem->pixmap(),
                        IconItem->boundingRect() );
  painter->drawText (
    IconItem->boundingRect().bottomLeft() + QPointF (
      0, Metric.boundingRect ( DatabaseUidName ).height() ),
    DatabaseUidName );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QPainterPath dbe::GraphicalObject::shape() const
{
  QPainterPath Path;
  Path.addRect ( IconItem->boundingRect() );
  return Path;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
  if ( !IsFetched )
  {
    AddGraphicChildren();
  }

  if ( !IsExpanded )
  {
    ShowGraphicsChildren();
  }
  else
  {
    HideGraphicsChildren();
  }

  update();
  QGraphicsItem::mouseDoubleClickEvent ( event );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
  if ( event->button() == Qt::LeftButton )
  {
    StartDrag = event->pos();
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
  if ( ! ( event->buttons() & Qt::LeftButton ) )
  {
    return;
  }

  if ( ( event->pos() - StartDrag ).manhattanLength() < QApplication::startDragDistance() )
  {
    return;
  }

  QDrag * drag = new QDrag ( event->widget() );
  QPixmap Icon = IconItem->pixmap();

  QStringList DataList;
  QMimeData * mimeData = new QMimeData;
  QByteArray ItemData;
  QDataStream DataStream ( &ItemData, QIODevice::WriteOnly );

  DataList.append ( QString ( DatabaseUidName ) );
  DataList.append ( QString ( DatabaseClassName ) );
  DataStream << DataList;
  mimeData->setData ( "application/vnd.text.list", ItemData );

  drag->setMimeData ( mimeData );
  drag->setPixmap ( Icon );

  Qt::DropAction dropAction = drag->exec();
  Q_UNUSED ( dropAction )
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::dropEvent ( QGraphicsSceneDragDropEvent * event )
{
  QByteArray encodedData = event->mimeData()->data ( "application/vnd.text.list" );
  QDataStream stream ( &encodedData, QIODevice::ReadOnly );
  QList<QStringList> NewItems;

  while ( !stream.atEnd() )
  {
    QStringList Text;
    stream >> Text;
    NewItems << Text;
  }

  QStringList ObjectList;

  for ( QStringList & i : NewItems )
  {
    ObjectList.append ( i.at ( 0 ) );
  }

  for ( int i = 0; i < NewItems.size(); ++i )
  {
    if ( NewItems.at ( 0 ).at ( 1 ) != NewItems.at ( i ).at ( 1 ) )
    {
      return;
    }
  }

  QStringList PossibleRelationships;
  QString ClassName = NewItems.at ( 0 ).at ( 1 );

  dunedaq::conffwk::class_t ClassInfoReceiverObject =
    dbe::config::api::info::onclass::definition ( DatabaseClassName.toStdString(), false );
  std::vector<dunedaq::conffwk::relationship_t> RelationshipList = ClassInfoReceiverObject
                                                              .p_relationships;

  for ( dunedaq::conffwk::relationship_t & i : RelationshipList )
  {
    if ( i.p_type == ClassName.toStdString() ) PossibleRelationships.append (
        QString::fromStdString ( i.p_name ) );

    dunedaq::conffwk::class_t ClassInfoDroppedObject =
      dbe::config::api::info::onclass::definition ( i.p_type,
                                                    false );
    std::vector<std::string> RelationshipListDropped = ClassInfoDroppedObject.p_subclasses;

    for ( std::string & j : RelationshipListDropped )
    {
      if ( j == ClassName.toStdString() ) PossibleRelationships.append (
          QString::fromStdString ( i.p_name ) );
    }
  }

  QString SelectedRelationship;

  if ( PossibleRelationships.size() == 0 )
  {
    return;
  }
  else if ( PossibleRelationships.size() == 1 ) SelectedRelationship = PossibleRelationships
                                                                         .at ( 0 );
  else
  {
    QDialog * NewDialog = new QDialog();
    QHBoxLayout * NewLayout = new QHBoxLayout();
    QPushButton * OkButton = new QPushButton ( "Ok" );
    QComboBox * NewCombo = new QComboBox();
    QLabel * NewLabel = new QLabel ( "Choose relationship : " );
    NewCombo->addItems ( PossibleRelationships );
    NewLayout->addWidget ( NewLabel );
    NewLayout->addWidget ( NewCombo );
    NewLayout->addWidget ( OkButton );
    NewDialog->setLayout ( NewLayout );
    NewDialog->adjustSize();
    connect ( OkButton, SIGNAL ( clicked() ), NewDialog, SLOT ( accept() ),
              Qt::UniqueConnection );

    int Status = NewDialog->exec();

    if ( Status == QDialog::Accepted )
    {
      SelectedRelationship = NewCombo->currentText();
    }
  }

  treenode * NodeObject = confaccessor::gethandler()->getnode ( DatabaseClassName,
                                                                DatabaseUidName );

  if ( NodeObject )
  {
    for ( treenode * Child : NodeObject->GetChildren() )
    {
      if ( dynamic_cast<RelationshipNode *> ( Child ) )
      {
        RelationshipNode * NodeRelationship = dynamic_cast<RelationshipNode *> ( Child );
        dunedaq::conffwk::relationship_t RelationshipData =
          NodeRelationship->relation_t();

        if ( RelationshipData.p_name == SelectedRelationship.toStdString() )
        {
          if ( ( RelationshipData.p_cardinality == dunedaq::conffwk::zero_or_many ) || ( RelationshipData
                                                                                    .p_cardinality
                                                                                    == dunedaq::conffwk::one_or_many ) )
          {
            std::vector<std::string> RelationshipValues;

            for ( treenode * RelationshipChildren : NodeRelationship->GetChildren() )
              RelationshipValues.push_back (
                RelationshipChildren->GetData ( 0 ).toString().toStdString() );

            for ( QStringList & Item : NewItems )
              if ( std::find ( RelationshipValues.begin(), RelationshipValues.end(),
                               Item.at ( 0 ).toStdString() )
                   == RelationshipValues.end() ) RelationshipValues.push_back (
                       Item.at ( 0 ).toStdString() );

            tref Object = NodeObject->GetObject();
            dbe::config::api::commands::modobj ( Object, RelationshipData,
                                                 RelationshipValues );
          }
          else
          {
            QString SelectedObject;

            if ( NewItems.size() > 1 )
            {
              QDialog * NewDialog = new QDialog();
              QHBoxLayout * NewLayout = new QHBoxLayout();
              QPushButton * OkButton = new QPushButton ( "Ok" );
              QComboBox * NewCombo = new QComboBox();
              QLabel * NewLabel = new QLabel ( "Choose Object : " );
              NewCombo->addItems ( ObjectList );
              NewLayout->addWidget ( NewLabel );
              NewLayout->addWidget ( NewCombo );
              NewLayout->addWidget ( OkButton );
              NewDialog->setLayout ( NewLayout );
              NewDialog->adjustSize();
              connect ( OkButton, SIGNAL ( clicked() ), NewDialog, SLOT ( accept() ),
                        Qt::UniqueConnection );

              int Status = NewDialog->exec();

              if ( Status == QDialog::Accepted )
              {
                SelectedObject = NewCombo->currentText();
              }
            }
            else
            {
              SelectedObject = NewItems.at ( 0 ).at ( 0 );
            }

            tref Object = NodeObject->GetObject();
            dbe::config::api::commands::modobj ( Object, RelationshipData,
            { SelectedObject.toStdString() } );

          }
        }
      }
    }
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::AddGraphicChildren()
{
  double dx = 0;
  double dy = 0;

  treenode * NodeObject = confaccessor::gethandler()->getnode ( DatabaseClassName,
                                                                DatabaseUidName );

  if ( NodeObject )
  {
    for ( treenode * ChildNode : NodeObject->GetChildren() )
    {
      RelationshipNode * NodeRelationship = dynamic_cast<RelationshipNode *> ( ChildNode );

      if ( NodeRelationship && NodeRelationship->GetHasStructure() )
      {
        dunedaq::conffwk::relationship_t RelationshipData =
          NodeRelationship->relation_t();
        GraphicalRelationship * RelationshipChildNode = new GraphicalRelationship (
          DatabaseUidName, DatabaseClassName, RelationshipData );
        RelationshipChildNode->setParentItem ( this );
        RelationshipChildNode->AddGraphicChildren();

        QPointF coord = RelationshipChildNode->mapFromParent ( boundingRect().bottomRight() );
        RelationshipChildNode->setX ( coord.x() + dx );
        RelationshipChildNode->setY ( coord.y() + dy );

        for ( QGraphicsItem * GraphicItem : RelationshipChildNode->childItems() )
        {
          dy += GraphicItem->boundingRect().height();
        }
      }
    }
  }

  IsFetched = true;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::ShowGraphicsChildren()
{
  for ( QGraphicsItem * i : this->childItems() )
  {
    i->show();
  }

  double ThisOffsetX = 0;
  double ThisOffsetY = 0;
  GetOffsetX ( this, ThisOffsetX );
  GetOffsetY ( this, ThisOffsetY );
  QPointF ThisItemBottom = this->mapToScene ( this->boundingRect().bottomRight() );

  if ( !this->childItems().isEmpty() )
  {
    ThisOffsetX -= ThisItemBottom.x();
    ThisOffsetY -= ThisItemBottom.y();
  }

/// First change items with the same parent
  QGraphicsItem * AuxiliaryItem = this;

  while ( AuxiliaryItem->parentItem() != nullptr )
  {
    for ( int i = AuxiliaryItem->parentItem()->childItems().indexOf ( AuxiliaryItem ) + 1;
          i < AuxiliaryItem->parentItem()->childItems().size(); ++i )
    {
      QGraphicsItem * Child = AuxiliaryItem->parentItem()->childItems().at ( i );
      Child->setY ( Child->y() + ThisOffsetY );
    }

    AuxiliaryItem = AuxiliaryItem->parentItem();
  }

/// Now that the items with the same parent are in their final position
/// calculate the OffsetX/OffsetY of this item top level parent item
  double ThisTopLevelOffsetX = 0;
  double ThisTopLevelOffsetY = 0;
  GetOffsetX ( this->topLevelItem(), ThisTopLevelOffsetX );
  GetOffsetY ( this->topLevelItem(), ThisTopLevelOffsetY );
  QPointF ThisTopLevelItemBottom = this->topLevelItem()->mapToScene (
                                     this->topLevelItem()->boundingRect().bottomRight() );

  if ( !this->topLevelItem()->childItems().isEmpty() )
  {
    ThisTopLevelOffsetX -= ThisTopLevelItemBottom.x();
    ThisTopLevelOffsetY -= ThisTopLevelItemBottom.y();
  }

/// Now get all the items in the scene
  QList<QGraphicsItem *> TopLevelItems = scene()->items();
/// Now get the highest y in the same line as this top level item
  GraphicalObject * ThisTopLevelItem = dynamic_cast<GraphicalObject *>
                                       ( this->topLevelItem() );
  double LineHighestOffsetY = 0;

  for ( QGraphicsItem * Item : TopLevelItems )
  {
    /// Get only top levels items at the same line as this top level item
    /// and not equal as this top level item
    if ( Item->parentItem() == nullptr && ThisTopLevelItem->y() == Item->y()
         && ThisTopLevelItem != Item )
    {
      GraphicalObject * ItemObject = dynamic_cast<GraphicalObject *> ( Item );

      if ( ItemObject && LineHighestOffsetY < ItemObject->GetExpandedY() ) LineHighestOffsetY =
          ItemObject->GetExpandedY();
    }
  }

/// Now i have the highest offset on the line, distance from the line bottom line
/// Iterating over the top level items to add proper offsetX/offsetY
  for ( QGraphicsItem * Item : TopLevelItems )
  {
    if ( ThisTopLevelItem != Item && Item->parentItem() == nullptr )
    {
      if ( ThisTopLevelItem->y() < Item->y() && ThisTopLevelOffsetY > LineHighestOffsetY )
      {
        if ( LineHighestOffsetY == 0 )
        {
          Item->setY ( Item->y() + ThisOffsetY );
        }
        else
        {
          if ( ThisTopLevelOffsetY - LineHighestOffsetY < ThisTopLevelOffsetY
               - ThisTopLevelItem->GetExpandedY() ) Item->setY (
                   Item->y() + ThisTopLevelOffsetY - LineHighestOffsetY );
          else Item->setY (
              Item->y() + ThisTopLevelOffsetY - ThisTopLevelItem->GetExpandedY() );
        }
      }
    }

    if ( ThisTopLevelItem->y() == Item->y() && ThisTopLevelItem->x() < Item->x()
         && Item->parentItem() == nullptr )
    {
      if ( ThisTopLevelOffsetX > ThisTopLevelItem->GetExpandedX() )
      {
        if ( ThisTopLevelItem->GetExpandedX() == 0 )
        {
          Item->setX ( Item->x() + ThisOffsetX );
        }
        else
        {
          Item->setX ( Item->x() + ThisTopLevelOffsetX - ThisTopLevelItem->GetExpandedX() );
        }
      }
    }
  }

  ThisTopLevelItem->SetExpandedX ( ThisTopLevelOffsetX );
  ThisTopLevelItem->SetExpandedY ( ThisTopLevelOffsetY );
  IsExpanded = true;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::HideGraphicsChildren()
{
  double ThisOffsetX = 0;
  double ThisOffsetY = 0;
  GetOffsetX ( this, ThisOffsetX );
  GetOffsetY ( this, ThisOffsetY );
  QPointF ThisItemBottom = this->mapToScene ( this->boundingRect().bottomRight() );

  if ( !this->childItems().isEmpty() )
  {
    ThisOffsetX -= ThisItemBottom.x();
    ThisOffsetY -= ThisItemBottom.y();
  }

/// First change items with the same parent
  QGraphicsItem * AuxiliaryItem = this;

  while ( AuxiliaryItem->parentItem() != nullptr )
  {
    for ( int i = AuxiliaryItem->parentItem()->childItems().indexOf ( AuxiliaryItem ) + 1;
          i < AuxiliaryItem->parentItem()->childItems().size(); ++i )
    {
      QGraphicsItem * Child = AuxiliaryItem->parentItem()->childItems().at ( i );
      Child->setY ( Child->y() - ThisOffsetY );
    }

    AuxiliaryItem = AuxiliaryItem->parentItem();
  }

/// Now that the items with the same parent are in their final position
/// calculate the OffsetX/OffsetY of this item top level parent item
/// THIS FOR IS EXTREMELY IMPORTANT -> DO NOT TOUCH IT
/// IF YOU TOUCH IT I WILL HUNT YOU FOREVER....AND ONE DAY I WILL CATCH YOU
  for ( QGraphicsItem * i : this->childItems() )
  {
    i->hide();
  }

  double ThisTopLevelOffsetX = 0;
  double ThisTopLevelOffsetY = 0;
  GetOffsetX ( this->topLevelItem(), ThisTopLevelOffsetX );
  GetOffsetY ( this->topLevelItem(), ThisTopLevelOffsetY );
  QPointF ThisTopLevelItemBottom = this->topLevelItem()->mapToScene (
                                     this->topLevelItem()->boundingRect().bottomRight() );

  if ( !this->topLevelItem()->childItems().isEmpty()
       && this->topLevelItem()->childItems().at (
         0 )->isVisible() )
  {
    ThisTopLevelOffsetX -= ThisTopLevelItemBottom.x();
    ThisTopLevelOffsetY -= ThisTopLevelItemBottom.y();
  }

/// Now get all the items in the scene
  QList<QGraphicsItem *> TopLevelItems = scene()->items();
/// Now get the highest y in the same line as this top level item
  GraphicalObject * ThisTopLevelItem = dynamic_cast<GraphicalObject *>
                                       ( this->topLevelItem() );
  double LineHighestOffsetY = 0;

  for ( QGraphicsItem * Item : TopLevelItems )
  {
    /// Get only top levels items at the same line as this top level item
    /// and not equal as this top level item
    if ( Item->parentItem() == nullptr && ThisTopLevelItem->y() == Item->y()
         && ThisTopLevelItem != Item )
    {
      GraphicalObject * ItemObject = dynamic_cast<GraphicalObject *> ( Item );

      if ( ItemObject && LineHighestOffsetY < ItemObject->GetExpandedY() ) LineHighestOffsetY =
          ItemObject->GetExpandedY();
    }
  }

/// Now i have the highest offset on the line, distance from the line bottom line
/// Iterating over the top level items to add proper offsetX/offsetY
  for ( QGraphicsItem * Item : TopLevelItems )
  {
    if ( ThisTopLevelItem != Item && Item->parentItem() == nullptr )
    {
      if ( ThisTopLevelItem->y() < Item->y() && ThisTopLevelItem->GetExpandedY()
           > LineHighestOffsetY )
      {
        if ( LineHighestOffsetY == 0 )
        {
          Item->setY ( Item->y() - ThisOffsetY );
        }
        else
        {
          if ( ThisTopLevelOffsetY >= LineHighestOffsetY && ThisTopLevelOffsetY
               < ThisTopLevelItem->GetExpandedY() ) Item->setY (
                   Item->y() - abs ( ThisTopLevelOffsetY - ThisTopLevelItem->GetExpandedY() ) );

          if ( ThisTopLevelItem->GetExpandedY() > LineHighestOffsetY && ThisTopLevelOffsetY
               < LineHighestOffsetY ) Item->setY (
                   Item->y() - abs ( ThisTopLevelItem->GetExpandedY() - LineHighestOffsetY ) );
        }
      }
    }

    if ( ThisTopLevelItem->y() == Item->y() && ThisTopLevelItem->x() < Item->x()
         && Item->parentItem() == nullptr )
    {
      if ( ThisTopLevelOffsetX < ThisTopLevelItem->GetExpandedX() )
      {
        Item->setX ( Item->x() - abs ( ThisTopLevelOffsetX - ThisTopLevelItem->GetExpandedX() ) );
      }
    }
  }

  ThisTopLevelItem->SetExpandedX ( ThisTopLevelOffsetX );
  ThisTopLevelItem->SetExpandedY ( ThisTopLevelOffsetY );
  IsExpanded = false;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::GetOffsetX ( QGraphicsItem * Item, double & Offset )
{
  for ( QGraphicsItem * ChildItem : Item->childItems() )
  {
    if ( ChildItem->isVisible() )
    {
      QPointF ChildItemBottom = ChildItem->mapToScene (
                                  ChildItem->boundingRect().bottomRight() );

      if ( Offset < ChildItemBottom.x() )
      {
        Offset = ChildItemBottom.x();
      }
    }

    GetOffsetX ( ChildItem, Offset );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::GetOffsetY ( QGraphicsItem * Item, double & Offset )
{
  for ( QGraphicsItem * ChildItem : Item->childItems() )
  {
    if ( ChildItem->isVisible() )
    {
      QPointF ChildItemBottom = ChildItem->mapToScene (
                                  ChildItem->boundingRect().bottomRight() );

      if ( Offset < ChildItemBottom.y() )
      {
        Offset = ChildItemBottom.y();
      }
    }

    GetOffsetY ( ChildItem, Offset );
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::LocateTopMostItem ( QGraphicsItem * Auxiliary )
{
  while ( Auxiliary->parentItem() != nullptr )
  {
    Auxiliary = Auxiliary->parentItem();
  }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QString dbe::GraphicalObject::GetDatabaseClassName() const
{
  return DatabaseClassName;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QString dbe::GraphicalObject::GetDatabaseUidName() const
{
  return DatabaseUidName;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
double dbe::GraphicalObject::GetExpandedX() const
{
  return expandedX;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
double dbe::GraphicalObject::GetExpandedY() const
{
  return expandedY;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::SetExpandedX ( double dx )
{
  expandedX = dx;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalObject::SetExpandedY ( double dy )
{
  expandedY = dy;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::GraphicalRelationship::GraphicalRelationship ( QString ObjectName, QString ClassName,
                                                    dunedaq::conffwk::relationship_t & Data,
                                                    QGraphicsObject * parent )
  : QGraphicsObject ( parent ),
    DatabaseClassName ( ClassName ),
    DatabaseUidName ( ObjectName ),
    RelationshipData ( Data ),
    TextItem ( nullptr )
{
  TextItem = new QGraphicsTextItem ( QString ( Data.p_name.c_str() ) );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::GraphicalRelationship::~GraphicalRelationship()
{
  delete TextItem;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QRectF dbe::GraphicalRelationship::boundingRect() const
{
  return TextItem->boundingRect();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalRelationship::paint ( QPainter * painter,
                                         const QStyleOptionGraphicsItem * option,
                                         QWidget * widget )
{
  painter->setFont ( QFont ( "Helvetica", 10 ) );
  TextItem->paint ( painter, option, widget );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
QPainterPath dbe::GraphicalRelationship::shape() const
{
  QPainterPath path;
  path.addRect ( TextItem->boundingRect() );
  return path;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalRelationship::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
{
  Q_UNUSED ( event )
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalRelationship::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
  Q_UNUSED ( event )
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalRelationship::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
  Q_UNUSED ( event )
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::GraphicalRelationship::AddGraphicChildren()
{
  double OffsetY = 0;
  double OffsetX = 10;

  treenode * NodeObject = confaccessor::gethandler()->getnode ( DatabaseClassName,
                                                                DatabaseUidName );

  if ( NodeObject )
  {
    for ( treenode * ChildNode : NodeObject->GetChildren() )
    {
      if ( ChildNode->GetData ( 0 ).toString() == QString ( RelationshipData.p_name.c_str() ) )
      {
        for ( treenode * ChildChildNode : ChildNode->GetChildren() )
        {
          ObjectNode * ChildChildNodeObject = dynamic_cast<ObjectNode *> ( ChildChildNode );

          if ( ChildChildNodeObject )
          {
            std::string cname = ChildChildNodeObject->GetObject().class_name();
            GraphicalClass Dummy = confaccessor::guiconfig()->graphical ( cname );
            GraphicalObject * ObjectNodeGraphical = new GraphicalObject (
              true, ChildChildNode->GetData ( 0 ).toString(), Dummy );
            ObjectNodeGraphical->setParentItem ( this );

            QPointF coord = ObjectNodeGraphical->mapFromParent ( boundingRect().topRight() );
            ObjectNodeGraphical->setX ( coord.x() + OffsetX );
            ObjectNodeGraphical->setY ( coord.y() + OffsetY );
            OffsetY += ObjectNodeGraphical->boundingRect().height();
          }
        }

        break;
      }
    }
  }
}
//------------------------------------------------------------------------------------------
