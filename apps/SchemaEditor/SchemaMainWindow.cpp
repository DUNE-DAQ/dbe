/// Including Schema Editor
#include "dbe/SchemaMainWindow.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include "dbe/SchemaGraphicsScene.hpp"
#include "dbe/SchemaTab.hpp"
#include "dbe/SchemaClassEditor.hpp"
#include "dbe/SchemaRelationshipEditor.hpp"
#include "dbe/SchemaMethodImplementationEditor.hpp"
#include "dbe/SchemaIncludeFileWidget.hpp"

#include "oks/kernel.hpp"  // for CanNotSetActiveFile exception

/// Including Auto-Generated Files
#include "ui_SchemaMainWindow.h"
/// Including QT Headers
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QPushButton>
#include <QGraphicsScene>
#include "QInputDialog"
#include <QCloseEvent>
#include <QPrinter>
#include <QPrintDialog>
#include <QSvgGenerator>

//#include <format>
#include <sstream>

using namespace dunedaq;
using namespace dunedaq::oks;


dbse::SchemaMainWindow::SchemaMainWindow ( QString SchemaFile, QWidget * parent )
  : QMainWindow ( parent ),
    ui ( new Ui::SchemaMainWindow ),
    FileModel ( nullptr ),
    TableModel ( nullptr ),
    proxyModel ( new QSortFilterProxyModel() ),
    ContextMenuFileView ( nullptr ),
    ContextMenuTableView ( nullptr )
{
  InitialSettings();
  InitialTab();
  InitialTabCorner();
  SetController();
  setFocusPolicy( Qt::StrongFocus );

  OpenSchemaFile(SchemaFile);
}

dbse::SchemaMainWindow::~SchemaMainWindow() = default;

void dbse::SchemaMainWindow::InitialSettings()
{
  ui->setupUi ( this );
  setWindowTitle ( m_title );
  ui->UndoView->setStack ( KernelWrapper::GetInstance().GetUndoStack() );
  ui->ClassTableView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  ui->ClassTableView->setDragEnabled ( true );
  ui->ClassTableView->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  ui->FileView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  ui->FileView->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  ui->FileView->setSelectionBehavior ( QAbstractItemView::SelectRows );
  ui->TabWidget->setTabsClosable ( true );
  ui->ClassTableSearchLine->setProperty ( "placeholderText",
                                          QVariant ( QString ( "Type to search" ) ) );
}

void dbse::SchemaMainWindow::InitialTab()
{
  add_tab();
  ui->TabWidget->removeTab ( 0 );
}

void dbse::SchemaMainWindow::InitialTabCorner()
{
  QPushButton * RightButton = new QPushButton ( "+" );
  ui->TabWidget->setCornerWidget ( RightButton, Qt::TopLeftCorner );
  connect ( RightButton, SIGNAL ( clicked() ), this, SLOT ( add_tab() ) );
}

void dbse::SchemaMainWindow::SetController()
{
  connect ( ui->OpenFileSchema, SIGNAL ( triggered() ), this, SLOT ( OpenSchemaFile() ) );
  connect ( ui->CreateNewSchema, SIGNAL ( triggered() ), this, SLOT ( CreateNewSchema() ) );
  connect ( ui->AddInclude, SIGNAL ( triggered() ), this, SLOT ( LaunchIncludeEditorActiveSchema() ) );
  connect ( ui->SaveSchema, SIGNAL ( triggered() ), this, SLOT ( SaveSchema() ) );
  connect ( ui->SetRelationship, SIGNAL ( triggered ( bool ) ), this,
            SLOT ( ChangeCursorRelationship ( bool ) ) );
  connect ( ui->SetInheritance, SIGNAL ( triggered ( bool ) ), this,
            SLOT ( ChangeCursorInheritance ( bool ) ) );
  connect ( ui->AddClass, SIGNAL ( triggered() ), this, SLOT ( AddNewClass() ) );
  connect ( ui->SaveView, SIGNAL ( triggered() ), this, SLOT ( SaveView() ) );
  connect ( ui->SaveViewAs, SIGNAL ( triggered() ), this, SLOT ( SaveViewAs() ) );
  connect ( ui->LoadView, SIGNAL ( triggered() ), this, SLOT ( LoadView() ) );
  connect ( ui->NameView, SIGNAL ( triggered() ), this, SLOT ( NameView() ) );
  connect ( ui->Exit, SIGNAL ( triggered() ), this, SLOT ( close() ) );
  connect ( ui->ClassTableView, SIGNAL ( doubleClicked ( QModelIndex ) ), this,
            SLOT ( LaunchClassEditor ( QModelIndex ) ) );
  connect ( ui->close_tab, SIGNAL ( triggered() ), this, SLOT ( close_tab() ) );

  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassCreated( QString ) ), this,
            SLOT ( update_models() ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassUpdated ( QString ) ), this,
            SLOT ( update_models() ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassRemoved ( QString ) ), this,
            SLOT ( update_models() ) );
  connect ( ui->TabWidget, SIGNAL ( tabCloseRequested ( int ) ), this,
            SLOT ( RemoveTab ( int ) ) );
  connect ( ui->FileView, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
            SLOT ( CustomContextMenuFileView ( QPoint ) ) );
  connect ( ui->ClassTableView, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
            SLOT ( CustomContextMenuTableView ( QPoint ) ) );
  connect ( ui->PrintView, SIGNAL ( triggered() ), this, SLOT ( PrintCurrentView() ) );
  connect ( ui->exportView, SIGNAL ( triggered() ), this, SLOT ( export_current_view() ) );
  connect ( ui->ClassTableSearchLine, SIGNAL( textChanged ( QString ) ), proxyModel, SLOT( setFilterRegExp( QString ) ) );

}

void dbse::SchemaMainWindow::LaunchIncludeEditor()
{
  QModelIndex Index = ui->FileView->currentIndex();
  QStringList Row = FileModel->getRowFromIndex ( Index );
  auto * file_widget = new dbse::SchemaIncludeFileWidget ( Row.at ( 0 ) );
  connect (file_widget, &SchemaIncludeFileWidget::files_updated,
           this, &SchemaMainWindow::update_models);
  file_widget->show();
}

void dbse::SchemaMainWindow::LaunchIncludeEditorActiveSchema()
{
  std::string ActiveSchema = KernelWrapper::GetInstance().GetActiveSchema();

  auto file_widget = new dbse::SchemaIncludeFileWidget ( QString::fromStdString (ActiveSchema) );
  connect (file_widget, &SchemaIncludeFileWidget::files_updated,
           this, &SchemaMainWindow::update_models);
  file_widget->show();
}

void dbse::SchemaMainWindow::BuildFileModel()
{
  QStringList Headers { "File Name", "Access", "Status" };

  if ( FileModel != nullptr )
  {
    delete FileModel;
  }
  FileModel = new CustomFileModel ( Headers );

  ui->FileView->setModel ( FileModel );
  ui->FileView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  ui->FileView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  ui->FileView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}

void dbse::SchemaMainWindow::BuildTableModel()
{
  QStringList Headers
  { "Class Name" };

  if ( TableModel == nullptr )
  {
    TableModel = new CustomTableModel ( Headers );
  }
  else
  {
    delete TableModel;
    TableModel = new CustomTableModel ( Headers );
  }

  proxyModel->setSourceModel(TableModel);
  ui->ClassTableView->setModel ( proxyModel );
}

int dbse::SchemaMainWindow::ShouldSaveViewChanges() const
{
  QString modified_views;
  for (int index=0; index<ui->TabWidget->count(); ++index) {
    auto tab = dynamic_cast<SchemaTab *> (ui->TabWidget->widget(index));
    if (tab->GetScene()->IsModified()) {
      auto label = ui->TabWidget->tabText(index);
      label = label.remove('*');
      modified_views.append("  " + label + "\n");
    }
  }
  if (!modified_views.isEmpty()) {
    QString message ( "There are unsaved changes in the schema views:\n");
    message.append (modified_views);
    message.append ("Do you want to save the changes in the schema views?\n" );
    return QMessageBox::question (
      0, tr ( "SchemaEditor" ),
      message,
      QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );
  }

  return QMessageBox::Discard;
}

int dbse::SchemaMainWindow::ShouldSaveChanges() const
{
  // if ( KernelWrapper::GetInstance().GetUndoStack()->isClean() )
  auto modified = KernelWrapper::GetInstance().ModifiedSchemaFiles();
  if (modified.empty())
  {
    return QMessageBox::Discard;
  }

  std::string msg = "There are unsaved changes in the following files:\n\n"
    + modified + "Do you want to save the changes in the schema?\n";
  return QMessageBox::question (
           0, tr ( "SchemaEditor" ),
           QString ( msg.c_str() ),
           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save );
}

void dbse::SchemaMainWindow::AddNewClass()
{
    SchemaClassEditor::createNewClass();
    BuildFileModel();
}

void dbse::SchemaMainWindow::RemoveClass()
{
  QModelIndex Index = ui->ClassTableView->currentIndex();
  QModelIndex proxyIndex = proxyModel->mapToSource( Index );
  QStringList Row = TableModel->getRowFromIndex ( proxyIndex );
  OksClass * SchemaClass = KernelWrapper::GetInstance().FindClass ( Row.at (
                                                                      0 ).toStdString() );

  if ( SchemaClass->all_sub_classes()->size() != 0 )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Cannot delete class because it has sub-classes." ) );
    return;
  }
  else if ( KernelWrapper::GetInstance().AnyClassReferenceThis ( SchemaClass ) )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Cannot delete class because some classes references it." ) );
    return;
  }
  else
  {
    QString Name = QString::fromStdString ( SchemaClass->get_name() );
    KernelWrapper::GetInstance().PushRemoveClassCommand ( SchemaClass, SchemaClass->get_name(),
                                                          SchemaClass->get_description(),
                                                          SchemaClass->get_is_abstract() );
    BuildFileModel();
  }
}

void dbse::SchemaMainWindow::editClass() {
  QModelIndex Index = ui->ClassTableView->currentIndex();
  QModelIndex proxyIndex = proxyModel->mapToSource( Index );
  QStringList Row = TableModel->getRowFromIndex ( proxyIndex );

  if ( !Row.isEmpty() ) {
    bool widget_found = false;
    QString class_name = Row.at ( 0 );
    OksClass * class_info = KernelWrapper::GetInstance().FindClass (
      class_name.toStdString() );

    for ( QWidget * widget : QApplication::allWidgets() ) {
      auto editor = dynamic_cast<SchemaClassEditor *> ( widget );
      if ( editor != nullptr ) {
        if ( ( editor->objectName() ).compare ( class_name ) == 0 ) {
          editor->raise();
          editor->setVisible ( true );
          editor->activateWindow();
          widget_found = true;
        }
      }
    }
    if ( !widget_found ) {
      SchemaClassEditor * editor = new SchemaClassEditor ( class_info );
      editor->show();
    }
  }
}

void dbse::SchemaMainWindow::SetSchemaFileActive()
{
  QModelIndex index = ui->FileView->currentIndex();
  QString file = FileModel->getRowFromIndex ( index ).at(0);
  try {
    KernelWrapper::GetInstance().SetActiveSchema ( file.toStdString() );
  }
  catch (oks::CanNotSetActiveFile& exc) {
    QMessageBox::warning(0,
                         "Set Active Schema",
                         QString("Could not make schema active!\n\n").append(QString(exc.what())),
                         QMessageBox::Ok);
    return;
  }
  update_window_title(file);

  // In case we are highlighting classes in the active file, redraw current
  // view tab now we've changed active file
  SchemaTab * current_tab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );
  SchemaGraphicsScene * scene = current_tab->GetScene();
  scene->update();

  BuildFileModel();
}
void dbse::SchemaMainWindow::SaveSchemaFile()
{
  QModelIndex Index = ui->FileView->currentIndex();
  const auto File = FileModel->getRowFromIndex ( Index ).at ( 0 );
  QString message;
  try {
    KernelWrapper::GetInstance().SaveSchema ( File.toStdString() );
    message = QString ( "File %1 saved" ).arg ( File );
  }
  catch (const oks::exception& exc) {
    message = QString ( "Faled to save file %1" ).arg ( File );
  }
  ui->StatusBar->showMessage( message );
  BuildFileModel();
}

void dbse::SchemaMainWindow::PrintCurrentView()
{
  SchemaTab * CurrentTab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );

  QPrinter printer;

  if ( QPrintDialog ( &printer ).exec() == QDialog::Accepted )
  {
    QPainter painter ( &printer );
    painter.setRenderHint ( QPainter::Antialiasing );

    SchemaGraphicsScene * Scene = CurrentTab->GetScene();
    QGraphicsView * View = CurrentTab->GetView();
    Scene->render ( &painter, QRectF(), View->viewport()->rect() );
  }
}
void dbse::SchemaMainWindow::export_current_view(){
  auto tab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );

  auto file = QFileDialog::getSaveFileName(
    this, tr("Export to SVG"),
    m_export_path,
    tr("SVG files (*.svg);;All files (*)"));
  if (file.isEmpty()) {
    return;
  }

  auto spos = file.lastIndexOf('/');
  if (spos != -1) {
    m_export_path = file;
    m_export_path.truncate(spos);
  }

  auto scene = tab->GetScene();
  auto view = tab->GetView();

  QSvgGenerator generator;
  generator.setFileName(file);
  auto vpr=view->viewport()->rect();
  generator.setSize(QSize(vpr.width(),vpr.height()));
  generator.setViewBox(vpr);

  QPainter painter;
  painter.begin(&generator);
  scene->render ( &painter, QRectF(), view->viewport()->rect() );
  painter.end();
}

void dbse::SchemaMainWindow::closeEvent ( QCloseEvent * event )
{
  int UserChoice = ShouldSaveChanges();

  if ( UserChoice == QMessageBox::Save )
  {
    SaveModifiedSchema();
  }
  else if ( UserChoice == QMessageBox::Cancel )
  {
    event->ignore();
    return;
  }

  UserChoice = ShouldSaveViewChanges();
  if ( UserChoice == QMessageBox::Cancel )
  {
    event->ignore();
    return;
  }

  KernelWrapper::GetInstance().CloseAllSchema();

  for ( QWidget * Widget : QApplication::allWidgets() )
  {
    Widget->close();
  }

  event->accept();
}

void dbse::SchemaMainWindow::update_models() {
  BuildFileModel();
  BuildTableModel();
}

void dbse::SchemaMainWindow::OpenSchemaFile(QString SchemaFile) {
  if(!SchemaFile.isEmpty()) {
    try {
      KernelWrapper::GetInstance().LoadSchema(SchemaFile.toStdString());

#ifdef QT_DEBUG
      /// KernelWrapper::GetInstance().ShowSchemaClasses();
#endif
    }
    catch(oks::exception &Ex) {
      QMessageBox::warning(0,
                           "Load Schema",
                           QString("Could not load schema!\n\n").append(QString(Ex.what())),
                           QMessageBox::Ok);
    }
    try {
      KernelWrapper::GetInstance().SetActiveSchema(SchemaFile.toStdString());
    }
    catch (oks::CanNotSetActiveFile& exc) {
      QMessageBox::warning(0,
                           "Load Schema",
                           QString("Could not make schema active!\n\n").append(QString(exc.what())),
                           QMessageBox::Ok);
    }

    BuildTableModel();
    BuildFileModel();

    update_window_title(QFileInfo(SchemaFile).fileName());
    ui->CreateNewSchema->setDisabled (true );
    ui->OpenFileSchema->setDisabled (true );
  }
}

void dbse:: SchemaMainWindow::update_window_title(QString text) {
  m_title.remove(QRegularExpression(": --.*"));
  m_title.append(QString(": -- ") + text);
    setWindowTitle ( m_title );
}

void dbse::SchemaMainWindow::OpenSchemaFile()
{
  QFileDialog FileDialog ( this, tr ( "Open File" ), ".", tr ( "XML files (*.xml);;All files (*)" ) );
  FileDialog.setAcceptMode ( QFileDialog::AcceptOpen );
  FileDialog.setFileMode ( QFileDialog::AnyFile );
  FileDialog.setViewMode ( QFileDialog::Detail );
  QStringList FilesSelected;
  QString SchemaPath;

  if ( FileDialog.exec() )
  {
    FilesSelected = FileDialog.selectedFiles();
  }

  if ( FilesSelected.size() )
  {
    SchemaPath = FilesSelected.value ( 0 );
  }

  OpenSchemaFile(SchemaPath);
}
void dbse::SchemaMainWindow::SaveModifiedSchema()
{
   try
    {
      auto saved = KernelWrapper::GetInstance().SaveModifiedSchema();
      //std::format msg("{} schema files successfully saved", nsaved)
      std::ostringstream ostream;
      ostream << "Schema files:\n" + saved + " successfully saved";
      std::string msg = ostream.str();
      QMessageBox::information ( 0, "Schema editor",
                                 QString ( msg.c_str() ) );

    }
    catch ( oks::exception & Ex )
    {
      QMessageBox::warning ( 0, "Schema editor",
                             QString ( "Could not save schemas.\n\n%1" ).arg ( QString ( Ex.what() ) ) );
    }
 }

void dbse::SchemaMainWindow::SaveSchema()
{
  auto modified = KernelWrapper::GetInstance().ModifiedSchemaFiles();
  if (modified.empty())
  {
      QMessageBox::information ( 0, "Schema editor",
                                 QString ( "No modified schema files need saving" ) );
      
  }
  else {
    SaveModifiedSchema();
  }
  BuildFileModel();
}

void dbse::SchemaMainWindow::CreateNewSchema()
{
  QString FileName = QFileDialog::getSaveFileName ( this, tr ( "New schema File" ) );

  if ( FileName.isEmpty() )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Please provide a name for the schema !" ) );
    return;
  }

  if ( !FileName.endsWith ( ".schema.xml" ) )
  {
    FileName.append ( ".schema.xml" );
  }

  QFile FileInfo ( FileName );
  std::string FileNameStd = FileInfo.fileName().toStdString();

  try
  {
    KernelWrapper::GetInstance().CreateNewSchema ( FileNameStd );
    KernelWrapper::GetInstance().SaveSchema ( FileNameStd );
    BuildTableModel();
    BuildFileModel();
    ui->CreateNewSchema->setDisabled (true );
    ui->OpenFileSchema->setDisabled (true );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning (
      0,
      "Schema editor",
      QString ( "Could not create file : %1.\n\n%2" ).arg ( QString::fromStdString (
                                                              FileNameStd ) ).arg (
        QString ( Ex.what() ) ) );
  }
}

void dbse::SchemaMainWindow::ChangeCursorRelationship ( bool State )
{
  if ( ui->SetInheritance->isChecked() )
  {
    ui->SetInheritance->setChecked ( false );
  }

  if ( State )
  {
    ui->TabWidget->setCursor ( Qt::CrossCursor );
  }
  else
  {
    ui->TabWidget->setCursor ( Qt::ArrowCursor );
  }

  KernelWrapper::GetInstance().SetInheritanceMode ( false );
}

void dbse::SchemaMainWindow::ChangeCursorInheritance ( bool State )
{
  if ( ui->SetRelationship->isChecked() )
  {
    ui->SetRelationship->setChecked ( false );
  }

  if ( State )
  {
    ui->TabWidget->setCursor ( Qt::CrossCursor );
  }
  else
  {
    ui->TabWidget->setCursor ( Qt::ArrowCursor );
  }

  KernelWrapper::GetInstance().SetInheritanceMode ( true );
}

void dbse::SchemaMainWindow::add_tab()
{
  auto index = ui->TabWidget->addTab ( new SchemaTab(), "unnamed Schema View" );
  ui->TabWidget->setCurrentIndex ( index );
  auto  tab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );
  connect (tab->GetScene(), &SchemaGraphicsScene::sceneModified,
           this, &dbse::SchemaMainWindow::modifiedView);
}

void dbse::SchemaMainWindow::modifiedView(bool modified) {
  auto index = ui->TabWidget->currentIndex();
  auto label = ui->TabWidget->tabText(index);
  if (modified && !label.endsWith("*")) {
    label += "*";
  }
  if (!modified && label.endsWith("*")) {
    label = label.remove('*');
  }
  ui->TabWidget->setTabText(index, label);
}

void dbse::SchemaMainWindow::NameView() {
  auto index = ui->TabWidget->currentIndex();
  bool ok;
  QString text = QInputDialog::getText(nullptr,
                                       "Schema editor: rename class view",
                                       "New view name:",
                                       QLineEdit::Normal,
                                       "New Class View",
                                       &ok);
  if(ok && !text.isEmpty()) {
    ui->TabWidget->setTabText(index, text);
    SchemaTab * CurrentTab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );
    CurrentTab->setName(text);
  }

  auto newtext = ui->TabWidget->tabText(index);
}

void dbse::SchemaMainWindow::SaveView() {
  SchemaTab * tab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );
  if ( tab->GetScene()->items().size() != 0 )
  {
    auto file_name = tab->getFileName();
    if (file_name == "./") {
      SaveViewAs();
      return;
    }
    write_view_file(file_name, tab);
  }
}
void dbse::SchemaMainWindow::SaveViewAs() {
  SchemaTab * CurrentTab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );
  if ( CurrentTab->GetScene()->items().size() != 0 )
  {
    auto defName = CurrentTab->getFileName();
    if (defName == "./") {
      defName = m_view_dir;
    }
    QString FileName = QFileDialog::getSaveFileName (
      this, tr ( "Save View" ), defName,
      tr("View files (*.view);;All files (*)") );

    if (! FileName.isEmpty()) {
      auto spos = FileName.lastIndexOf('/');
      if (spos != -1) {
        m_view_dir = FileName;
        m_view_dir.truncate(spos);
      }

      if ( !FileName.endsWith ( ".view" ) ) {
        FileName.append ( ".view" );
      }

      CurrentTab->setFileName ( FileName );
      auto text = QFileInfo(FileName).baseName();
      CurrentTab->setName(text);
      auto index = ui->TabWidget->currentIndex();
      ui->TabWidget->setTabText(index, text);

      write_view_file(FileName, CurrentTab);
    }
  }
}

void dbse::SchemaMainWindow::write_view_file (const QString& file_name,
                                              SchemaTab* tab) {
  QFile file ( file_name );
  file.open ( QIODevice::WriteOnly );

  for ( QGraphicsItem * item : tab->GetScene()->items() ) {
    auto object = dynamic_cast<SchemaGraphicObject *> ( item );
    if ( object != nullptr ) {
      QString description = object->GetClassName() + ","
        + QString::number ( object->scenePos().x() ) + ","
        + QString::number ( object->scenePos().y() ) + "\n";
      file.write ( description.toUtf8() );
    }
    else {
      auto note = dynamic_cast<SchemaGraphicNote*> (item);
      if ( note != nullptr && !note->text().isEmpty()) {
        auto text = note->text().replace("\n", "<br>");
        text = text.replace(",", "<comma>");
        QString line = "#,"
          + QString::number ( note->scenePos().x() ) + ","
          + QString::number ( note->scenePos().y() ) + ","
          + text + "\n";
        file.write ( line.toUtf8() );
      }
    }
  }

  file.close();
  auto message = QString("Saved view to %1").arg(file_name);
  ui->StatusBar->showMessage( message );
  tab->GetScene()->ClearModified();
}

void dbse::SchemaMainWindow::LoadView() {
  QString ViewPath = QFileDialog::getOpenFileName (
    this,
    tr ("Open view file"),
    m_view_dir,
    tr("View files (*.view);;All files (*)"));

  if ( !ViewPath.isEmpty() )
  {
    auto spos = ViewPath.lastIndexOf('/');
    if (spos != -1) {
      m_view_dir = ViewPath;
      m_view_dir.truncate(spos);
    }
    QFile ViewFile ( ViewPath );
    ViewFile.open ( QIODevice::ReadOnly );

    auto text = QFileInfo(ViewPath).baseName();
    auto tab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );
    if (!tab->getName().isEmpty() || tab->GetScene()->IsModified()) {
      add_tab();
      tab = dynamic_cast<SchemaTab *> ( ui->TabWidget->currentWidget() );
    }
    auto index = ui->TabWidget->currentIndex();
    ui->TabWidget->setTabText(index, text);
    tab->setName(text);
    tab->setFileName(ViewPath);

    QStringList ClassesNames;
    QList<QPointF> Positions;
    QList<QPointF> note_positions;
    QStringList notes;
    while ( !ViewFile.atEnd() )
    {
      QString Line ( ViewFile.readLine() );
      if (!Line.isEmpty()) {
        QStringList ObjectDescription = Line.split ( "," );
        QPointF Position;
        Position.setX ( ObjectDescription.at ( 1 ).toInt() );
        Position.setY ( ObjectDescription.at ( 2 ).toInt() );
        if (ObjectDescription.at ( 0 ) == "#") {
          note_positions.append (Position);
          auto text = ObjectDescription.at(3);
          if (text.back() == '\n') {
            text.chop(1);
          }
          text = text.replace("<br>", "\n");
          text = text.replace("<comma>", ",");
          notes.append ( text );
        }
        else {
          ClassesNames.append ( ObjectDescription.at ( 0 ) );
          Positions.append ( Position );
        }
      }
    }
    ViewFile.close();

    auto message = QString("Loaded view from %1").arg(ViewPath);
    ui->StatusBar->showMessage( message );

    auto scene = tab->GetScene();
    scene->CleanItemMap();
    auto missing = scene->AddItemsToScene ( ClassesNames, Positions );
    if (!missing.empty()) {
      QString text{"The following classes in "};
      text.append(QFileInfo(ViewPath).fileName());
      text.append(" are not present in the loaded schema:\n  ");
      text.append(missing.join(",\n  "));
      QMessageBox::warning(this, tr("Load View"), text);
    }
    scene->ClearModified();

    scene->add_notes(notes, note_positions);
  }
}

void dbse::SchemaMainWindow::LaunchClassEditor ( QModelIndex Index )
{
  QModelIndex proxyIndex = proxyModel->mapToSource( Index );
  QStringList Row = TableModel->getRowFromIndex ( proxyIndex );

  if ( !Row.isEmpty() )
  {
    bool WidgetFound = false;
    QString ClassName = Row.at ( 0 );
    OksClass * ClassInfo = KernelWrapper::GetInstance().FindClass ( ClassName.toStdString() );

    for ( QWidget * Editor : QApplication::allWidgets() )
    {
      SchemaClassEditor * Widget = dynamic_cast<SchemaClassEditor *> ( Editor );

      if ( Widget != nullptr )
      {
        if ( ( Widget->objectName() ).compare ( ClassName ) == 0 )
        {
          Widget->raise();
          Widget->setVisible ( true );
          Widget->activateWindow();
          WidgetFound = true;
        }
      }
    }

    if ( !WidgetFound )
    {
      SchemaClassEditor * Editor = new SchemaClassEditor ( ClassInfo );
      Editor->show();
    }
  }
}

void dbse::SchemaMainWindow::close_tab() {
  RemoveTab(ui->TabWidget->currentIndex());
}

void dbse::SchemaMainWindow::RemoveTab ( int index )
{
  if ( index == -1 || ( ( ui->TabWidget->count() == 1 ) && index == 0 ) ) {
    return;
  }

  auto tab = dynamic_cast<SchemaTab *> (ui->TabWidget->widget(index));
  if (tab->GetScene()->IsModified()) {
    auto choice = QMessageBox::question (
      0, tr ( "SchemaEditor" ),
      QString ( "There are unsaved changes in the schema view:\n"
                "Do you really want to delete this schema view?\n" ),
      QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel );
    if (choice == QMessageBox::Cancel) {
      return;
    }
  }
  ui->TabWidget->removeTab ( index );
  delete tab;
}

void dbse::SchemaMainWindow::CustomContextMenuFileView ( QPoint Pos )
{
  if ( ContextMenuFileView == nullptr )
  {
    ContextMenuFileView = new QMenu ( this );

    QAction * Act = new QAction ( tr ( "Set as Active Schema" ), this );
    connect ( Act, SIGNAL ( triggered() ), this, SLOT ( SetSchemaFileActive() ) );
    QAction * Sav = new QAction ( tr ( "Save Schema File" ), this );
    connect ( Sav, SIGNAL ( triggered() ), this, SLOT ( SaveSchemaFile() ) );
    QAction * Inc = new QAction ( tr ( "Show/Update include file list" ), this );
    connect ( Inc, SIGNAL ( triggered() ), this, SLOT ( LaunchIncludeEditor() ) );

    ContextMenuFileView->addAction ( Act );
    ContextMenuFileView->addAction ( Inc );
    ContextMenuFileView->addAction ( Sav );
  }

  QModelIndex Index = ui->FileView->currentIndex();

  if ( Index.isValid() )
  {
    ContextMenuFileView->exec ( ui->FileView->mapToGlobal ( Pos ) );
  }
}

void dbse::SchemaMainWindow::CustomContextMenuTableView ( QPoint Pos )
{
  if ( ContextMenuTableView == nullptr )
  {
    ContextMenuTableView = new QMenu ( this );

    QAction * add = new QAction ( tr ( "&Add New Class" ), this );
    connect ( add, SIGNAL ( triggered() ), this, SLOT ( AddNewClass() ) );

    QAction * remove = new QAction ( tr ( "&Remove Selected Class" ), this );
    connect ( remove, SIGNAL ( triggered() ), this, SLOT ( RemoveClass() ) );

    QAction * edit = new QAction ( tr ( "&Edit Selected Class" ), this );
    connect ( edit, SIGNAL ( triggered() ), this, SLOT ( editClass() ) );

    ContextMenuTableView->addAction ( add );
    ContextMenuTableView->addAction ( edit );
    ContextMenuTableView->addAction ( remove );
  }

  QModelIndex Index = ui->ClassTableView->currentIndex();

  if ( Index.isValid() )
  {
    ContextMenuTableView->exec ( ui->ClassTableView->mapToGlobal ( Pos ) );
  }
}

