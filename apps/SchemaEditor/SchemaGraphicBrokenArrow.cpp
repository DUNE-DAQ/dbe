/// Including QT Headers
#include <QPainter>
#include <QPen>
/// Including Schema Editor
#include "dbe/SchemaGraphicBrokenArrow.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
/// Including C++ Headers
#include <cmath>

namespace dbse {

SchemaGraphicBrokenArrow::SchemaGraphicBrokenArrow ( SchemaGraphicObject * start_item,
                                               SchemaGraphicObject * end_item, bool is_inheritance,
                                               bool is_composite, QString arrow_name,
                                               QString arrow_cardinality, QGraphicsItem * parent )
  : QGraphicsPathItem ( parent ),
    m_start_item ( start_item ),
    m_end_item ( end_item ),
    m_inheritance ( is_inheritance ),
    m_composite ( is_composite ),
    m_name ( arrow_name ),
    m_cardinality ( arrow_cardinality ),
    m_label ( nullptr ),
    LastDegree ( 0 ),
    LastRotation ( 0 )
{
  //setPen(QPen(Qt::black,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
  setFlag ( ItemIsSelectable, true );
  //LabelString = m_name + " - " + m_cardinality;

  if ( !m_start_item->collidesWithItem ( m_end_item ) && !m_inheritance )
  {
    m_label = new QGraphicsSimpleTextItem ( m_name + " - " + m_cardinality );
    m_label->setParentItem ( this );
  }

}

SchemaGraphicBrokenArrow::~SchemaGraphicBrokenArrow()
{
}

QRectF SchemaGraphicBrokenArrow::boundingRect() const
{
    if ( m_start_item->collidesWithItem ( m_end_item ) )
  {
    if ( m_label ) {
      return m_label->boundingRect();
    } else {
      return QRectF();
    }
  }

  qreal extra = ( pen().width() + 20 ) / 2.0 + 10;
  return QRectF ( p1(),
                  QSizeF ( p2().x() - p1().x(),
                           p2().y() - p1().y() ) ).normalized().adjusted (
           -extra, -extra, extra, extra );
}

QPainterPath SchemaGraphicBrokenArrow::shape() const
{
  QPainterPath path = QGraphicsPathItem::shape();
  path.addPolygon ( ArrowHead );
  path.addText ( p2() + QPoint ( 10, 10 ), QFont ( "Helvetica [Cronyx]", 10 ),
                 m_cardinality );
  path.addText ( p1() + QPoint ( -10, -10 ), QFont ( "Helvetica [Cronyx]", 10 ),
                 m_cardinality );
  return path;
}

void SchemaGraphicBrokenArrow::UpdatePosition()
{
  QPainterPath path( mapFromItem ( m_start_item, 0, 0 ) );
  path.lineTo(mapFromItem ( m_end_item, 0, 0 ));
  setPath(path);
  // QLineF line ( mapFromItem ( m_start_item, 0, 0 ), mapFromItem ( m_end_item, 0, 0 ) );
  // setLine ( line );
}

SchemaGraphicObject * SchemaGraphicBrokenArrow::GetStartItem() const
{
  return m_start_item;
}

SchemaGraphicObject * SchemaGraphicBrokenArrow::GetEndItem() const
{
  return m_end_item;
}

bool SchemaGraphicBrokenArrow::GetInheritanceMode()
{
  return m_inheritance;
}

void SchemaGraphicBrokenArrow::RemoveArrow()
{
  if ( m_inheritance )
  {
    /// Remove Super class
  }
  else
  {
    /// Remove relationship
  }
}

void SchemaGraphicBrokenArrow::SetLabelScene ( SchemaGraphicsScene * Scene )
{
  Scene->addItem ( m_label );
}

void SchemaGraphicBrokenArrow::paint ( QPainter * painter,
                                       const QStyleOptionGraphicsItem * option,
                                       QWidget * widget )
{
  Q_UNUSED ( option )
  Q_UNUSED ( widget )

  if ( m_start_item->collidesWithItem ( m_end_item ) )
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

  QLineF centerLine ( m_start_item->mapToScene ( m_start_item->boundingRect().center() ),
                      m_end_item->mapToScene ( m_end_item->boundingRect().center() ) );
  QPolygonF startPolygon = QPolygonF ( m_start_item->boundingRect() );
  QPolygonF endPolygon = QPolygonF ( m_end_item->boundingRect() );
  QPointF p1 = endPolygon.first() + m_end_item->pos();
  QPointF p2;
  QPointF intersectPointStart;
  QPointF intersectPointEnd;
  QLineF polyLine;

  for ( int i = 1; i < endPolygon.count(); ++i )
  {
    p2 = endPolygon.at ( i ) + m_end_item->pos();
    polyLine = QLineF ( p1, p2 );
    QLineF::IntersectType intersectType = polyLine.intersect ( centerLine, &intersectPointEnd );

    if ( intersectType == QLineF::BoundedIntersection )
    {
      break;
    }

    p1 = p2;
  }

  p1 = startPolygon.first() + m_start_item->pos();

  for ( int i = 1; i < startPolygon.count(); ++i )
  {
    p2 = startPolygon.at ( i ) + m_start_item->pos();
    polyLine = QLineF ( p1, p2 );
    QLineF::IntersectType intersectType = polyLine.intersect ( centerLine,
                                                               &intersectPointStart );

    if ( intersectType == QLineF::BoundedIntersection )
    {
      break;
    }

    p1 = p2;
  }

  QPainterPath path( intersectPointEnd );
  path.lineTo( intersectPointStart );
  setPath(path);

  // setLine ( QLineF ( intersectPointEnd, intersectPointStart ) );

  double angle = ::acos ( this->dx() / this->path().length() );

  if ( this->dy() >= 0 )
  {
    angle = ( M_PI * 2 ) - angle;
  }

  QPointF arrowP1 = this->p1()
                    + QPointF ( sin ( angle + M_PI / 3 ) * arrowSize, cos ( angle + M_PI / 3 ) * arrowSize );
  QPointF arrowP2 = this->p1()
                    + QPointF ( sin ( angle + M_PI - M_PI / 3 ) * arrowSize,
                                cos ( angle + M_PI - M_PI / 3 ) * arrowSize );
  QPointF middlePoint = QPointF ( ( arrowP1.x() + arrowP2.x() ) / 2,
                                  ( arrowP1.y() + arrowP2.y() ) / 2 );
  QPointF arrowP3 = QPointF ( this->p1().x() - 2 * ( this->p1().x() - middlePoint.x() ),
                              this->p1().y() - 2 * ( this->p1().y() - middlePoint.y() ) );
  ArrowHead.clear();
  ArrowHead << this->p1() << arrowP1 << arrowP2;

  QFontMetrics Metrics ( Font );
  Metrics.boundingRect ( m_name + " - " + m_cardinality );

  painter->drawPath ( this->path());
  qreal degree = ( angle * 180 ) / M_PI;

  if ( m_label )
  {

    m_label->setRotation ( -degree + LastDegree );

    if ( degree >= 90 && degree < 270 )
    {
      m_label->setTransformOriginPoint ( m_label->boundingRect().center() );
      m_label->setRotation ( -180 );
      m_label->setTransformOriginPoint ( 0, 0 );
      LastRotation = 180;
      m_label->setPos ( this->p2() + QPointF ( -5 * cos ( angle ), 5 * sin ( angle ) ) );
    }
    else
    {
      m_label->setTransformOriginPoint ( m_label->boundingRect().center() );
      m_label->setRotation ( 360 );
      m_label->setTransformOriginPoint ( 0, 0 );
      LastRotation = -180;

      if ( m_composite )
      {
        m_label->setPos ( this->p1() + QPointF ( 20 * cos ( angle ), -20 * sin ( angle ) ) );
      }
      else
      {
        m_label->setPos ( this->p1() + QPointF ( 5 * cos ( angle ), -5 * sin ( angle ) ) );
      }
    }

    LastDegree = degree;
  }

  if ( m_inheritance )
  {
    painter->drawPolygon ( ArrowHead );
  }
  else if ( m_composite )
  {
    /// Draw Rhombus
    ArrowHead.clear();
    ArrowHead << this->p1() << arrowP1 << arrowP3 << arrowP2;
    painter->drawPolygon ( ArrowHead );
  }
}

} // namespace dbse
