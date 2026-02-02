
#include "dbe/SchemaClassEditor.hpp"
#include "dbe/SchemaFileInfo.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include "dbe/SchemaStyle.hpp"
#include "oks/class.hpp"
#include "oks/kernel.hpp"  // for CanNotSetActiveFile exception
#include "ui_SchemaFileInfo.h"

#include <QBrush>
#include <QColor>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QDialogButtonBox>

namespace dbse {

SchemaFileInfo::SchemaFileInfo(std::string filename, QWidget* /*parent*/)
  : m_ui(new Ui::SchemaFileInfo), m_filename(filename),
    m_include_menu(nullptr), m_class_menu(nullptr) {

  QWidget::setAttribute(Qt::WA_DeleteOnClose);

  m_ui->setupUi(this);
  // m_ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  m_ui->missing_button->hide();
  // m_ui->class_list->setDragEnabled ( true );
  // m_ui->class_list->setAcceptDrops ( true );
  setObjectName(QString::fromStdString(filename));
  auto sp = filename.find_last_of('/');
  if (sp == std::string::npos) {
    sp = 0;
  }
  else {
    sp++;
  }
  setWindowTitle(
    QString("Schema File:  %1").arg(QString::fromStdString(filename.substr(sp))));

  m_ui->label->setText(QString::fromStdString(filename));
  if (!KernelWrapper::GetInstance().IsFileWritable ( m_filename )) {
    QPalette pal;
    pal.setColor(QPalette::Active, QPalette::WindowText,SchemaStyle::get_color("foreground", "readonly"));
    m_ui->label->setPalette(pal);
//    m_ui->label->setStyleSheet("color:rgb(128,0,0);");
  }
  show_status();


  QString DUNEDAQ_DB_PATH = getenv ( "DUNEDAQ_DB_PATH" );
  m_path_list = DUNEDAQ_DB_PATH.split (QLatin1Char(':'), Qt::SkipEmptyParts );
  for ( QString & path : m_path_list ) {
    if ( !path.endsWith ( "/" ) ) {
      path.append ( "/" );
    }
    m_path_urls.append(QUrl::fromLocalFile(path));
  }

  get_includes();

  m_ui->textBrowser->hide();

  update_class_list();

  m_ui->include_list->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  m_ui->class_list->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  connect (m_ui->include_list, SIGNAL (itemActivated(QListWidgetItem*)),
           this, SLOT (show_file_info(QListWidgetItem*)) );
  connect (m_ui->include_list, SIGNAL (customContextMenuRequested(QPoint)),
           this, SLOT (activate_include_context_menu(QPoint)));


  if (KernelWrapper::GetInstance().IsFileWritable ( m_filename )) {
    connect (m_ui->active_button, SIGNAL(pressed()), this, SLOT(set_active()));
    connect (m_ui->add_button, SIGNAL(pressed()), this, SLOT(add_include()));
    connect (m_ui->missing_button, SIGNAL (pressed()), this, SLOT(add_missing_includes()));
    connect (&KernelWrapper::GetInstance(), SIGNAL (ClassUpdated(QString)),
             this, SLOT(class_updated(QString)));
    connect (m_ui->class_list, SIGNAL (customContextMenuRequested(QPoint)),
             this, SLOT (activate_class_context_menu(QPoint)));
  }
  else {
    m_ui->add_button->setEnabled(false);
    m_ui->active_button->setEnabled(false);
    m_ui->save_button->setEnabled(false);
  }
  connect (m_ui->save_button, SIGNAL(pressed()), this, SLOT(save_schema()));
  connect (m_ui->close_button, SIGNAL(pressed()), this, SLOT(close()));
  connect (&KernelWrapper::GetInstance(), SIGNAL (active_updated()),
           this, SLOT(show_status()));

  connect (m_ui->class_list, SIGNAL (itemActivated(QListWidgetItem*)),
           this, SLOT (launch_class_editor(QListWidgetItem*)));
}

void SchemaFileInfo::get_includes() {
  m_ui->include_list->clear();
  m_all_includes.clear();
  KernelWrapper::GetInstance().get_all_includes(m_filename, m_all_includes);

  std::set<std::string> direct_includes;
  KernelWrapper::GetInstance().get_direct_includes(m_filename, direct_includes);

  for (auto inc: m_all_includes) {
    auto file = prune_path(inc);
    auto item = new QListWidgetItem(QString::fromStdString(file));
    item->setToolTip(QString::fromStdString(inc));
    m_ui->include_list->addItem(item);
    if (!direct_includes.contains(file)) {
      item->setForeground(QBrush(SchemaStyle::get_color("foreground", "inherited")));
    }
    else if (!KernelWrapper::GetInstance().IsFileWritable (inc)) {
      item->setForeground(QBrush(SchemaStyle::get_color("foreground", "readonly")));
      item->setBackground(QBrush(SchemaStyle::get_color("background", "readonly")));
    }
    else {
      item->setForeground(QBrush(SchemaStyle::get_color("foreground", "default")));
      item->setBackground(QBrush(SchemaStyle::get_color("background", "default")));
    }
  }
  m_ui->include_list->update();
}

bool SchemaFileInfo::check_relationships(dunedaq::oks::OksClass* cls) {
  bool ok = true;
  auto relationships = cls->direct_relationships();
  if (relationships != nullptr) {
    for (auto rel: *relationships) {
      auto rel_class = rel->get_class_type();
      if (rel_class == nullptr) {
        QString warning = "<b>Warning</b> class <i>"
          + QString::fromStdString(cls->get_name())
          + "</i> has relationship "
          + QString::fromStdString(rel->get_name())
          + " referring to class <i>"
          + QString::fromStdString(rel->get_type())
          + "</i> which is not loaded<br>";
        m_ui->textBrowser->insertHtml(warning);
        m_ui->textBrowser->show();
        ok = false;
        continue;
      }
      auto file = rel_class->get_file()->get_full_file_name();
      if (file != m_filename && !m_all_includes.contains(file)) {
        m_missing_includes.insert(file);
        QString warning = "<b>Warning</b> class <i>"
          + QString::fromStdString(cls->get_name())
          + "</i> has relationship "
          + QString::fromStdString(rel->get_name())
          + " referring to class <i>"
          + QString::fromStdString(rel->get_class_type()->get_name())
          + "</i> from " + QString::fromStdString(file)
          + " which is not included<br>";
        m_ui->textBrowser->insertHtml(warning);
        m_ui->textBrowser->show();
        ok = false;
      }
    }
  }
  return ok;
}

bool SchemaFileInfo::check_superclasses(dunedaq::oks::OksClass* cls) {
  bool ok = true;
  auto super_classes = cls->direct_super_classes();
  if (super_classes != nullptr) {
    for (auto sc: *super_classes) {
      auto sclass = KernelWrapper::GetInstance().FindClass(*sc);
      if (sclass == nullptr) {
        QString warning = "<b>Warning</b> class <i>"
          + QString::fromStdString(cls->get_name())
          + "</i> refers to super class <i>" + QString::fromStdString(*sc)
          + "</i> which is not known<br>";
        m_ui->textBrowser->insertHtml(warning);
        m_ui->textBrowser->show();
        ok = false;
        continue;
      }
      auto file = sclass->get_file()->get_full_file_name();
      if (file != m_filename && !m_all_includes.contains(file)) {
        m_missing_includes.insert(file);
        QString warning = "<b>Warning</b> class <i>"
          + QString::fromStdString(cls->get_name())
          + "</i> refers to super class <i>" + QString::fromStdString(*sc)
          + "</i> from " + QString::fromStdString(file)
          + " which is not included<br>";
        m_ui->textBrowser->insertHtml(warning);
        m_ui->textBrowser->show();
        ok = false;
      }
    }
  }
  return ok;
}

void SchemaFileInfo::remove_include(std::string filename) {
  std::cout << "Removing include file " << filename << "\n";

  auto temp_files = m_all_includes;
  temp_files.erase(filename);
  temp_files.insert(m_filename);

  auto classes = KernelWrapper::GetInstance().get_schema_classes(m_filename);
  for (auto cls: classes) {
    // Ignore classes in file we're removing
    if (cls->get_file()->get_full_file_name() != filename) {
      auto relationships = cls->direct_relationships();
      if (relationships != nullptr) {
        for (auto rel: *relationships) {
          auto file = rel->get_class_type()->get_file()->get_full_file_name();
          if (!temp_files.contains(file)) {
            auto message = QString(
              "Cannot remove %1 as class %2 has relationship to its %3 class").arg(
                QString::fromStdString(filename)).arg(
                  QString::fromStdString(cls->get_name())).arg(
                    QString::fromStdString(rel->get_class_type()->get_name()));
            QMessageBox::warning ( 0, "Schema editor", message );
            return;
          }
        }
      }
      auto super_classes = cls->direct_super_classes();
      if (super_classes != nullptr) {
        for (auto sc: *super_classes) {
          auto sclass = KernelWrapper::GetInstance().FindClass(*sc);
          auto file = sclass->get_file()->get_full_file_name();
          if (!temp_files.contains(file)) {
            auto message = QString(
              "Cannot remove %1 as class %2 has superclass %3 in %1").arg(
                QString::fromStdString(filename)).arg(
                  QString::fromStdString(cls->get_name())).arg(
                    QString::fromStdString(*sc));
            QMessageBox::warning ( 0, "Schema editor", message );
            return;
          }
        }
      }
    }
  }
  try {
    dbse::KernelWrapper::GetInstance().RemoveInclude(m_filename,
                                                     prune_path(filename));
    m_all_includes.erase(filename);
  }
  catch (dunedaq::oks::FailedRemoveInclude& exc) {
    QMessageBox::warning (0, "Schema editor", QString::fromStdString(exc.what()));
  }

  get_includes();
  update_class_list();
  show_status();
}

void dbse::SchemaFileInfo::remove_include() {
  auto item = m_ui->include_list->currentItem();
  std::string fn = item->text().toStdString();
  remove_include (fn);
}


void SchemaFileInfo::accept() {
  close();
}

void SchemaFileInfo::reject() {
  close();
}

void dbse::SchemaFileInfo::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}

void SchemaFileInfo::launch_class_editor(QListWidgetItem* item) {
  SchemaClassEditor::launch(item->text());
}

void SchemaFileInfo::update_class_list() {
  m_ui->class_list->clear();
  m_ui->textBrowser->clear();
  auto classes = KernelWrapper::GetInstance().get_schema_classes(m_filename);
  for (auto cls: classes) {
    auto item = new QListWidgetItem(QString::fromStdString(cls->get_name()));

    if (!KernelWrapper::GetInstance().IsFileWritable ( m_filename )) {
      item->setForeground(QBrush(SchemaStyle::get_color("foreground", "readonly")));
      item->setBackground(QBrush(SchemaStyle::get_color("background", "readonly")));
    }
    if (!check_relationships(cls)) {
      item->setForeground(QBrush(SchemaStyle::get_color("foreground", "error")));
      item->setBackground(QBrush(SchemaStyle::get_color("background", "error")));
    }
    if (!check_superclasses(cls)) {
      item->setForeground(QBrush(SchemaStyle::get_color("foreground", "error")));
      item->setBackground(QBrush(SchemaStyle::get_color("background", "error")));
    }

    m_ui->class_list->addItem(item);
  }
  m_ui->class_summary->setText (
    QString("Total of %1 classes").arg(classes.size()));

  // if (!m_missing_includes.empty() && m_missing_button == nullptr) {
  //   m_missing_button = m_ui->buttonBox->addButton("Add missing includes", QDialogButtonBox::ApplyRole);
  //   if (m_missing_button != nullptr) {
  //     connect ( m_missing_button, SIGNAL (pressed()), this, SLOT(add_missing_includes()));
  //   }
  //   else {
  //     std::cout << "Failed to add button\n";
  //   }
  // }
  if (m_missing_includes.empty()) {
    m_ui->missing_button->setEnabled(false);
    m_ui->missing_button->hide();
  }
  else {
    m_ui->missing_button->setEnabled(true);
    m_ui->missing_button->show();
  }
}

void SchemaFileInfo::class_updated(QString /*class_name*/) {
  // auto cls = KernelWrapper::GetInstance().FindClass(class_name.toStdString());
  // auto file = cls->get_file()->get_full_file_name();
  // if (file == m_filename) {
    show_status();
    update_class_list();
  // }
}

void SchemaFileInfo::save_schema() {
  try {
    KernelWrapper::GetInstance().SaveSchema (m_filename);
  }
  catch (const dunedaq::oks::exception& exc) {
    QMessageBox::warning(0,
                         "Save Schema",
                         QString("Failed to save file %1" )
                         .arg (m_filename.c_str())
                         .append(QString(exc.what())),
                         QMessageBox::Ok);
  }
  show_status();
  emit files_updated();
}

void SchemaFileInfo::show_status() {
  QString status;
  if (KernelWrapper::GetInstance().IsFileWritable ( m_filename )) {
    status.append("Read/Write");
  }
  else {
    status.append("Read only");
  }
  if (KernelWrapper::GetInstance().is_file_modified ( m_filename )) {
    status.append("  Modified");
    if (m_missing_includes.empty()) {
      m_ui->save_button->setEnabled(true);
    }
    else {
      m_ui->save_button->setEnabled(false);
    }
  }
  else {
    m_ui->save_button->setEnabled(false);
  }

  if (m_filename == KernelWrapper::GetInstance().GetActiveSchema()) {
    status.append("  Active");
    m_ui->active_button->setEnabled(false);
  }
  else if (KernelWrapper::GetInstance().IsFileWritable (m_filename)) {
    m_ui->active_button->setEnabled(true);
  }
  m_ui->status->setText(status);
}

void SchemaFileInfo::add_include() {
  auto fd = new QFileDialog ( this, tr ( "Open File" ), ".",
                              tr ( "XML schema files (*.schema.xml)" ) );
  fd->setFileMode ( QFileDialog::ExistingFiles );
  fd->setViewMode ( QFileDialog::Detail );
  fd->setAcceptMode ( QFileDialog::AcceptOpen );
  fd->setSidebarUrls(m_path_urls);
  fd->exec();
  auto files = fd->selectedFiles();
  for (auto file: files) {
    add_file(file.toStdString());
  }
  get_includes();
  update_class_list();
  show_status();
}

void SchemaFileInfo::add_missing_includes() {
  std::cout << "\nAdding missing include files:\n";

  for (auto file: m_missing_includes) {
    add_file(file);
  }
  m_missing_includes.clear();

  get_includes();
  m_ui->textBrowser->clear();
  m_ui->textBrowser->hide();

  get_includes();
  update_class_list();
  show_status();
}

std::string SchemaFileInfo::prune_path(std::string file) {
  for ( QString & element : m_path_list ) {
    if (file.starts_with(element.toStdString())) {
      file = file.substr (element.size());
      break;
    }
  }
  return file;
}
  void dbse::SchemaFileInfo::add_file(std::string file) {
  std::cout << "   " << file << "\n";
  try {
    KernelWrapper::GetInstance().AddInclude(m_filename, prune_path(file));
    emit files_updated();
  }
  catch (std::exception& exc) {
    QString message = QString(
      "Failed to add %1 to included files, %2" ).arg(
        QString::fromStdString(file)).arg(exc.what());
    QMessageBox::warning ( 0, "Schema editor", message );
  }
}

void dbse::SchemaFileInfo::activate_include_context_menu (QPoint pos)
{
  if (m_include_menu == nullptr) {
    m_include_menu = new QMenu (this);

    if (KernelWrapper::GetInstance().IsFileWritable ( m_filename )) {
      QAction* add = new QAction ( tr ( "Add New Include File" ), this );
      connect (add, SIGNAL ( triggered() ), this, SLOT ( add_include() ) );
      QAction* remove = new QAction ( tr ( "Remove Selected Include File" ), this );
      connect (remove, SIGNAL ( triggered() ), this, SLOT ( remove_include() ) );
      m_include_menu->addAction ( add );
      m_include_menu->addAction ( remove );
    }
    QAction* info = new QAction ( tr ( "Show file info" ), this );
    connect (info, SIGNAL ( triggered() ), this, SLOT ( show_file_info() ) );

    m_include_menu->addAction ( info );
  }

  if (m_ui->include_list->currentIndex().isValid()) {
    m_include_menu->exec (m_ui->include_list->mapToGlobal(pos));
  }
}

void dbse::SchemaFileInfo::activate_class_context_menu (QPoint pos)
{
  if (m_class_menu == nullptr) {
    m_class_menu = new QMenu (this);

    QAction * add = new QAction (tr("&Add New Class"), this );
    connect (add, SIGNAL (triggered()), this, SLOT (add_new_class()));

    QAction* remove = new QAction(tr("&Remove Selected Class"), this );
    connect (remove, SIGNAL(triggered()), this, SLOT(remove_class()));

    QAction* edit = new QAction(tr( "&Edit Selected Class"), this );
    connect (edit, SIGNAL (triggered()), this, SLOT (edit_class()));

    QAction* move = new QAction(tr( "&Move Selected Class"), this );
    connect (move, SIGNAL (triggered()), this, SLOT (move_class()));

    m_class_menu->addAction (add);
    m_class_menu->addAction (edit);
    m_class_menu->addAction (move);
    m_class_menu->addAction (remove);
  }

  if (m_ui->class_list->currentIndex().isValid()) {
    m_class_menu->exec (m_ui->class_list->mapToGlobal(pos));
  }
}

void dbse::SchemaFileInfo::edit_class()
{
  SchemaClassEditor::launch(m_ui->class_list->currentItem()->text());
}
void dbse::SchemaFileInfo::move_class()
{
  auto oks_class = KernelWrapper::GetInstance().FindClass(
    m_ui->class_list->currentItem()->text().toStdString());
  SchemaClassEditor::move_class(oks_class, this);
}
void dbse::SchemaFileInfo::remove_class()
{
  auto item = m_ui->class_list->currentItem();
  QString cn = item->text();
  dunedaq::oks::OksClass* schema_class =
    KernelWrapper::GetInstance().FindClass(cn.toStdString());
  if (schema_class->all_sub_classes()->size() != 0) {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Cannot delete class because it has sub-classes." ) );
    return;
  }
  else if (KernelWrapper::GetInstance().AnyClassReferenceThis(schema_class)) {
    QMessageBox::warning (0, "Schema editor",
                          QString ( "Cannot delete class because some other classes references it."));
    return;
  }

  KernelWrapper::GetInstance().PushRemoveClassCommand (schema_class,
                                                       schema_class->get_name(),
                                                       schema_class->get_description(),
                                                       schema_class->get_is_abstract() );
  delete item;
  m_ui->class_list->update();
}

bool dbse::SchemaFileInfo::set_active(std::string filename) {
  try {
    KernelWrapper::GetInstance().SetActiveSchema (filename);
  }
  catch (dunedaq::oks::CanNotSetActiveFile& exc) {
    auto qfn = QString::fromStdString(m_filename);
    auto text = QString("Could not make schema file %1 active!\n\n").arg(qfn);
    QMessageBox::warning(0,
                         "Set Active Schema",
                         text.append(QString(exc.what())),
                         QMessageBox::Ok);
    return false;
  }
  show_status();
  return true;
}


void dbse::SchemaFileInfo::set_schemafile_active() {
  auto item = m_ui->include_list->currentItem();
  std::string fn = item->text().toStdString();
  set_active (fn);
  show_status();
}

void dbse::SchemaFileInfo::set_active() {
  set_active (m_filename);
  show_status();
}


void dbse::SchemaFileInfo::add_new_class()
{
  if (set_active (m_filename)) {
    auto nc = SchemaClassEditor::createNewClass();
    if (!nc.isEmpty()) {
      auto item = new QListWidgetItem(nc);
      m_ui->class_list->addItem(item);
      m_ui->class_list->sortItems();
      m_ui->class_list->setCurrentItem(item);
      m_ui->class_list->update();
    }
  }
}



void dbse::SchemaFileInfo::show_file_info() {
  show_file_info(m_ui->include_list->currentItem());
}
void dbse::SchemaFileInfo::show_file_info(QListWidgetItem* item) {
  show_file_info(item->text());
}
void dbse::SchemaFileInfo::show_file_info(QString fn) {
  bool widget_found = false;
  for ( QWidget * widget : QApplication::allWidgets() ) {
    auto sfi = dynamic_cast<SchemaFileInfo *> ( widget );
    if ( sfi != nullptr ) {
      if ( (sfi->objectName() ).compare ( fn ) == 0 ) {
        sfi->raise();
        sfi->setVisible ( true );
        sfi->activateWindow();
        widget_found = true;
        break;
      }
    }
  }
  if ( !widget_found ) {
    auto info = new SchemaFileInfo(fn.toStdString());
    // connect (info, &SchemaFileInfo::files_updated,
    //          this, &SchemaMainWindow::update_models);
    emit new_window(this);
    info->show();
  }
}
} //namespace dbse
