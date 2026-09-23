#ifndef SchemaGraphicSegmentedArrow_H
#define SchemaGraphicSegmentedArrow_H

/// Including QT Headers
#include <QGraphicsPathItem>
/// Including Schema Editor
#include "dbe/SchemaGraphicObject.hpp"
#include "dbe/SchemaGraphicsScene.hpp"

namespace dbse {

class SchemaGraphicSegmentedArrow : public QGraphicsPathItem
{
public:
  SchemaGraphicSegmentedArrow(SchemaGraphicObject* StartItem,
                              SchemaGraphicObject* EndItem,
                              int connection_count,
                              bool IsInheritance,
                              bool IsComposite,
                              QString ArrowName,
                              QString ArrowCardinality,
                              QGraphicsItem* parent = nullptr);
  ~SchemaGraphicSegmentedArrow();
  QRectF boundingRect() const;
  QPainterPath shape() const;
  void UpdatePosition();
  SchemaGraphicObject* GetStartItem() const;
  SchemaGraphicObject* GetEndItem() const;
  bool GetInheritanceMode();
  void RemoveArrow();

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
  void hoverEnterEvent(QGraphicsSceneHoverEvent* ev);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* ev);

private:
  QPointF p1() const { return path().elementCount() > 0 ? path().elementAt(0) : QPointF(); }
  QPointF p2() const { return path().elementCount() > 0 ? path().elementAt(path().elementCount() - 1) : QPointF(); }
  qreal dx() const { return p2().x() - p1().x(); }
  qreal dy() const { return p2().y() - p1().y(); }

  QPolygonF make_arrow_head(qreal angle) const;
  QPolygonF make_rhombus(qreal angle) const;

  SchemaGraphicObject* m_start_item;
  SchemaGraphicObject* m_end_item;
  int m_connection_count;
  QPolygonF m_marker;
  QRectF m_rel_label_br;
  QRectF m_rel_cardinality_br;
  bool m_inheritance;
  bool m_composite;
  QString m_name;
  QString m_cardinality;
  QColor m_default_color;
  QFont m_label_font;
  qreal m_arrow_size;

  double LastDegree;
  double LastRotation;
  // QString LabelString;
};

} // namespace dbse
#endif // SchemaGraphicSegmentedArrow_H
