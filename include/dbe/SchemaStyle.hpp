#ifndef SCHEMASTYLE_H
#define SCHEMASTYLE_H

#include <QColor>
#include <QPalette>
#include <QString>

namespace dbse {

  struct SchemaStyle {
    static void load();
    [[nodiscard]] static QColor get_color(const QString& item, const QString& group);
    static QColor set_color(const QString& item, const QString& group);
    [[nodiscard]] static QFont get_font(const QString& group);
    static QFont set_font(const QString& group);

  };

} // namespace dbse

#endif // SCHEMASTYLE_H
