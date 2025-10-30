#include <QStringList>

#include "dbe/SchemaSettings.hpp"
#include "dbe/SchemaStyle.hpp"

#include "ui_SchemaSettings.h"

using namespace dbse;

SchemaSettings::SchemaSettings(QWidget* parent)
  : QDialog(parent), m_ui(new Ui::SchemaSettings)
{
  m_ui->setupUi(this);
  setObjectName("Settings");

  SchemaStyle::load();

  for (auto group : QStringList{"default", "active_file", "inherited",
        "highlight", "readonly", "error", "note", "line"}) {
    m_ui->color_list->addItem(group+" Foreground");
    m_ui->color_list->addItem(group+" Background");
  }
  for (auto group : QStringList{"default", "highlight", "abstract", "note",
        "line"}) {
    m_ui->font_list->addItem(group);
  }

  connect (m_ui->color_list, SIGNAL(itemActivated(QListWidgetItem*)),
           this, SLOT(set_color(QListWidgetItem*)));
  connect (m_ui->font_list, SIGNAL(itemActivated(QListWidgetItem*)),
           this, SLOT(set_font(QListWidgetItem*)));

}

void SchemaSettings::set_color(QListWidgetItem* item) {
  QStringList text = item->text().toLower().split(" ");
  SchemaStyle::set_color(text.at(1), text.at(0));
  emit settings_updated();
}
void SchemaSettings::set_font(QListWidgetItem* item) {
  SchemaStyle::set_font(item->text());
  emit settings_updated();
}
