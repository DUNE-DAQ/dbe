#include "dbe/confaccessor.hpp"
#include "dbe/config_api.hpp"
#include "dbe/config_api_commands.hpp"
#include "dbe/FileInfo.hpp"
#include "dbe/ObjectEditor.hpp"
#include "dbe/StyleUtility.hpp"

#include "ui_FileInfo.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QString>
#include <QWidget>

namespace dbe {

FileInfo::FileInfo(QString filename, QWidget* /*parent*/)
  : m_ui(new Ui::FileInfo), m_filename(filename), m_uuid(QUuid::createUuid()) {

  QWidget::setAttribute(Qt::WA_DeleteOnClose);

  m_ui->setupUi(this);
  m_ui->filename->setText(filename);
  if (confaccessor::check_file_rw(filename)) {
    m_ui->rstatus->setText(QString("RW"));
    m_readonly = false;
  }
  else {
    m_ui->rstatus->setText(QString("RO"));
    m_readonly = true;
  }

  setObjectName(filename);
  setWindowTitle("File:  " + filename.section('/',-1));

  parse_includes();
  parse_objects();

  m_ui->schema_list->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  m_ui->data_list->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  m_ui->object_list->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );

  connect (m_ui->add_schema, SIGNAL(pressed()), this, SLOT (add_schemafile()));
  connect (m_ui->add_data, SIGNAL(pressed()), this, SLOT (add_datafile()));

  connect (m_ui->schema_list, SIGNAL (customContextMenuRequested(QPoint)),
           this, SLOT (activate_schema_context_menu(QPoint)));

  connect (m_ui->data_list, SIGNAL (customContextMenuRequested(QPoint)),
           this, SLOT (activate_data_context_menu(QPoint)));
  connect (m_ui->data_list, SIGNAL (itemActivated(QListWidgetItem*)),
           this, SLOT (file_info_slot(QListWidgetItem*)) );

  connect (m_ui->object_list, SIGNAL (itemActivated(QListWidgetItem*)),
           this, SLOT (edit_object_slot(QListWidgetItem*)) );
  connect (m_ui->object_list, SIGNAL (customContextMenuRequested(QPoint)),
           this, SLOT (activate_object_context_menu(QPoint)));

  QString DUNEDAQ_DB_PATH = getenv ( "DUNEDAQ_DB_PATH" );
  m_path_list = DUNEDAQ_DB_PATH.split (QLatin1Char(':'), Qt::SkipEmptyParts );
  for ( QString & path : m_path_list ) {
    if ( !path.endsWith ( "/" ) ) {
      path.append ( "/" );
    }
    m_path_urls.append(QUrl::fromLocalFile(path));
  }
}

void FileInfo::parse_objects() {
  m_ui->object_list->clear();
  m_obj_map.clear();
  for (auto const& class_name :
         config::api::info::onclass::allnames <std::vector<std::string>>()) {
    for (auto obj : config::api::info::onclass::objects(class_name, false)) {
      if (obj.contained_in().ends_with(m_filename.toStdString())) {
        auto name = QString::fromStdString(obj.full_name());
        auto item = new QListWidgetItem(name);
        m_ui->object_list->addItem(item);
        m_obj_map.insert({name, obj});
      }
    }
  }
  m_ui->object_list->update();
}

void FileInfo::parse_includes() {
  QStringList includes(config::api::get::file::inclusions_singlefile (
                         m_filename));
  m_ui->schema_list->clear();
  m_ui->data_list->clear();
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

void FileInfo::edit_object_slot() {
  auto item = m_ui->object_list->currentItem();
  edit_object_slot(item);
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

void FileInfo::delete_object_slot() {
  auto name = m_ui->object_list->currentItem()->text();
  if (m_obj_map.contains(name)) {
    config::api::commands::delobj(m_obj_map.at(name), m_uuid);
  }
  parse_objects();
}

void FileInfo::rename_object_slot() {
  auto name = m_ui->object_list->currentItem()->text();
  if (m_obj_map.contains(name)) {
    QInputDialog dia(this);
    dia.setLabelText("Enter new name for " + name);
    auto code = dia.exec();
    if (code == QDialog::Accepted) {
      std::string new_name = dia.textValue().toStdString();
      config::api::commands::renobj(m_obj_map.at(name), new_name, m_uuid);
    }
  }
  parse_objects();
}

void FileInfo::add_datafile() {
  auto fd = new QFileDialog ( this, tr ( "Open Data File" ), ".",
                              tr ( "XML data files (*.data.xml)" ) );
  add_includefile(fd);
}
void FileInfo::add_schemafile() {
  auto fd = new QFileDialog ( this, tr ( "Open Schema File" ), ".",
                              tr ( "XML schema files (*.schema.xml)" ) );
  add_includefile(fd);
}
void FileInfo::add_includefile(QFileDialog* fd) {
  fd->setFileMode ( QFileDialog::ExistingFiles );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptOpen );
  fd->setSidebarUrls(m_path_urls);
  fd->exec();
  auto files = fd->selectedFiles();
  for (auto file: files) {
    for (const QString& element : m_path_list) {
      if (file.startsWith(element)) {
        file = file.remove(element);
        break;
      }
    }
    config::api::commands::file::add(m_filename, file);
  }
  parse_includes();
}


void FileInfo::file_info_slot() {
  show_file_info(m_ui->data_list->currentItem()->text());
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

void FileInfo::activate_schema_context_menu (QPoint pos) {
  if (m_schema_menu == nullptr) {
    m_schema_menu = new QMenu(this);

    if (!m_readonly) {
      auto add = new QAction("Add include file", this);
      connect (add, SIGNAL(triggered()), this, SLOT (add_schemafile()));
      m_schema_menu->addAction(add);
    }
  }

  m_schema_menu->exec (m_ui->schema_list->mapToGlobal(pos));
}

void FileInfo::activate_data_context_menu (QPoint pos) {
  if (m_data_menu == nullptr) {
    m_data_menu = new QMenu(this);

    auto info = new QAction(tr("Show file info"), this);
    connect (info, SIGNAL(triggered()), this, SLOT (file_info_slot()));
    m_data_menu->addAction(info);

    if (!m_readonly) {
      auto add = new QAction("Add include file", this);
      connect (add, SIGNAL(triggered()), this, SLOT (add_datafile()));
      m_data_menu->addAction(add);
    }
  }

  if (m_ui->data_list->currentIndex().isValid()) {
    m_data_menu->actions().at(0)->setVisible (true);
  }
  else {
    m_data_menu->actions().at(0)->setVisible (false);
  }
  m_data_menu->exec (m_ui->data_list->mapToGlobal(pos));
}

void FileInfo::activate_object_context_menu (QPoint pos) {
  if (m_object_menu == nullptr) {
    m_object_menu = new QMenu(this);

    auto edit_action = new QAction(tr("&Edit object"), this);
    connect (edit_action, SIGNAL(triggered()), this, SLOT(edit_object_slot()));
    m_object_menu->addAction(edit_action);

    auto delete_action = new QAction(tr("&Delete Object"), this );
    connect (delete_action, SIGNAL(triggered()), this, SLOT(delete_object_slot()));
    m_object_menu->addAction(delete_action);

    auto rename_action = new QAction(tr("&Rename Object"), this );
    connect (rename_action, SIGNAL(triggered()), this, SLOT(rename_object_slot()));
    m_object_menu->addAction(rename_action);
  }

  if (m_ui->object_list->currentIndex().isValid()) {
    m_object_menu->exec(m_ui->object_list->mapToGlobal(pos));
  }
}

} //namespace dbe
