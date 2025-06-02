#ifndef SCHEMAGRAPHICOBJECT_H
#define SCHEMAGRAPHICOBJECT_H

/// Including QT Headers
#include <QGraphicsObject>
#include <QGraphicsSceneHoverEvent>
#include <QFont>
#include <QColor>
#include <QPen>

/// Including Oks Headers
#include "oks/class.hpp"

namespace dbse
{

class SchemaGraphicSegmentedArrow;

class SchemaGraphicObject: public QGraphicsObject
{
  Q_OBJECT
public:
  explicit SchemaGraphicObject ( QString & ClassName, QGraphicsObject * parent = nullptr );
  ~SchemaGraphicObject();

  [[nodiscard]] dunedaq::oks::OksClass * GetClass() const;
  [[nodiscard]] QString GetClassName() const;
  void GetInfo();
  /// Graphic API
  void set_inherited_properties_visibility( bool visible );
  void set_highlight_active( bool highlight );
  [[nodiscard]] QRectF boundingRect() const override;
  [[nodiscard]] QPainterPath shape() const override;
  void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option,
               QWidget * widget ) override;
  /// Arrow API
  void AddArrow ( SchemaGraphicSegmentedArrow * Arrow );
  void RemoveArrow ( SchemaGraphicSegmentedArrow * Arrow );
  void RemoveArrows();
  bool HasArrow ( SchemaGraphicObject * Dest ) const;
protected:
  QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
  void hoverEnterEvent ( QGraphicsSceneHoverEvent* ev );
  void hoverLeaveEvent ( QGraphicsSceneHoverEvent* ev );
  void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent* ev );
private:
  dunedaq::oks::OksClass * m_class_info;
  QString m_class_object_name;
  QStringList m_class_attributes;
  QStringList m_class_methods;
  QStringList m_class_relationhips;

  QStringList m_class_inherited_attributes;
  QStringList m_class_inherited_relationhips;
  QStringList m_class_inherited_methods;

  bool m_inherited_properties_visible;  
  bool m_highlight_active{false};
  QFont m_font;
  QFont m_bold_font;
  QColor m_default_color;
  QColor m_highlight_color;
  QColor m_opaque_color;

  double LineOffsetX;
  double LineOffsetY;
  QList<SchemaGraphicSegmentedArrow *> m_arrows;
private slots:
  void UpdateObject ( QString Name );
  void RemoveObject ( QString Name );
};

}  // namespace dbse
#endif // SCHEMAGRAPHICOBJECT_H
