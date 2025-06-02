/// Including QT Headers
#include <QPainter>
#include <QPen>
#include <QToolTip>
/// Including Schema Editor
#include "dbe/SchemaGraphicSegmentedArrow.hpp"
#include "dbe/SchemaKernelWrapper.hpp"

/// Including Oks Headers
#include "oks/class.hpp"

/// Including C++ Headers
#include <cmath>

namespace dbse {

SchemaGraphicSegmentedArrow::SchemaGraphicSegmentedArrow ( SchemaGraphicObject * start_item,
                                                           SchemaGraphicObject * end_item,
                                                           int connection_count,
                                                           bool is_inheritance,
                                               bool is_composite, QString arrow_name,
                                               QString arrow_cardinality, QGraphicsItem * parent )
  : QGraphicsPathItem ( parent ),
    m_start_item ( start_item ),
    m_end_item ( end_item ),
    m_connection_count ( connection_count),
    m_inheritance ( is_inheritance ),
    m_composite ( is_composite ),
    m_name ( arrow_name ),
    m_cardinality ( arrow_cardinality ),
    LastDegree ( 0 ),
    LastRotation ( 0 )
{
  if (!is_inheritance) {
    setAcceptHoverEvents(true);
  }
  //setPen(QPen(Qt::black,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
  setFlag ( ItemIsSelectable, true );
  m_default_color = QColor ( 0x1e1b18 );
  m_label_font = QFont( "Helvetica [Cronyx]", 9);
  m_arrow_size = 20;

}

SchemaGraphicSegmentedArrow::~SchemaGraphicSegmentedArrow()
{
}

void dbse::SchemaGraphicSegmentedArrow::hoverEnterEvent ( QGraphicsSceneHoverEvent* he) {
  if (!m_name.isEmpty()) {
    auto info = m_start_item->GetClass();
    
    auto rel = info->find_relationship(m_name.toStdString());
    
    QToolTip::showText( he->screenPos(),
                        QString::fromStdString(rel->get_description()) );
  }
  he->ignore();
}
void dbse::SchemaGraphicSegmentedArrow::hoverLeaveEvent ( QGraphicsSceneHoverEvent* he) {
  QToolTip::hideText();
  he->ignore();
}

QRectF SchemaGraphicSegmentedArrow::boundingRect() const
{
  if ( m_start_item->collidesWithItem ( m_end_item ) )
  {
    return QRectF();
  }

  qreal extra = ( pen().width() + 20 ) / 2.0 + 10;
  auto br =  QRectF ( p1(),
                  QSizeF ( p2().x() - p1().x(),
                           p2().y() - p1().y() ) ).normalized().adjusted (
           -extra, -extra, extra, extra );
  br = br.united(m_rel_label_br);
  br = br.united(m_rel_cardinality_br);
  return br;
}

QPainterPath SchemaGraphicSegmentedArrow::shape() const
{
  QPainterPath path = QGraphicsPathItem::shape();
  path.addRect(m_rel_label_br);
  path.addRect(m_rel_cardinality_br);
  path.addPolygon ( m_marker );
  // path.addText ( p2() + QPoint ( 10, 10 ), QFont ( "Helvetica [Cronyx]", 10 ),
                //  m_cardinality );
  // path.addText ( p1() + QPoint ( -10, -10 ), QFont ( "Helvetica [Cronyx]", 10 ),
                //  m_cardinality );
  return path;
}

void SchemaGraphicSegmentedArrow::UpdatePosition()
{
  if ( m_start_item->collidesWithItem ( m_end_item ) ) {
    // this->setPath(QPainterPath());
    // m_marker = QPolygonF();
    return;
  }

  std::vector<QLineF> norms = { 
    {0.,0., 0., 1.}, // up
    {0.,0., 1., 0.}, // right
    {0.,0., 0, -1.}, // down
    {0.,0., -1., 0.}, // left
  };

  qreal xoffset;
  qreal yoffset;
  const qreal xfactor = 23.0;
  const qreal yfactor = 17.0;
  if (m_start_item->boundingRect().x() < m_end_item->boundingRect().x()) {
    xoffset = m_connection_count*xfactor;
  }
  else {
    xoffset = m_connection_count*-xfactor;
  }

  if (m_start_item->boundingRect().y() > m_end_item->boundingRect().y()) {
    yoffset = m_connection_count*yfactor;
  }
  else {
    yoffset = m_connection_count*-yfactor;
  }

  QPointF start_offset(xoffset, yoffset);
  QPointF end_offset(0, 0);

  QLineF center_line ( m_start_item->mapToScene ( m_start_item->boundingRect().center()+start_offset ),
                      m_end_item->mapToScene ( m_end_item->boundingRect().center()+end_offset ) );
  QPolygonF start_polygon = QPolygonF ( m_start_item->boundingRect() );
  QPolygonF end_polygon = QPolygonF ( m_end_item->boundingRect() );
  QPointF intersect_point_start, intersect_point_end;
  QLineF intersect_norm_start, intersect_norm_end;

  // Iterate on the sides starting from top-left
  QPointF p1 = end_polygon.first() + m_end_item->pos(), p2;
  for ( int i = 1; i < end_polygon.count(); ++i )
  {

    p2 = end_polygon.at ( i ) + m_end_item->pos();
    QLineF item_side = QLineF ( p1, p2 );
    QLineF::IntersectType intersect_type = item_side.intersects ( center_line, &intersect_point_end );

    if ( intersect_type == QLineF::BoundedIntersection ) {
      intersect_norm_end = norms[i-1];
      break;
    }

    p1 = p2;
  }

  p1 = start_polygon.first() + m_start_item->pos();
  QPointF start_sp;
  for ( int i = 1; i < start_polygon.count(); ++i )
  {
    p2 = start_polygon.at ( i ) + m_start_item->pos();
    QLineF item_side = QLineF ( p1, p2 );
    QLineF::IntersectType intersect_type = item_side.intersects ( center_line,
                                                               &intersect_point_start );
    if ( intersect_type == QLineF::BoundedIntersection ) {
      switch (i) {
      case 1:
        start_sp=QPointF {m_start_item->pos().x() + (m_start_item->boundingRect().width() / 2) + xoffset,
        intersect_point_start.y()};
        break;
      case 3:
        start_sp=QPointF {m_start_item->pos().x() + (m_start_item->boundingRect().width() / 2) - xoffset,
        intersect_point_start.y()};
        break;

      case 2:
      case 4: start_sp={intersect_point_start.x(),
          m_start_item->pos().y()+m_start_item->boundingRect().height() / 2+yoffset};
        break;
      }
      intersect_norm_start = norms[i-1];
      break;
    }

    p1 = p2;
  }

  QLineF direct_line(start_sp, intersect_point_end);
  auto label_br = QRectF(QFontMetrics ( m_label_font ).boundingRect ( m_name ));
  // Center rectangle on origin
  label_br.translate(-label_br.width()/2, label_br.height()/2);


  auto cardinality_br = QRectF(QFontMetrics ( m_label_font ).boundingRect ( m_cardinality ));
  // Center rectangle on origin
  cardinality_br.translate(-cardinality_br.width()/2, cardinality_br.height()/2);


  qreal label_x_padding = 2;
  qreal label_y_padding = -2;
  qreal card_x_padding = 20;
  qreal card_y_padding = 0;

  QPainterPath path( intersect_point_start );

  if ((intersect_norm_start.dx() == 0) && (intersect_norm_end.dx() == 0)) {
    // Three segments, starting and ending vertically
    QPointF wp1 = QPointF(direct_line.x1(),
                          direct_line.center().y()+yoffset);
    QPointF wp2 = QPointF(direct_line.x2(),
                          direct_line.center().y()+yoffset);
    path.moveTo(direct_line.x1(), direct_line.y1());
    path.lineTo(wp1);
    path.lineTo(wp2);

    // Place the label
    label_br.translate( wp2 
      + QPointF(
        (direct_line.dx() < 0 ? 1 : -1) * (label_x_padding+label_br.width()/2), 
        (direct_line.dy() > 0 ? 1 : -1) * (label_br.height()+label_y_padding)
        )
    );
    // Place cardinality
    cardinality_br.translate( intersect_point_start
      + QPointF(
        (direct_line.dx() < 0 ? 1 : -1) * (card_x_padding+cardinality_br.width()/2), 
        (direct_line.dy() > 0 ? 1 : -1) * (cardinality_br.height()+card_y_padding)
      )
    );

  } else if ((intersect_norm_start.dy() == 0) && (intersect_norm_end.dy() == 0)) {
    // Three segments, starting and ending horizontally
    QPointF wp1 = QPointF(direct_line.center().x()-xoffset,
                          direct_line.y1());
    QPointF wp2 = QPointF(direct_line.center().x()-xoffset,
                          direct_line.y2());

    path.moveTo(direct_line.x1(), direct_line.y1());
    path.lineTo(wp1);
    path.lineTo(wp2);

    // Place the label
    label_br.translate( wp2 
      + QPointF(
        (direct_line.dx() > 0 ? 1 : -1) * (label_x_padding+label_br.width()/2), 
        (direct_line.dy() < 0 ? 1 : -1) * (label_br.height()+label_y_padding)
        )
    );
    cardinality_br.translate( intersect_point_start
      + QPointF(
        (direct_line.dx() > 0 ? 1 : -1) * (card_x_padding+cardinality_br.width()/2), 
        (direct_line.dy() < 0 ? 1 : -1) * (cardinality_br.height()+card_y_padding)
      )
    );

  } else if (intersect_norm_start.dx() == 0) {
    // Two segments, starting vertically, ending horizontally
    QPointF wp1 = QPointF(direct_line.x1(), direct_line.y2());

    path.moveTo(direct_line.x1(), direct_line.y1());
    path.lineTo(wp1);

    label_br.translate( wp1 
      + QPointF(
        (direct_line.dx() > 0 ? 1 : -1) * (label_x_padding+label_br.width()/2), 
        (direct_line.dy() < 0 ? 1 : -1) * (label_br.height()+label_y_padding)
        )
    );
    cardinality_br.translate( intersect_point_start
      + QPointF(
        (direct_line.dx() < 0 ? 1 : -1) * (card_x_padding+cardinality_br.width()/2), 
        (direct_line.dy() > 0 ? 1 : -1) * (cardinality_br.height()+card_y_padding)
      )
    );

  } else if (intersect_norm_start.dy() == 0) {
    // Two segments, starting horizontally, ending vertically
    QPointF wp1 = QPointF(direct_line.x2(), direct_line.y1() );
    path.moveTo(direct_line.x1(), direct_line.y1());

    path.lineTo(wp1);


    label_br.translate( wp1 
      + QPointF(
        (direct_line.dx() > 0 ? 1 : -1) * (label_x_padding+label_br.width()/2), 
        (direct_line.dy() < 0 ? 1 : -1) * (label_br.height()+label_y_padding)
        )
    );
    cardinality_br.translate( intersect_point_start
      + QPointF(
        (direct_line.dx() < 0 ? 1 : -1) * (card_x_padding+cardinality_br.width()/2), 
        (direct_line.dy() > 0 ? 1 : -1) * (cardinality_br.height()+card_y_padding)
      )
    );

  }

  path.lineTo( intersect_point_end );
  setPath(path);
  
  m_rel_label_br = label_br;
  m_rel_cardinality_br = cardinality_br;

  // Why does a function called UpdatePosition add decorations to a line??
  // =====================================================================
  if ( m_inheritance )
  {
    m_marker = this->make_arrow_head(-intersect_norm_end.angle()/180*M_PI).translated(this->p2());
  } else {
    m_marker = this->make_rhombus(-intersect_norm_start.angle()/180*M_PI).translated(this->p1());
  }
}

SchemaGraphicObject * SchemaGraphicSegmentedArrow::GetStartItem() const
{
  return m_start_item;
}

SchemaGraphicObject * SchemaGraphicSegmentedArrow::GetEndItem() const
{
  return m_end_item;
}

bool SchemaGraphicSegmentedArrow::GetInheritanceMode()
{
  return m_inheritance;
}

void SchemaGraphicSegmentedArrow::RemoveArrow()
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

QPolygonF SchemaGraphicSegmentedArrow::make_arrow_head(qreal rotation) const {

  float opening_angle = atan2(1, 0.5);
  qreal size = m_arrow_size;
  QPointF a1 = QPointF ( sin ( rotation + opening_angle ) * size, cos ( rotation + opening_angle ) * size );
  QPointF a2 = QPointF ( sin ( rotation + M_PI - opening_angle ) * size, cos ( rotation + M_PI - opening_angle ) * size );

  QPolygonF arrow;
  arrow << QPointF(0,0) << a1 << a2;
  return arrow;
}

QPolygonF SchemaGraphicSegmentedArrow::make_rhombus(qreal rotation) const {

  float opening_angle = M_PI / 3;
  qreal size = 0.6*m_arrow_size;

  QPointF a1 = QPointF ( sin ( rotation + opening_angle ) * size, cos ( rotation + opening_angle ) * size );
  QPointF a2 = QPointF ( sin ( rotation + M_PI - opening_angle ) * size, cos ( rotation + M_PI - opening_angle ) * size );

  QPointF m = QPointF ( ( a1.x() + a2.x() ) / 2, ( a1.y() + a2.y() ) / 2 );
  QPointF a3 = QPointF ( 2 * m.x(), 2 * m.y() );
  
  QPolygonF rhombus;
  rhombus << QPointF(0,0) << a1 << a3 << a2;
  return rhombus;
}



void SchemaGraphicSegmentedArrow::paint ( QPainter * painter,
                                       const QStyleOptionGraphicsItem * option,
                                       QWidget * widget )
{
  Q_UNUSED ( option )
  Q_UNUSED ( widget )

  if ( m_start_item->collidesWithItem ( m_end_item ) )
  {
    return;
  }

  const QPen line_pen = QPen(m_default_color, 1);
  const QPen arrow_pen = QPen(m_default_color, 1);

  painter->setFont ( m_label_font );
  painter->setPen ( line_pen );
  painter->setBrush ( {} );
  painter->setRenderHint(QPainter::Antialiasing);


  painter->drawPath ( this->path());
   
  painter->setPen ( arrow_pen );
  if ( !m_inheritance ) {
    painter->drawText(m_rel_label_br, Qt::AlignTop | Qt::AlignLeft, QString(m_name));
    painter->drawText(m_rel_cardinality_br, Qt::AlignTop | Qt::AlignLeft, QString(m_cardinality));
  }

  if ( m_inheritance )
  {
    painter->setBrush ( Qt::white );
  }
  else if ( m_composite )
  {
    /// Draw Rhombus
    painter->setBrush ( Qt::black );


  } else {
    /// Draw Rhombus
    painter->setBrush ( Qt::white );
  }
  painter->drawPolygon ( m_marker );

}

} // namespace dbse
