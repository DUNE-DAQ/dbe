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

  for (auto entry : QStringList{"default/normal unhighlighted text",
        "active_file/items associated with the active file",
        "inherited/properties inherited from a parent class",
        "highlight/highlighted class",
        "readonly/items from a readonly file",
        "error/error messages",
        "note/notes on class diagrams",
        "line/lines showing inheritance or relationships between classes"}) {
    auto list = entry.split("/");
    auto group = list.at(0);
    auto fg_colour = SchemaStyle::get_color("foreground", group);
    auto bg_colour = SchemaStyle::get_color("background", group);
    for (auto fb: QStringList{"Foreground", "Background"}) {
      auto item= new QListWidgetItem(group+" "+fb);
      item->setForeground(fg_colour);
      item->setBackground(bg_colour);
      if (list.size()>1) {
        item->setToolTip(fb+" color for "+list.at(1)); 
      }
      m_ui->color_list->addItem(item);
    }
  }

  for (auto entry : QStringList{"default/normal unhighlighted text",
        "highlight/highlighted class",
        "abstract/abstract base class",
        "note/notes on class diagrams",
        "line/lines showing inheritance or relationships between classes"}) {
    auto list = entry.split("/");
    auto group = list.at(0);
    auto item= new QListWidgetItem(group+" font");
    item->setFont(SchemaStyle::get_font(group));
    if (list.size()>1) {
      item->setToolTip("Font for "+list.at(1)); 
    }
    m_ui->font_list->addItem(item);
  }

  connect (m_ui->color_list, SIGNAL(itemActivated(QListWidgetItem*)),
           this, SLOT(set_color(QListWidgetItem*)));
  connect (m_ui->font_list, SIGNAL(itemActivated(QListWidgetItem*)),
           this, SLOT(set_font(QListWidgetItem*)));

  QSettings settings;
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
  settings.endGroup();

  settings.beginGroup("MainWindow");
  m_ui->save_settings->setChecked(settings.value("saveLayout", false).toBool());
  connect (m_ui->save_settings, SIGNAL(stateChanged(int)),
           this, SLOT(toggle_save(int)));
  settings.endGroup();
}

void SchemaSettings::toggle_abstract(int state) {
  QSettings settings;
  settings.beginGroup("view defaults");
  settings.setValue("highlight_abstract", (state>0));
}
void SchemaSettings::toggle_active(int state) {
  QSettings settings;
  settings.beginGroup("view defaults");
  settings.setValue("highlight_active", (state>0));
}
void SchemaSettings::toggle_inherited(int state) {
  QSettings settings;
  settings.beginGroup("view defaults");
  settings.setValue("show_inherited", (state>0));
}
void SchemaSettings::toggle_default(int state) {
  QSettings settings;
  settings.beginGroup("view defaults");
  settings.setValue("show_default", (state>0));
}
void SchemaSettings::toggle_save(int state) {
  QSettings settings;
  settings.beginGroup("MainWindow");
  settings.setValue("saveLayout", (state>0));
}


void SchemaSettings::set_color(QListWidgetItem* item) {
  QStringList text = item->text().toLower().split(" ");
  SchemaStyle::set_color(text.at(1), text.at(0));
  item->setForeground(SchemaStyle::get_color("foreground", text.at(0)));
  item->setBackground(SchemaStyle::get_color("background", text.at(0)));
  m_ui->color_list->update();
  emit settings_updated();
}
void SchemaSettings::set_font(QListWidgetItem* item) {
  QStringList text = item->text().toLower().split(" ");
  SchemaStyle::set_font(text.at(0));
  emit settings_updated();
}
