/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/GraphicalClass.h to include/dbe/GraphicalClass.hpp).
 */

#ifndef GRAPHICALCLASS_H
#define GRAPHICALCLASS_H

/// Including QT
#include <QGraphicsObject>
/// Including config
#include "conffwk/Schema.hpp"

namespace dbe
{

struct InitAttributeFromEnv
{
  QString AttributeName;
  QStringList EnvNames;
};

struct DualRelationship
{
  QString Direct;
  QString Reverse;
};

struct GraphicalClass
{
  QString GraphicalUID;
  QString DatabaseClassName;
  QString GenericPixmapFile;
  QString UsedPixmapFile;
  QString BitmapFile;
  QString BitmapMaskFile;
  bool ShowAllAttributes;
  QStringList Attributes;
  bool ShowAllRelationships;
  QStringList Relationships;
  QString IconTitle;
  bool DerivedClassesLoaded;
  QStringList DerivedClasses;
  QList<DualRelationship> DualRelationships;
  QList<InitAttributeFromEnv> NeedToInitialize;
};

struct Window
{
  QString Title;
  QStringList GraphicalClassesList;
  bool ShowChildren;
};

class GraphicalObject: public QGraphicsObject
{
  Q_OBJECT
public:
  explicit GraphicalObject ( bool Used, QString ObjectName, GraphicalClass GraphInfo,
                             QGraphicsObject * parent = 0 );
  ~GraphicalObject();
  QRectF boundingRect() const;
  void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option,
               QWidget * widget );
  QPainterPath shape() const;
  /// Event Handler
  void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
  void mousePressEvent ( QGraphicsSceneMouseEvent * event );
  void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
  void dropEvent ( QGraphicsSceneDragDropEvent * event );
  /// Graphics
  void AddGraphicChildren();
  void ShowGraphicsChildren();
  void HideGraphicsChildren();
  double GetExpandedX() const;
  double GetExpandedY() const;
  void SetExpandedX ( double dx );
  void SetExpandedY ( double dy );
  /// Some Api
  void GetOffsetX ( QGraphicsItem * Item, double & Offset );
  void GetOffsetY ( QGraphicsItem * Item, double & Offset );
  void LocateTopMostItem ( QGraphicsItem * Auxiliary );
  QString GetDatabaseClassName() const;
  QString GetDatabaseUidName() const;
private:
  GraphicalClass GraphicalInfo;
  QGraphicsPixmapItem * IconItem;
  QGraphicsTextItem * TextItem;
  QString DatabaseClassName;
  QString DatabaseUidName;
  bool IsExpanded;
  bool IsFetched;
  QPointF StartDrag;
  double expandedX;
  double expandedY;
};

class GraphicalRelationship: public QGraphicsObject
{
  Q_OBJECT
public:
  explicit GraphicalRelationship ( QString ObjectName, QString ClassName,
                                   dunedaq::conffwk::relationship_t & Data, QGraphicsObject * parent =
                                     0 );
  ~GraphicalRelationship();
  QRectF boundingRect() const;
  void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option,
               QWidget * widget );
  QPainterPath shape() const;
  /// Event Handler
  void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
  void mousePressEvent ( QGraphicsSceneMouseEvent * event );
  void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
  /// Graphics
  void AddGraphicChildren();
private:
  QString DatabaseClassName;
  QString DatabaseUidName;
  dunedaq::conffwk::relationship_t RelationshipData;
  QGraphicsTextItem * TextItem;
};

} // end namespace dbe
#endif // GRAPHICALCLASS_H
