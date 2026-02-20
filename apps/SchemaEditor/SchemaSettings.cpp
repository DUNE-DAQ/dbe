#include <QSettings>
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

  QSettings settings("dunedaq", "dbse");
  settings.beginGroup("view defaults");
  m_ui->abstract_button->setChecked(settings.value("highlight_abstract", false).toBool());
  connect (m_ui->abstract_button, SIGNAL(stateChanged(int)),
           this, SLOT(toggle_abstract(int)));
  m_ui->active_button->setChecked(settings.value("highlight_active", false).toBool());
  connect (m_ui->active_button, SIGNAL(stateChanged(int)),
           this, SLOT(toggle_active(int)));
  m_ui->inherited_button->setChecked(settings.value("show_inherited", false).toBool());
  connect (m_ui->inherited_button, SIGNAL(stateChanged(int)),
           this, SLOT(toggle_inherited(int)));
  m_ui->default_value_button->setChecked(settings.value("show_default", false).toBool());
  connect (m_ui->default_value_button, SIGNAL(stateChanged(int)),
           this, SLOT(toggle_default(int)));
}

void SchemaSettings::toggle_abstract(int state) {
  QSettings settings("dunedaq", "dbse");
  settings.beginGroup("view defaults");
  settings.setValue("highlight_abstract", (state>0));
}
void SchemaSettings::toggle_active(int state) {
  QSettings settings("dunedaq", "dbse");
  settings.beginGroup("view defaults");
  settings.setValue("highlight_active", (state>0));
}
void SchemaSettings::toggle_inherited(int state) {
  QSettings settings("dunedaq", "dbse");
  settings.beginGroup("view defaults");
  settings.setValue("show_inherited", (state>0));
}
void SchemaSettings::toggle_default(int state) {
  QSettings settings("dunedaq", "dbse");
  settings.beginGroup("view defaults");
  settings.setValue("show_default", (state>0));
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
