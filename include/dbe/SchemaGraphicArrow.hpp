/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/SchemaGraphicArrow.h to include/dbe/SchemaGraphicArrow.hpp).
 */

#ifndef SCHEMAGRAPHICARROW_H
#define SCHEMAGRAPHICARROW_H

/// Including QT Headers
#include <QGraphicsLineItem>
/// Including Schema Editor
#include "dbe/SchemaGraphicObject.hpp"
#include "dbe/SchemaGraphicsScene.hpp"

namespace dbse
{

class SchemaGraphicArrow: public QGraphicsLineItem
{
public:
  SchemaGraphicArrow ( SchemaGraphicObject * StartItem, SchemaGraphicObject * EndItem,
                       bool IsInheritance, bool IsComposite, QString ArrowName,
                       QString ArrowCardinality, QGraphicsItem * parent = nullptr );
  ~SchemaGraphicArrow();
  QRectF boundingRect() const;
  QPainterPath shape() const;
  void UpdatePosition();
  SchemaGraphicObject * GetStartItem() const;
  SchemaGraphicObject * GetEndItem() const;
  bool GetInheritanceMode();
  void RemoveArrow();
  void SetLabelScene ( SchemaGraphicsScene * Scene );
protected:
  //void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
  //void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
  //void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
  void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option,
               QWidget * widget = 0 );
private:
  SchemaGraphicObject * Start;
  SchemaGraphicObject * End;
  QPolygonF ArrowHead;
  bool Inheritance;
  bool Composite;
  QString Name;
  QString Cardinality;
  QGraphicsSimpleTextItem * Label;
  double LastDegree;
  double LastRotation;
  //QString LabelString;
};

}  // namespace dbse
#endif // SCHEMAGRAPHICARROW_H
