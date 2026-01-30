#include <QColorDialog>
#include <QSettings>
#include <QStringList>

#include "dbe/Preferences.hpp"
#include "dbe/StyleUtility.hpp"

#include "ui_Preferences.h"

namespace dbe {

Preferences::Preferences(QWidget* parent)
  : QDialog(parent), m_ui(new Ui::Preferences)
{
  m_ui->setupUi(this);
  setObjectName("Preferences");

  QSettings settings;
  auto item = new QListWidgetItem("Attribute Foreground");
  item->setToolTip("Foreground colour for attributes");
  item->setForeground(settings.value("attribute/foreground").value<QColor>());
  m_ui->color_list->addItem(item);

  item = new QListWidgetItem("Attribute Background");
  item->setToolTip("Normal background colour for attributes");
  item->setForeground(settings.value("attribute/background").value<QColor>());
  m_ui->color_list->addItem(item);

  item = new QListWidgetItem("Attribute Default-Background");
  item->setToolTip("Background colour for attributes set to their default value");
  item->setForeground(settings.value("attribute/default-background").value<QColor>());
  m_ui->color_list->addItem(item);

  item = new QListWidgetItem("Relationship Foreground");
  item->setToolTip("Foreground colour for relationships");
  item->setForeground(settings.value("relationship/foreground").value<QColor>());
  m_ui->color_list->addItem(item);

  item = new QListWidgetItem("Relationship Background");
  item->setToolTip("background colour for relationships");
  item->setForeground(settings.value("relationship/background").value<QColor>());
  m_ui->color_list->addItem(item);

  item = new QListWidgetItem("Readonly Foreground");
  item->setToolTip("Foreground text colour for readonly items");
  item->setForeground(settings.value("readonly/foreground").value<QColor>());
  m_ui->color_list->addItem(item);

  item = new QListWidgetItem("Readonly Background");
  item->setToolTip("Background text colour for readonly items");
  item->setForeground(settings.value("readonly/background").value<QColor>());
  m_ui->color_list->addItem(item);

  connect (m_ui->color_list, SIGNAL(itemActivated(QListWidgetItem*)),
           this, SLOT(set_color(QListWidgetItem*)));

}

void Preferences::set_color(QListWidgetItem* item) {
  QSettings settings;
  QStringList text = item->text().toLower().split(" ");
  settings.beginGroup(text.at(0));

  auto color = settings.value(text.at(1)).value<QColor>();
  auto title = QString("Select color for ") + item->text();
  color = QColorDialog::getColor(color, nullptr, title);

  if (color.isValid()) {
    settings.setValue(text.at(1), color);
    StyleUtility::InitColorManagement();
  }
}
} // namespace dbe
