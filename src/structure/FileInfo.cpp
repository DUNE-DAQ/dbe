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

FileInfo::FileInfo(const QString& name) : m_filename(name) {
  m_short_name = prune_path(m_filename);
  parse_objects();
  check_includes();
}

void FileInfo::parse_objects() {
  m_objects.clear();
  for (auto const& class_name :
         config::api::info::onclass::allnames <std::vector<std::string>>()) {
    for (auto obj : config::api::info::onclass::objects(class_name, false)) {
      auto file = prune_path(QString::fromStdString(obj.contained_in()));
      if (file == m_short_name) {
        m_objects.insert({QString::fromStdString(obj.full_name()), obj});
      }
    }
  }
}

bool FileInfo::check_includes() {
  m_message.clear();
  m_missing_schema.clear();
  m_missing_data.clear();
  QStringList includes(config::api::get::file::inclusions_singlefile (
                         m_filename));
  for (auto [id, obj] : m_objects) {
    dunedaq::conffwk::class_t const & classdef =
      dbe::config::api::info::onclass::definition (obj.class_name(), false);

    auto schema_file = QString::fromStdString(classdef.p_schema_path);
    if (!match_path(schema_file, m_filename, includes)) {
      m_message += QString("Object <i>" + id + "</i> is of class <i>"
                           + QString::fromStdString(obj.class_name())
                           + "</i> defined in file <b>" + schema_file
                           + "</b> which is not included<br>");
      m_missing_schema.insert(prune_path(schema_file));
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
        if (!(FileInfo::match_path(file, m_filename, includes))) {
          m_message += QString("Object <i>" + id + "</i> has relationship to <i>"
                               + QString::fromStdString(relobj.full_name())
                               + "</i> in file <b>" + file
                               + "</b> which is not included<br>");
          m_missing_data.insert(FileInfo::prune_path(file));
        }
      }
    }
  }
  return m_message.isEmpty();
}

const QString& FileInfo::data_path() {
  if (s_data_path == ".") {
    s_data_path = s_schema_path;
  }
  return s_data_path;
}
void FileInfo::update_data_path(const QString& path) {
  s_data_path = path;
}

const QString& FileInfo::schema_path() {
  if (s_schema_path == ".") {
    s_schema_path = s_data_path;
  }
  return s_schema_path;
}
void FileInfo::update_schema_path(const QString& path) {
  s_schema_path = path;
}


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


FileInfoWidget::FileInfoWidget(QString filename, QWidget* /*parent*/)
  : m_ui(new Ui::FileInfo),
    m_file(filename), m_uuid(QUuid::createUuid()) {

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

  m_file.setup_paths();
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

void FileInfoWidget::filemodel_updated() {
  if (m_updating) {
    return;
  }
  parse_includes();
  parse_objects();
}


void FileInfoWidget::parse_objects() {
  m_file.parse_objects();
  m_file.check_includes();
  m_ui->object_list->clear();
  for (auto const& [obj_name, obj_ref] : m_file.objects()) {
    auto item = new QListWidgetItem(obj_name);
    m_ui->object_list->addItem(item);
  }

  m_ui->object_list->update();

  m_ui->add_missing_schema->setVisible(!m_file.missing_schema().empty());
  m_ui->add_missing->setVisible(!m_file.missing_data().empty());

  m_ui->message->setText(m_file.message());
  m_ui->warningBox->setVisible(!m_file.message().isEmpty());
}


void FileInfoWidget::parse_includes() {
  QStringList includes(config::api::get::file::inclusions_singlefile (
                         m_file.name()));
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

void FileInfoWidget::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}

void FileInfoWidget::accept() {
  close();
}
void FileInfoWidget::reject() {
  close();
}

void FileInfoWidget::edit_object_slot() {
  auto item = m_ui->object_list->currentItem();
  edit_object_slot(item);
}

void FileInfoWidget::edit_object_slot(QListWidgetItem* item) {
  auto name = item->text();
  if (m_file.objects().contains(name)) {
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

    auto obj = m_file.objects().at(name);
    auto oe = new ObjectEditor(obj);
    oe->show();
  }
}

void FileInfoWidget::delete_object_slot() {
  auto name = m_ui->object_list->currentItem()->text();
  if (m_file.objects().contains(name)) {
    config::api::commands::delobj(m_file.objects().at(name), m_uuid);
  }
  parse_objects();
}

void FileInfoWidget::rename_object_slot() {
  auto name = m_ui->object_list->currentItem()->text();
  if (m_file.objects().contains(name)) {
    QInputDialog dia(this);
    dia.setLabelText("Enter new name for " + name);
    auto code = dia.exec();
    if (code == QDialog::Accepted) {
      std::string new_name = dia.textValue().toStdString();
      config::api::commands::renobj(m_file.objects().at(name), new_name, m_uuid);
    }
  }
  parse_objects();
}


void FileInfoWidget::add_datafile() {
  auto fd = new QFileDialog ( this, tr ( "Open Data File" ),
                              m_file.data_path(),
                              tr ( "XML data files (*.data.xml)" ) );
  add_includefile(fd);
  if (fd->result() == QDialog::Accepted) {
    m_file.update_data_path(fd->directory().path());
  }
}

void FileInfoWidget::add_schemafile() {
  auto fd = new QFileDialog ( this, tr ( "Open Schema File" ),
                              m_file.schema_path(),
                              tr ( "XML schema files (*.schema.xml)" ) );
  add_includefile(fd);
  if (fd->result() == QDialog::Accepted) {
    m_file.update_schema_path (fd->directory().path());
  }
}

void FileInfoWidget::add_includefile(QFileDialog* fd) {
  fd->setFileMode ( QFileDialog::ExistingFiles );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptOpen );
  fd->setSidebarUrls(m_file.get_path_urls());
  if (fd->exec() == QDialog::Accepted) {
    auto files = fd->selectedFiles();
    QScopedValueRollback<bool> rb(m_updating,true);
    for (auto file: files) {
      file = m_file.prune_path(file);
      config::api::commands::file::add(m_file.name(), file);
    }
    parse_includes();
    parse_objects();
  }
}

void FileInfoWidget::add_missing_schemafiles() {
  QScopedValueRollback<bool> rb(m_updating,true);
  for (const auto& file : m_file.missing_schema()) {
    config::api::commands::file::add(m_file.name(), file);
  }
  parse_includes();
  parse_objects();
}

void FileInfoWidget::add_missing_datafiles() {
  QScopedValueRollback<bool> rb(m_updating,true);
  for (const auto& file : m_file.missing_data()) {
    config::api::commands::file::add(m_file.name(), file);
  }
  parse_includes();
  parse_objects();
}

void FileInfoWidget::remove_schemafile_slot() {
  remove_includefile(m_ui->schema_list->currentItem()->text());
}
void FileInfoWidget::remove_datafile_slot() {
  remove_includefile(m_ui->data_list->currentItem()->text());
}
void FileInfoWidget::remove_includefile(const QString& file) {
  config::api::commands::file::remove(m_file.name(), file);
  parse_includes();
  parse_objects();
}


void FileInfoWidget::file_info_slot() {
  show_file_info(m_ui->data_list->currentItem()->text());
}

void FileInfoWidget::file_info_slot(QListWidgetItem* item) {
  show_file_info(item->text());
}

void FileInfoWidget::file_info_slot(const QString& filename) {
  show_file_info(filename);
}

void FileInfoWidget::show_file_info(const QString& filename) {
  for ( QWidget * widget : QApplication::allWidgets() ) {
    auto fi = dynamic_cast<FileInfoWidget *> ( widget );
    if ( fi != nullptr ) {
      if (fi->objectName() == filename) {
        fi->raise();
        fi->setVisible ( true );
        fi->activateWindow();
        return;
      }
    }
  }

  auto fi = new FileInfoWidget(filename);
  fi->show();
}

void FileInfoWidget::activate_schema_context_menu (QPoint pos) {
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

void FileInfoWidget::activate_data_context_menu (QPoint pos) {
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

void FileInfoWidget::activate_object_context_menu (QPoint pos) {
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
