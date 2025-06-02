#ifndef SCHEMAGRAPHICNOTE_H
#define SCHEMAGRAPHICNOTE_H

/// Including QT Headers
#include <QGraphicsObject>
#include <QGraphicsSceneHoverEvent>
#include <QFont>
#include <QColor>
#include <QPen>
#include <QRectF>

namespace dbse
{
  class SchemaGraphicNote : public QGraphicsObject
  {
    Q_OBJECT
    public:

    explicit SchemaGraphicNote ( const QString& name, const QString& text,
                                 QGraphicsObject* parent = nullptr );
    ~SchemaGraphicNote() = default;

    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] QPainterPath shape() const override;
    void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option,
                 QWidget * widget ) override;


    [[nodiscard]] QString text() {return m_text;}
    void update_note (QString text);
    void open_editor();
  protected:
    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent* ev ) override;
  signals:
    void updated();

  private:
    QString m_name;
    QString m_text;
    QFont m_font;
    QFont m_bold_font;
    QColor m_default_color;
    QColor m_highlight_color;
    QColor m_background_color;


  };
}  // namespace dbse
#endif // SCHEMAGRAPHICNOTE_H
