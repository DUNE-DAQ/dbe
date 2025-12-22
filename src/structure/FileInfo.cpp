#include "dbe/confaccessor.hpp"
#include "dbe/config_api.hpp"
#include "dbe/FileInfo.hpp"
#include "dbe/ObjectEditor.hpp"
#include "dbe/StyleUtility.hpp"

#include "ui_FileInfo.h"

#include <QListWidgetItem>
#include <QString>
#include <QWidget>

namespace dbe {

FileInfo::FileInfo(QString filename, QWidget* /*parent*/)
  : m_ui(new Ui::FileInfo) {

  QWidget::setAttribute(Qt::WA_DeleteOnClose);

  m_ui->setupUi(this);
  m_ui->filename->setText(filename);
  if (confaccessor::check_file_rw(filename)) {
    m_ui->rstatus->setText(QString("RW"));
  }
  else {
    m_ui->rstatus->setText(QString("RO"));
  }

  setObjectName(filename);
  setWindowTitle("File:  " + filename.section('/',-1));

  QStringList includes(config::api::get::file::inclusions_singlefile (
                         filename));
  for (auto inc: includes) {
    auto item = new QListWidgetItem(inc);
    if (!confaccessor::check_file_rw(inc)) {
      item->setForeground(QBrush(StyleUtility::FileReadOnlyForeground));
      item->setBackground(QBrush(StyleUtility::FileReadOnlyBackground));
    }
    if (inc.endsWith(".schema.xml")) {
      m_ui->schema_list->addItem(item);
    }
    if (inc.endsWith(".data.xml")) {
      m_ui->data_list->addItem(item);
      
    }
  }
  m_ui->schema_list->update();
  m_ui->data_list->update();
  connect (m_ui->data_list, SIGNAL (itemActivated(QListWidgetItem*)),
           this, SLOT (file_info_slot(QListWidgetItem*)) );

  for (auto const& class_name :
         config::api::info::onclass::allnames <std::vector<std::string>>()) {
    for (auto obj : config::api::info::onclass::objects(class_name, false)) {
      if (obj.contained_in().ends_with(filename.toStdString())) {
        auto name = QString::fromStdString(obj.full_name());
        auto item = new QListWidgetItem(name);
        m_ui->objects->addItem(item);
        m_obj_map.insert({name, obj});
      }
    }
  }
  connect (m_ui->objects, SIGNAL (itemActivated(QListWidgetItem*)),
           this, SLOT (edit_object_slot(QListWidgetItem*)) );

}

void FileInfo::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}

void FileInfo::accept() {
  close();
}
void FileInfo::reject() {
  close();
}

void FileInfo::edit_object_slot(QListWidgetItem* item) {
  auto name = item->text();
  if (m_obj_map.contains(name)) {
    for ( QWidget * widget : QApplication::allWidgets() ) {
      auto oe = dynamic_cast<ObjectEditor*> ( widget );
      if ( oe != nullptr ) {
        if (oe->objectName() == name) {
          oe->raise();
          oe->setVisible ( true );
          oe->activateWindow();
          return;
        }
      }
    }

    auto obj = m_obj_map.at(name);
    auto oe = new ObjectEditor(obj);
    oe->show();
  }
}

void FileInfo::file_info_slot(QListWidgetItem* item) {
  show_file_info(item->text());
}

void FileInfo::file_info_slot(QString filename) {
  show_file_info(filename);
}

void FileInfo::show_file_info(QString filename) {
  for ( QWidget * widget : QApplication::allWidgets() ) {
    auto fi = dynamic_cast<FileInfo *> ( widget );
    if ( fi != nullptr ) {
      if (fi->objectName() == filename) {
        fi->raise();
        fi->setVisible ( true );
        fi->activateWindow();
        return;
      }
    }
  }

  auto fi = new FileInfo(filename);
  fi->show();
}

} //namespace dbe
