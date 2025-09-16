/// Including QT Headers
#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>

#include <QToolTip>

#include "dbe/SchemaGraphicNote.hpp"
#include "dbe/SchemaGraphicsScene.hpp"
#include "dbe/SchemaNoteEditor.hpp"
#include "dbe/SchemaStyle.hpp"

#include <iostream>

namespace dbse {

SchemaGraphicNote::SchemaGraphicNote (const QString& name,
                                      const QString& text,
                                      QGraphicsObject* parent)
  : QGraphicsObject (parent),
    m_name(name),
    m_text(text) {

  setAcceptHoverEvents(true);

  setFlag ( ItemIsMovable );
  setFlag ( ItemSendsGeometryChanges, true );
  setFlag ( ItemSendsScenePositionChanges, true );

}

void SchemaGraphicNote::open_editor() {
  bool editorFound = false;

  for ( auto widget : QApplication::allWidgets() ) {
    auto editor = dynamic_cast<SchemaNoteEditor*> (widget);
    if ( editor != nullptr ) {
      if ( (editor->objectName() ).compare (m_name) == 0 ) {
        editor->raise();
        editor->setVisible ( true );
        editor->activateWindow();
        editorFound = true;
      }
    }
  }

  if ( !editorFound ) {
    auto editor = new SchemaNoteEditor ( this );
    editor->show();
  }
}

void SchemaGraphicNote::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent* ) {
  open_editor();
}


void SchemaGraphicNote::update_note (QString text) {
  auto this_scene = dynamic_cast<SchemaGraphicsScene*>(scene());
  this_scene->remove_note_object(this);
  m_text = text;
  this_scene->addItem(this);
  emit updated();
}

QRectF SchemaGraphicNote::boundingRect() const
{
  QFontMetrics font_metrics(SchemaStyle::get_font("note"));
  double height = font_metrics.height()/2;
  double width = 0;
  for (auto line: m_text.split("\n")) {
    height += font_metrics.height();
    QRectF font_rect = font_metrics.boundingRect(line);
    if (font_rect.width() > width) {
      width = font_rect.width();
    }
  }
  width += font_metrics.averageCharWidth()*2;

  return QRectF(0, 0, width, height);
  //return font_metrics.boundingRect(m_text);
}

void SchemaGraphicNote::paint (QPainter* painter,
                               const QStyleOptionGraphicsItem* /*option*/,
                               QWidget* /*widget*/ ) {

  const QPen pen(SchemaStyle::get_color("foreground", "note"), 0.5);
  QBrush brush(SchemaStyle::get_color("background", "note"));
  painter->setFont (SchemaStyle::get_font("note"));
  painter->setBrush (brush);
//  painter->setBackgroundMode(Qt::OpaqueMode);
  painter->setPen ( pen );
  auto rect = boundingRect();
  painter->drawRect ( rect );
  painter->drawText(rect.adjusted(5,5,-2,-2), Qt::AlignJustify, m_text);
}

QPainterPath SchemaGraphicNote::shape() const {
  QPainterPath path;
  path.addRect ( boundingRect() );
  return path;
}
} //namespace dbse
