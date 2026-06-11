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
#include <QMessageBox>
#include <QScopedValueRollback>
#include <QString>
#include <QWidget>

namespace dbegraph = dbe::config::api::graph;

namespace dbe {

QString FileInfo::s_schema_path{"."};
QString FileInfo::s_data_path{"."};
QStringList FileInfo::s_path_list{};
QList<QUrl> FileInfo::s_path_urls{};
std::map<QString, std::map<QString, const tref>> FileInfo::s_obj_map{};
std::map<QString, std::set<QString>> FileInfo::s_missing_schema_map{};
std::map<QString, std::set<QString>> FileInfo::s_missing_data_map{};

void FileInfo::setup_paths() {
  QString DUNEDAQ_DB_PATH = getenv ( "DUNEDAQ_DB_PATH" );
  s_path_list.clear();
  s_path_urls.clear();
  auto path_list = DUNEDAQ_DB_PATH.split (QLatin1Char(':'), Qt::SkipEmptyParts );
  for ( QString & path : path_list ) {
    char* rpath =  realpath(path.toStdString().c_str(), NULL);
    path = QString(rpath);
    free(rpath);

    if (! path.isEmpty()) {
      if ( !path.endsWith ( "/" ) ) {
        path.append ( "/" );
      }
      s_path_urls.append(QUrl::fromLocalFile(path));
      s_path_list.append(path);
    }
  }
}

QString FileInfo::prune_path(QString file) {
  if (s_path_list.isEmpty()) {
    setup_paths();
  }
  for (const QString& element : s_path_list) {
    if (file.startsWith(element)) {
      file.remove(element);
      break;
    }
  }
  return file;
}

bool FileInfo::match_path(const QString& file,
                          const QString& top_file,
                          const QStringList& includes) {
  if (top_file.endsWith(file)) {
    return true;
  }

  if (s_path_list.isEmpty()) {
    setup_paths();
  }

  QStringList candidates{file};
  for (const auto& element : s_path_list) {
    if (file.startsWith(element)) {
      auto short_name = file;
      candidates.append(short_name.remove(element));
    }
  }

  for (auto cand : candidates) {
    if (includes.contains(cand)) {
      return true;
    }
  }
  return false;
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

void FileInfo::parse_all_objects() {
  s_obj_map.clear();
  for (auto const& class_name :
         config::api::info::onclass::allnames <std::vector<std::string>>()) {
    for (auto obj : config::api::info::onclass::objects(class_name, false)) {
      auto file = prune_path(QString::fromStdString(obj.contained_in()));
      auto name = QString::fromStdString(obj.full_name());
      if (!s_obj_map.contains(file)) {
        s_obj_map.insert({file,{}});
        s_missing_schema_map.insert({file,{}});
        s_missing_data_map.insert({file,{}});
      }
      s_obj_map.at(file).insert({name, obj});
    }
  }
}

QString FileInfo::check_file_includes(const QString& filename) {
  QString message{};

  auto fname = prune_path(filename);
  if (!s_missing_schema_map.contains(fname)) {
    s_missing_schema_map.insert({fname,{}});
  } else {
    s_missing_schema_map.at(fname).clear();
  }
  if (!s_missing_data_map.contains(fname)) {
    s_missing_data_map.insert({fname,{}});
  } else {
    s_missing_data_map.at(fname).clear();
  }
  QStringList includes(config::api::get::file::inclusions_singlefile (
                         filename));
  if (s_obj_map.contains(prune_path(filename))) {
    for (auto [id, obj] : s_obj_map.at(prune_path(filename))) {
      dunedaq::conffwk::class_t const & classdef =
        dbe::config::api::info::onclass::definition (obj.class_name(), false);

      auto schema_file = QString::fromStdString(classdef.p_schema_path);
      if (!match_path(schema_file, filename, includes)) {
        message += QString("Object <i>" + id + "</i> is of class <i>"
                           + QString::fromStdString(obj.class_name())
                           + "</i> defined in file <b>" + schema_file
                           + "</b> which is not included<br>");
        s_missing_schema_map.at(fname).insert(prune_path(schema_file));
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
          if (!(match_path(file, filename, includes))) {
            message += QString("Object <i>" + id + "</i> has relationship to <i>"
                               + QString::fromStdString(relobj.full_name())
                               + "</i> in file <b>" + file
                               + "</b> which is not included<br>");
            s_missing_data_map.at(fname).insert(prune_path(file));
          }
        }
      }
    }
  }
  return message;
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

  connect (m_ui->add_missing_schema, SIGNAL(pressed()), this, SLOT (add_missing_schemafiles()));
  connect (m_ui->add_missing, SIGNAL(pressed()), this, SLOT (add_missing_datafiles()));

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
  if (m_updating) {
    return;
  }
  parse_includes();
  parse_objects();
}


void FileInfo::parse_objects() {
  parse_all_objects();
  m_ui->object_list->clear();
  auto fname = prune_path(m_filename);
  if (s_obj_map.contains(fname)) {
    auto& omap = s_obj_map.at(fname);
    for (auto const& [obj_name, obj_ref] : omap) {
      auto item = new QListWidgetItem(obj_name);
      m_ui->object_list->addItem(item);
    }
  }
  m_ui->object_list->update();

  auto status = check_includes();
  if (!s_missing_schema_map.contains(fname)) {
    s_missing_schema_map.insert({fname,{}});
  }
  if (!s_missing_data_map.contains(fname)) {
    s_missing_data_map.insert({fname,{}});
  }
  m_ui->add_missing_schema->setVisible(!s_missing_schema_map.at(fname).empty());
  m_ui->add_missing->setVisible(!s_missing_data_map.at(fname).empty());

  m_ui->warningBox->setVisible(!status);
}


bool FileInfo::check_includes() {
  QString message = check_file_includes(m_filename);
  m_ui->message->setText(message);
  return message.isEmpty();
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
  if (s_obj_map.at(prune_path(m_filename)).contains(name)) {
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

    auto obj = s_obj_map.at(prune_path(m_filename)).at(name);
    auto oe = new ObjectEditor(obj);
    oe->show();
  }
}

void FileInfo::delete_object_slot() {
  auto name = m_ui->object_list->currentItem()->text();
  auto file = prune_path(m_filename);
  if (s_obj_map.at(file).contains(name)) {
    config::api::commands::delobj(s_obj_map.at(file).at(name), m_uuid);
  }
  parse_objects();
}

void FileInfo::rename_object_slot() {
  auto name = m_ui->object_list->currentItem()->text();
  auto file = prune_path(m_filename);
  if (s_obj_map.at(file).contains(name)) {
    QInputDialog dia(this);
    dia.setLabelText("Enter new name for " + name);
    auto code = dia.exec();
    if (code == QDialog::Accepted) {
      std::string new_name = dia.textValue().toStdString();
      config::api::commands::renobj(s_obj_map.at(file).at(name), new_name, m_uuid);
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
    QScopedValueRollback<bool> rb(m_updating,true);
    for (auto file: files) {
      file = prune_path(file);
      config::api::commands::file::add(m_filename, file);
    }
    parse_includes();
    parse_objects();
  }
}

void FileInfo::add_missing_schemafiles() {
  QScopedValueRollback<bool> rb(m_updating,true);
  auto short_filename = prune_path(m_filename);
  if (!s_missing_schema_map.contains(short_filename)) {
    QMessageBox::warning (this, "Warning", 
                          QString("Missing schema map is corrupt and does not contain %1").arg(short_filename));
    return;
  }
  for (const auto& file : s_missing_schema_map.at(short_filename)) {
    config::api::commands::file::add(m_filename, file);
  }
  parse_includes();
  parse_objects();
}

void FileInfo::add_missing_datafiles() {
  QScopedValueRollback<bool> rb(m_updating,true);
  auto short_filename = prune_path(m_filename);
  if (!s_missing_data_map.contains(short_filename)) {
    QMessageBox::warning (this, "Warning", 
                          QString("Missing data map is corrupt and does not contain %1").arg(short_filename));
    return;
  }
  for (const auto& file : s_missing_data_map.at(prune_path(m_filename))) {
    config::api::commands::file::add(m_filename, file);
  }
  parse_includes();
  parse_objects();
}

void FileInfo::remove_schemafile_slot() {
  remove_includefile(m_ui->schema_list->currentItem()->text());
}
void FileInfo::remove_datafile_slot() {
  remove_includefile(m_ui->data_list->currentItem()->text());
}
void FileInfo::remove_includefile(const QString& file) {
  config::api::commands::file::remove(m_filename, file);
  parse_includes();
  parse_objects();
}


void FileInfo::file_info_slot() {
  show_file_info(m_ui->data_list->currentItem()->text());
}

void FileInfo::file_info_slot(QListWidgetItem* item) {
  show_file_info(item->text());
}

void FileInfo::file_info_slot(const QString& filename) {
  show_file_info(filename);
}

void FileInfo::show_file_info(const QString& filename) {
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

      auto remove = new QAction("Remove include file", this);
      connect (remove, SIGNAL(triggered()), this, SLOT (remove_schemafile_slot()));
      m_schema_menu->addAction(remove);
    }
  }

  if (m_ui->schema_list->currentIndex().isValid()) {
    m_schema_menu->actions().at(1)->setVisible (true);
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

      auto remove = new QAction("Remove include file", this);
      connect (remove, SIGNAL(triggered()), this, SLOT (remove_datafile_slot()));
      m_data_menu->addAction(remove);
    }
  }

  if (m_ui->data_list->currentIndex().isValid()) {
    m_data_menu->actions().at(0)->setVisible (true);
    m_data_menu->actions().at(2)->setVisible (true);
  }
  else {
    m_data_menu->actions().at(0)->setVisible (false);
    m_data_menu->actions().at(2)->setVisible (false);
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
