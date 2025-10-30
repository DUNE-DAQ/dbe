#include "dbe/SchemaStyle.hpp"

#include <QColor>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>

namespace dbse {

  void SchemaStyle::load() {
    // QSettings settings(".dbse.config", QSettings::IniFormat);
    QSettings settings("dunedaq", "dbse");

    settings.beginGroup("default");
    if (!settings.contains("background")) {
      QColor color = Qt::white;
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color = Qt::black;
      settings.setValue("foreground", color);
    }
    settings.endGroup();

    settings.beginGroup("active_file");
    if (!settings.contains("background")) {
      QColor color = Qt::white;
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color = Qt::blue;
      settings.setValue("foreground", color);
    }
    settings.endGroup();

    settings.beginGroup("highlight");
    if (!settings.contains("background")) {
      QColor color = Qt::white;
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color = Qt::magenta;
      settings.setValue("foreground", color);
    }
    settings.endGroup();

    settings.beginGroup("inherited");
    if (!settings.contains("background")) {
      QColor color = Qt::white;
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color {0x804040};
      settings.setValue("foreground", color);
    }
    settings.endGroup();

    settings.beginGroup("readonly");
    if (!settings.contains("background")) {
      QColor color = Qt::gray;
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color = Qt::darkRed;
      settings.setValue("foreground", color);
    }
    settings.endGroup();

    settings.beginGroup("error");
    if (!settings.contains("background")) {
      QColor color = Qt::white;
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color = Qt::red;
      settings.setValue("foreground", color);
    }
    settings.endGroup();

    settings.beginGroup("note");
    if (!settings.contains("background")) {
      QColor color {0xfff0ff};
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color = Qt::black;
      settings.setValue("foreground", color);
    }
    settings.endGroup();

    settings.beginGroup("line");
    if (!settings.contains("background")) {
      QColor color = Qt::black;
      settings.setValue("background", color);
    }
    if (!settings.contains("foreground")) {
      QColor color = Qt::black;
      settings.setValue("foreground", color);
    }
    settings.endGroup();

  }

  QColor SchemaStyle::get_color(const QString& item,
                               const QString& group) {
    QSettings settings("dunedaq", "dbse");  
    return settings.value(group+"/"+item).value<QColor>();
  }

  QColor SchemaStyle::set_color(const QString& item,
                               const QString& group) {
    QSettings settings("dunedaq", "dbse");
    settings.beginGroup(group);
    auto color = settings.value(item).value<QColor>();

    color = QColorDialog::getColor(color);

    if (color.isValid()) {
      settings.setValue(item, color);
    }
    return color;
  }


  QFont SchemaStyle::get_font(const QString& group) {
    QSettings settings("dunedaq", "dbse");  
    QString key{group+"/font"};
    if (settings.contains(key)) {
      return settings.value(key).value<QFont>();
    }
    return  QFont("Helvetica [Cronyx]", 9);
  }

  QFont SchemaStyle::set_font(const QString& group) {
    QSettings settings("dunedaq", "dbse");  
    settings.beginGroup(group);
    QFont font;
    if (settings.contains("font")) {
      font = settings.value("font").value<QFont>();
    }
    else {
      font = QFont("Helvetica [Cronyx]", 9);
    }
    font = QFontDialog::getFont(nullptr, font);
    settings.setValue("font", font);
    return font;
  }

} // namespace dbse
