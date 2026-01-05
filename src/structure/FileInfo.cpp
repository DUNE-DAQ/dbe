#include "dbe/confaccessor.hpp"
#include "dbe/config_api.hpp"
#include "dbe/config_api_commands.hpp"
#include "dbe/FileInfo.hpp"
#include "dbe/MainWindow.hpp"
#include "dbe/ObjectEditor.hpp"
#include "dbe/StyleUtility.hpp"

#include "ui_FileInfo.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QString>
#include <QWidget>

namespace dbegraph = dbe::config::api::graph;

namespace dbe {

QString FileInfo::s_schema_path{"."};
QString FileInfo::s_data_path{"."};
QStringList FileInfo::s_path_list{};
QList<QUrl> FileInfo::s_path_urls{};

void FileInfo::setup_paths() {
  QString DUNEDAQ_DB_PATH = getenv ( "DUNEDAQ_DB_PATH" );
  s_path_list = DUNEDAQ_DB_PATH.split (QLatin1Char(':'), Qt::SkipEmptyParts );
  for ( QString & path : s_path_list ) {
    if ( !path.endsWith ( "/" ) ) {
      path.append ( "/" );
    }
    s_path_urls.append(QUrl::fromLocalFile(path));
  }
}

QString FileInfo::prune_path(QString file) {
  if (s_path_list.isEmpty()) {
    setup_paths();
  }
  for (const QString& element : s_path_list) {
    if (file.startsWith(element)) {
      file = file.remove(element);
      break;
    }
  }
  return file;
}

QList<QUrl>  FileInfo::get_path_urls(){
  if (s_path_urls.isEmpty()) {
    setup_paths();
  }
  return s_path_urls;
}
QStringList FileInfo::get_path_list(){
  if (s_path_list.isEmpty()) {
    setup_paths();
  }
  return s_path_list;
}

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

  setup_paths();
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

  connect (MainWindow::findthis(), SIGNAL(signal_new_file_model()),
           this, SLOT (filemodel_updated()));
}

void FileInfo::filemodel_updated() {
  parse_includes();
  parse_objects();
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

  m_ui->warningBox->setVisible(!check_includes());
}

bool FileInfo::check_includes() {
  bool status = true;
  QString message{};
  m_ui->message->setText(message);
  QStringList includes(config::api::get::file::inclusions_singlefile (
                         m_filename));
  for (auto [id, obj] : m_obj_map) {
    dunedaq::conffwk::class_t const & classdef =
      dbe::config::api::info::onclass::definition (obj.class_name(), false);

    auto schema_file = QString::fromStdString(classdef.p_schema_path);
    if (!includes.contains(prune_path(schema_file))) {
      message += QString("Object <i>" + id + "</i> is of class <i>"
                             + QString::fromStdString(obj.class_name())
                             + "</i> defined in file <b>" + schema_file
                             + "</b> which is not included<br>");
          status = false;
    }
    std::vector<tref> relobjs;
    for (auto rel: classdef.p_relationships) {
      if (config::api::info::relation::is_simple(rel)) {
        try {
          relobjs.push_back(dbegraph::linked::through::relation<tref> (obj, rel));
        }
        catch ( daq::dbe::config_object_retrieval_result_is_null const & e ) {
          // nothing needs be done to handle cases that a relation has not been set
        }
      }
      else {
        relobjs = dbegraph::linked::through::relation<std::vector<tref>> (obj, rel);
      }

      for (auto relobj : relobjs) {
        auto file = QString::fromStdString(relobj.contained_in());
        file = prune_path(file);
        if (!(prune_path(m_filename)==file || includes.contains(file))) {
          message += QString("Object <i>" + id + "</i> has relationship to <i>"
                             + QString::fromStdString(relobj.full_name())
                             + "</i> in file <b>" + file
                             + "</b> which is not included<br>");
          status = false;
        }
      }
    }
  }
  m_ui->message->setText(message);
  return status;
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
  if (s_data_path == ".") {
    s_data_path = s_schema_path;
  }
  auto fd = new QFileDialog ( this, tr ( "Open Data File" ), s_data_path,
                              tr ( "XML data files (*.data.xml)" ) );
  add_includefile(fd);
  if (fd->result() == QDialog::Accepted) {
    s_data_path = fd->directory().path();
  }
}
void FileInfo::add_schemafile() {
  if (s_schema_path == ".") {
    s_schema_path = s_data_path;
  }
  auto fd = new QFileDialog ( this, tr ( "Open Schema File" ), s_schema_path,
                              tr ( "XML schema files (*.schema.xml)" ) );
  add_includefile(fd);
  if (fd->result() == QDialog::Accepted) {
    s_schema_path = fd->directory().path();
  }
}
void FileInfo::add_includefile(QFileDialog* fd) {
  fd->setFileMode ( QFileDialog::ExistingFiles );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptOpen );
  fd->setSidebarUrls(s_path_urls);
  if (fd->exec() == QDialog::Accepted) {
    auto files = fd->selectedFiles();
    for (auto file: files) {
      file = prune_path(file);
      config::api::commands::file::add(m_filename, file);
    }
    parse_includes();
    parse_objects();
  }
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
