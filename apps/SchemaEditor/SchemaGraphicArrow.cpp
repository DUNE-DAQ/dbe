/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from src/SchemaEditor/SchemaGraphicArrow.cpp to apps/SchemaEditor/SchemaGraphicArrow.cpp).
 */

/// Including QT Headers
#include <QPainter>
#include <QPen>
/// Including Schema Editor
#include "dbe/SchemaGraphicArrow.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
/// Including C++ Headers
#include <cmath>

dbse::SchemaGraphicArrow::SchemaGraphicArrow ( SchemaGraphicObject * StartItem,
                                               SchemaGraphicObject * EndItem, bool IsInheritance,
                                               bool IsComposite, QString ArrowName,
                                               QString ArrowCardinality, QGraphicsItem * parent )
  : QGraphicsLineItem ( parent ),
    Start ( StartItem ),
    End ( EndItem ),
    Inheritance ( IsInheritance ),
    Composite ( IsComposite ),
    Name ( ArrowName ),
    Cardinality ( ArrowCardinality ),
    Label ( nullptr ),
    LastDegree ( 0 ),
    LastRotation ( 0 )
{
  //setPen(QPen(Qt::black,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
  setFlag ( ItemIsSelectable, true );
  //LabelString = Name + " - " + Cardinality;

  if ( !Start->collidesWithItem ( End ) && !Inheritance )
  {
    Label = new QGraphicsSimpleTextItem ( Name + " - " + Cardinality );
    Label->setParentItem ( this );
  }

}

dbse::SchemaGraphicArrow::~SchemaGraphicArrow()
{
}

QRectF dbse::SchemaGraphicArrow::boundingRect() const
{
  qreal extra = ( pen().width() + 20 ) / 2.0 + 10;
  return QRectF ( line().p1(),
                  QSizeF ( line().p2().x() - line().p1().x(),
                           line().p2().y() - line().p1().y() ) ).normalized().adjusted (
           -extra, -extra, extra, extra );
}

QPainterPath dbse::SchemaGraphicArrow::shape() const
{
  QPainterPath path = QGraphicsLineItem::shape();
  path.addPolygon ( ArrowHead );
  path.addText ( line().p2() + QPoint ( 10, 10 ), QFont ( "Helvetica [Cronyx]", 10 ),
                 Cardinality );
  path.addText ( line().p1() + QPoint ( -10, -10 ), QFont ( "Helvetica [Cronyx]", 10 ),
                 Cardinality );
  return path;
}

void dbse::SchemaGraphicArrow::UpdatePosition()
{
  QLineF line ( mapFromItem ( Start, 0, 0 ), mapFromItem ( End, 0, 0 ) );
  setLine ( line );
}

dbse::SchemaGraphicObject * dbse::SchemaGraphicArrow::GetStartItem() const
{
  return Start;
}

dbse::SchemaGraphicObject * dbse::SchemaGraphicArrow::GetEndItem() const
{
  return End;
}

bool dbse::SchemaGraphicArrow::GetInheritanceMode()
{
  return Inheritance;
}

void dbse::SchemaGraphicArrow::RemoveArrow()
{
  if ( Inheritance )
  {
    /// Remove Super class
  }
  else
  {
    /// Remove relationship
  }
}

void dbse::SchemaGraphicArrow::SetLabelScene ( SchemaGraphicsScene * Scene )
{
  Scene->addItem ( Label );
}

void dbse::SchemaGraphicArrow::paint ( QPainter * painter,
                                       const QStyleOptionGraphicsItem * option,
                                       QWidget * widget )
{
  Q_UNUSED ( option )
  Q_UNUSED ( widget )

  if ( Start->collidesWithItem ( End ) )
  {
    return;
  }

  QPen myPen = pen();
  myPen.setColor ( Qt::black );

  QFont Font ( "Helvetica [Cronyx]", 10 );
  qreal arrowSize = 10;
  painter->setFont ( Font );
  painter->setPen ( myPen );
  painter->setBrush ( Qt::black );

  QLineF centerLine ( Start->mapToScene ( Start->boundingRect().center() ),
                      End->mapToScene ( End->boundingRect().center() ) );
  QPolygonF startPolygon = QPolygonF ( Start->boundingRect() );
  QPolygonF endPolygon = QPolygonF ( End->boundingRect() );
  QPointF p1 = endPolygon.first() + End->pos();
  QPointF p2;
  QPointF intersectPointStart;
  QPointF intersectPointEnd;
  QLineF polyLine;

  for ( int i = 1; i < endPolygon.count(); ++i )
  {
    p2 = endPolygon.at ( i ) + End->pos();
    polyLine = QLineF ( p1, p2 );
    QLineF::IntersectType intersectType = polyLine.intersect ( centerLine, &intersectPointEnd );

    if ( intersectType == QLineF::BoundedIntersection )
    {
      break;
    }

    p1 = p2;
  }

  p1 = startPolygon.first() + Start->pos();

  for ( int i = 1; i < startPolygon.count(); ++i )
  {
    p2 = startPolygon.at ( i ) + Start->pos();
    polyLine = QLineF ( p1, p2 );
    QLineF::IntersectType intersectType = polyLine.intersect ( centerLine,
                                                               &intersectPointStart );

    if ( intersectType == QLineF::BoundedIntersection )
    {
      break;
    }

    p1 = p2;
  }

  setLine ( QLineF ( intersectPointEnd, intersectPointStart ) );

  double angle = ::acos ( line().dx() / line().length() );

  if ( line().dy() >= 0 )
  {
    angle = ( M_PI * 2 ) - angle;
  }

  QPointF arrowP1 = line().p1()
                    + QPointF ( sin ( angle + M_PI / 3 ) * arrowSize, cos ( angle + M_PI / 3 ) * arrowSize );
  QPointF arrowP2 = line().p1()
                    + QPointF ( sin ( angle + M_PI - M_PI / 3 ) * arrowSize,
                                cos ( angle + M_PI - M_PI / 3 ) * arrowSize );
  QPointF middlePoint = QPointF ( ( arrowP1.x() + arrowP2.x() ) / 2,
                                  ( arrowP1.y() + arrowP2.y() ) / 2 );
  QPointF arrowP3 = QPointF ( line().p1().x() - 2 * ( line().p1().x() - middlePoint.x() ),
                              line().p1().y() - 2 * ( line().p1().y() - middlePoint.y() ) );
  ArrowHead.clear();
  ArrowHead << line().p1() << arrowP1 << arrowP2;

  QFontMetrics Metrics ( Font );
  Metrics.boundingRect ( Name + " - " + Cardinality );

  painter->drawLine ( line() );
  qreal degree = ( angle * 180 ) / M_PI;

  if ( Label )
  {

    Label->setRotation ( -degree + LastDegree );

    if ( degree >= 90 && degree < 270 )
    {
      Label->setTransformOriginPoint ( Label->boundingRect().center() );
      Label->setRotation ( -180 );
      Label->setTransformOriginPoint ( 0, 0 );
      LastRotation = 180;
      Label->setPos ( line().p2() + QPointF ( -5 * cos ( angle ), 5 * sin ( angle ) ) );
    }
    else
    {
      Label->setTransformOriginPoint ( Label->boundingRect().center() );
      Label->setRotation ( 360 );
      Label->setTransformOriginPoint ( 0, 0 );
      LastRotation = -180;

      if ( Composite )
      {
        Label->setPos ( line().p1() + QPointF ( 20 * cos ( angle ), -20 * sin ( angle ) ) );
      }
      else
      {
        Label->setPos ( line().p1() + QPointF ( 5 * cos ( angle ), -5 * sin ( angle ) ) );
      }
    }

    LastDegree = degree;
  }

  if ( Inheritance )
  {
    painter->drawPolygon ( ArrowHead );
  }
  else if ( Composite )
  {
    /// Draw Rhombus
    ArrowHead.clear();
    ArrowHead << line().p1() << arrowP1 << arrowP3 << arrowP2;
    painter->drawPolygon ( ArrowHead );
  }
}
