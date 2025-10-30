/// Including Qt
#include <QMessageBox>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QInputDialog>
#include <QString>
#include <QStringList>

/// Including Schema
#include "dbe/SchemaClassEditor.hpp"
#include "dbe/SchemaAttributeEditor.hpp"
#include "dbe/SchemaRelationshipEditor.hpp"
#include "dbe/SchemaMethodEditor.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
/// Including Ui file
#include "ui_SchemaClassEditor.h"

#include "oks/file.hpp"

#include <map>

using namespace dunedaq::oks;

dbse::SchemaClassEditor::SchemaClassEditor ( OksClass * ClassInfo, QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaClassEditor ),
    SchemaClass ( ClassInfo ),
    MethodModel ( nullptr ),
    AttributeModel ( nullptr ),
    RelationshipModel ( nullptr ),
    SuperClassModel ( nullptr ),
    SubClassModel ( nullptr ),
    ContextMenuAttribute ( nullptr ),
    ContextMenuRelationship ( nullptr ),
    ContextMenuMethod ( nullptr ),
    ContextMenuClass ( nullptr )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);

  /// Settings
  ui->setupUi ( this );
  setWindowTitle (
    QString ( "Class Editor : %1" ).arg ( QString::fromStdString (
                                            SchemaClass->get_name() ) ) );
  setObjectName ( QString::fromStdString ( SchemaClass->get_name() ) );
  /// Settings
  BuildModels();
  SetController();
  /// Editing Settings
  InitialSettings();
}

dbse::SchemaClassEditor::~SchemaClassEditor() = default;

void dbse::SchemaClassEditor::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}
void dbse::SchemaClassEditor::SetController()
{
  connect ( ui->buttonBox, SIGNAL ( accepted() ), this, SLOT ( ParseToSave() ) );
  connect ( ui->buttonBox, SIGNAL ( rejected() ), this, SLOT ( close_slot() ) );
  connect ( ui->moveButton, SIGNAL(clicked()), this, SLOT (move_class()));
  connect ( ui->AddButtonAttribute, SIGNAL ( clicked() ), this, SLOT ( AddNewAttribute() ) );
  connect ( ui->AddButtonSuperClass, SIGNAL ( clicked() ), this, SLOT ( AddNewSuperClass() ) );
  connect ( ui->AddButtonRelationship, SIGNAL ( clicked() ), this,
            SLOT ( AddNewRelationship() ) );
  connect ( ui->AddButtonMethod, SIGNAL ( clicked() ), this, SLOT ( AddNewMethod() ) );
  connect ( ui->RelationshipView, SIGNAL ( activated ( QModelIndex ) ), this,
            SLOT ( OpenRelationshipEditor ( QModelIndex ) ) );
  connect ( ui->MethodsView, SIGNAL ( activated ( QModelIndex ) ), this,
            SLOT ( OpenMethodEditor ( QModelIndex ) ) );
  connect ( ui->AttributeView, SIGNAL ( activated ( QModelIndex ) ), this,
            SLOT ( OpenAttributeEditor ( QModelIndex ) ) );
  connect ( ui->SuperClassView, SIGNAL ( activated ( QModelIndex ) ), this,
            SLOT ( OpenSuperClass ( QModelIndex ) ) );
  connect ( ui->SubClassView, SIGNAL ( activated ( QModelIndex ) ), this,
            SLOT ( OpenSubClass ( QModelIndex ) ) );
  connect ( ui->AttributeView, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
            SLOT ( CustomMenuAttributeView ( QPoint ) ) );
  connect ( ui->RelationshipView, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
            SLOT ( CustomMenuRelationshipView ( QPoint ) ) );
  connect ( ui->MethodsView, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
            SLOT ( CustomMenuMethodView ( QPoint ) ) );
  connect ( ui->SuperClassView, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
             SLOT ( CustomMenuClassView ( QPoint ) ) );
  connect ( ui->ShowDerivedAttributes, &QCheckBox::toggled, this, &SchemaClassEditor::BuildAttributeModelSlot );
  connect ( ui->ShowDerivedRelationships, &QCheckBox::toggled, this, &SchemaClassEditor::BuildRelationshipModelSlot );
  connect ( ui->ShowDerivedMethods, &QCheckBox::toggled, this, &SchemaClassEditor::BuildMethodModelSlot );
  connect ( ui->ShowAllSuperClasses, &QCheckBox::toggled, this, &SchemaClassEditor::BuildSuperClassModelSlot );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassRemoved ( QString ) ), this,
            SLOT ( ClassRemoved ( QString ) ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassUpdated ( QString ) ), this,
            SLOT ( ClassUpdated ( QString ) ) );
}

void dbse::SchemaClassEditor::ClassRemoved( QString className )
{
    if(className == objectName()) {
        QWidget::close();
    }
}

void dbse::SchemaClassEditor::ClassUpdated( QString className )
{
    if(className == objectName()) {
       BuildModels();
       InitialSettings();
    }
}

void dbse::SchemaClassEditor::BuildModels()
{
    BuildSubClassModelSlot();
    BuildSuperClassModelSlot();
    BuildMethodModelSlot();
    BuildAttributeModelSlot();
    BuildRelationshipModelSlot();
}

void dbse::SchemaClassEditor::InitialSettings()
{
  ui->AttributeView->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  ui->RelationshipView->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  ui->MethodsView->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  ui->SuperClassView->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );
  ui->SubClassView->setContextMenuPolicy ( Qt::ContextMenuPolicy::CustomContextMenu );

  /// Class Name
  ui->ClassNameLineEdit->setText ( QString::fromStdString ( SchemaClass->get_name() ) );
  //ui->ClassNameLineEdit->setEnabled ( false );
  /// Schema File
  ui->SchemaFileLineEdit->setText ( QString::fromStdString ( SchemaClass->get_file()->get_short_file_name() ) );
  //ui->SchemaFileLineEdit->setEnabled ( false );

  if (KernelWrapper::GetInstance().IsFileWritable(
        SchemaClass->get_file()->get_full_file_name())) {
    ui->AddButtonAttribute->setEnabled (true);
    ui->AddButtonSuperClass->setEnabled (true);
    ui->AddButtonRelationship->setEnabled (true);
    ui->AddButtonMethod->setEnabled (true);
    ui->moveButton->setEnabled (true);
  }
  else {
    ui->SchemaFileLineEdit->setStyleSheet("color:rgb(128,0,0);");
    // ui->DescriptionTextEdit->setEnabled(false);
    ui->DescriptionTextEdit->setReadOnly(true);

    ui->AddButtonAttribute->setEnabled (false);
    ui->AddButtonSuperClass->setEnabled (false);
    ui->AddButtonRelationship->setEnabled (false);
    ui->AddButtonMethod->setEnabled (false);
    ui->moveButton->setEnabled (false);
    ui->AbstractComboBox->setEnabled(false);
  }

  /// Description
  ui->DescriptionTextEdit->setPlainText ( QString::fromStdString ( SchemaClass->get_description() ) );
  ui->DescriptionTextEdit->setTabChangesFocus (true);
  /// Abstract
  if ( SchemaClass->get_is_abstract() )
  {
      ui->AbstractComboBox->setCurrentIndex ( 0 );
  }
  else
  {
      ui->AbstractComboBox->setCurrentIndex ( 1 );
  }
}

void dbse::SchemaClassEditor::move_class() {
  move_class(SchemaClass, this);
}


void dbse::SchemaClassEditor::move_class(dunedaq::oks::OksClass* schema_class,
  QWidget* pwidget) {
  std::string current_file = schema_class->get_file()->get_full_file_name();
  std::vector<OksFile*> files;
  KernelWrapper::GetInstance().GetSchemaFiles(files);
  QStringList writable_files;
  std::map<QString, OksFile*> file_map;
  int current_row = -1;
  for (auto file: files) {
    auto fn = file->get_full_file_name();
    if (KernelWrapper::GetInstance().IsFileWritable(fn)) {
      if (fn == current_file) {
        current_row = writable_files.size();
      }
      writable_files.append(QString::fromStdString(fn));
      file_map[QString::fromStdString(fn)] = file;
    }
  }
  if (writable_files.empty()) {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "No writable schema files to move class to." ) );
    return;
  }

  QWidget* widget = new QWidget();
  QString text = "Select file to hold class " +
    QString::fromStdString(schema_class->get_name());
  QLabel* label = new QLabel(text);
  QListWidget* qlw = new QListWidget();
  qlw->addItems(writable_files);
  if (current_row != -1) {
    qlw->setCurrentRow(current_row);
  }

  connect (qlw, &QListWidget::itemActivated,
           pwidget, [=] () {
             QListWidgetItem* it = qlw->currentItem();
             auto fn = it->text();
             if (fn.toStdString() != current_file) {
               schema_class->set_file(file_map.at(fn));
               emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(schema_class->get_name()) );
             }
             delete widget;
           }
    );

  auto bb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  connect (bb, &QDialogButtonBox::accepted, pwidget, [=] () {
    auto it = qlw->currentItem();
    if (it != nullptr) {
      auto fn = it->text();
      if (fn.toStdString() != current_file) {
        schema_class->set_file(file_map.at(fn));
        emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(schema_class->get_name()) );
      }
      delete widget;
    }
    else {
      QMessageBox::warning ( 0, "Schema editor",
                             QString ( "No schema file selected to move class to." ) );
      return;
    }
  });
  connect ( bb, &QDialogButtonBox::rejected, pwidget,  [=] () {delete widget;} );

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(label);
  layout->addWidget(qlw);
  layout->addWidget(bb);
  widget->setWindowTitle("Select new file for class");
  widget->setLayout(layout);
  widget->setParent(pwidget, Qt::Dialog);
  widget->show();
}

bool dbse::SchemaClassEditor::ShouldOpenAttributeEditor ( QString attrName )
{
  QString name = QString::fromStdString(SchemaClass->get_name() + "::") +
    attrName;
  for (QWidget* widget : QApplication::allWidgets())
  {
    if (dynamic_cast<SchemaAttributeEditor *> (widget) != nullptr)
    {
      if ( (widget->objectName()).compare (name) == 0 )
      {
        widget->raise();
        widget->setVisible ( true );
        widget->activateWindow();
        return false;
      }
    }
  }

  return true;
}

bool dbse::SchemaClassEditor::ShouldOpenRelationshipEditor ( QString relName )
{
  QString name = QString::fromStdString(SchemaClass->get_name() + "::") +
    relName;

  for ( QWidget* widget : QApplication::allWidgets() )
  {
    if (dynamic_cast<SchemaRelationshipEditor *> (widget) != nullptr)
    {
      if ( ( widget->objectName() ).compare ( name ) == 0 )
      {
        widget->raise();
        widget->setVisible ( true );
        widget->activateWindow();
        return false;
      }
    }
  }

  return true;
}

bool dbse::SchemaClassEditor::ShouldOpenMethodEditor ( QString Name )
{
  bool WidgetFound = false;

  for ( QWidget * Editor : QApplication::allWidgets() )
  {
    SchemaMethodEditor * Widget = dynamic_cast<SchemaMethodEditor *> ( Editor );

    if ( Widget != nullptr )
    {
      if ( ( Widget->objectName() ).compare ( Name ) == 0 )
      {
        Widget->raise();
        Widget->setVisible ( true );
        Widget->activateWindow();
        WidgetFound = true;
      }
    }
  }

  return !WidgetFound;
}

void dbse::SchemaClassEditor::RemoveAttribute()
{
  const std::string& attributeName = CurrentRow.at ( 0 ).toStdString();
  OksAttribute * SchemaAttribute = SchemaClass->find_direct_attribute( attributeName );
  if(SchemaAttribute != nullptr) {
      KernelWrapper::GetInstance().PushRemoveAttributeCommand (
        SchemaClass, SchemaAttribute, SchemaAttribute->get_name(), SchemaAttribute->get_type(),
        SchemaAttribute->get_is_multi_values(), SchemaAttribute->get_range(),
        SchemaAttribute->get_init_value(), SchemaAttribute->get_description(),
        SchemaAttribute->get_is_no_null(), SchemaAttribute->get_format() );
  } else {
      QMessageBox::warning ( 0, "Schema editor",
                             QString::fromStdString( "Cannot remove attribute \"" +  attributeName +
                                                     "\" because it is not a direct attribute of \"" + SchemaClass->get_name() + "\"") );
  }
}

void dbse::SchemaClassEditor::RemoveRelationship()
{
  const std::string& relationshipName = CurrentRow.at ( 0 ).toStdString();
  OksRelationship * SchemaRelationship = SchemaClass->find_direct_relationship( relationshipName );
  if(SchemaRelationship != nullptr) {
      KernelWrapper::GetInstance().PushRemoveRelationship (
        SchemaClass, SchemaRelationship, SchemaRelationship->get_name(),
        SchemaRelationship->get_description(), SchemaRelationship->get_type(),
        SchemaRelationship->get_is_composite(), SchemaRelationship->get_is_exclusive(),
        SchemaRelationship->get_is_dependent(),
        SchemaRelationship->get_low_cardinality_constraint(),
        SchemaRelationship->get_high_cardinality_constraint() );
  } else {
      QMessageBox::warning ( 0, "Schema editor",
                             QString::fromStdString( "Cannot remove relationship \"" +  relationshipName +
                                                     "\" because it is not a direct relationship of \"" + SchemaClass->get_name() + "\"") );
  }
}

void dbse::SchemaClassEditor::RemoveMethod()
{
  const std::string& methodName = CurrentRow.at ( 0 ).toStdString();
  OksMethod * SchemaMethod = SchemaClass->find_direct_method ( methodName );
  if(SchemaMethod != nullptr) {
      KernelWrapper::GetInstance().PushRemoveMethodCommand ( SchemaClass, SchemaMethod,
                                                             SchemaMethod->get_name(),
                                                             SchemaMethod->get_description() );
  } else {
      QMessageBox::warning ( 0, "Schema editor",
                             QString::fromStdString( "Cannot remove method \"" +  methodName +
                                                     "\" because it is not a direct method of \"" + SchemaClass->get_name() + "\"") );
  }
}

void dbse::SchemaClassEditor::RemoveSuperClass()
{
  const std::string& superClassName = CurrentRow.at ( 0 ).toStdString();
  if(SchemaClass->has_direct_super_class(superClassName))
  {
      KernelWrapper::GetInstance().PushRemoveSuperClassCommand(SchemaClass, superClassName);
  }
  else
  {
      QMessageBox::warning ( 0, "Schema editor",
                             QString::fromStdString( "Cannot remove super-class \"" +  superClassName +
                                                     "\" because it is not a direct super-class of \"" + SchemaClass->get_name() + "\"") );
  }
}


void dbse::SchemaClassEditor::ParseToSave()
{
  std::string NewDescription = ui->DescriptionTextEdit->toPlainText().toStdString();

  bool Abstract = false;

  if ( ui->AbstractComboBox->currentIndex() == 0 )
  {
    Abstract = true;
  }
  else
  {
    Abstract = false;
  }

  if ( Abstract != SchemaClass->get_is_abstract() )
  {
    KernelWrapper::GetInstance().PushSetAbstractClassCommand ( SchemaClass, Abstract );
  }

  if ( NewDescription != SchemaClass->get_description() )
  {
    KernelWrapper::GetInstance().PushSetDescriptionClassCommand ( SchemaClass, NewDescription );
  }

  close();
}

void dbse::SchemaClassEditor::AddNewSuperClass()
{
  bool widgetFound=false;
  QString class_name = QString::fromStdString(SchemaClass->get_name());

  QString wname = "add_sc_"+class_name;
  for ( QWidget* widget : QApplication::allWidgets() ) {
    if (widget->objectName().compare(wname) == 0 ) {
      widget->raise();
      widget->setVisible ( true );
      widget->activateWindow();
      widgetFound = true;
      break;
    }
  }

  if (!widgetFound) {
    QWidget* widget = new QWidget();
    widget->setObjectName(wname);
    QLabel* label = new QLabel("Select a super-class to add to " + class_name);
    label->setWordWrap(true);

    // QLabel* status = new QLabel();
    // status->setStyleSheet("QLabel { color : green; }");

    auto search = new QLineEdit(widget);
    search->setPlaceholderText("Search classes");
    search->setClearButtonEnabled(true);

    QListWidget* qlwidget = new QListWidget();
    QStringList allClasses;
    KernelWrapper::GetInstance().GetClassListString(allClasses);
    qlwidget->addItems(allClasses);

    connect ( qlwidget, &QListWidget::itemActivated,
              this, [=] () {
                                const std::string& className = qlwidget->currentItem()->text().toStdString();
                                KernelWrapper::GetInstance().PushAddSuperClassCommand(SchemaClass, className);
                                BuildModels();
                                // status->setText(QString::fromStdString(className + " added as a super-class of " + SchemaClass->get_name()));
                           });

    connect (search, &QLineEdit::textChanged, this, [search, qlwidget] () {
      auto items = qlwidget->findItems(search->text(), Qt::MatchRegularExpression);
      if (!items.empty()) {
        qlwidget->setCurrentItem(items[0]);
      }
    });

    auto close_button = new QPushButton("Close");
    connect (close_button, SIGNAL (clicked()), widget, SLOT(close()));
    QVBoxLayout* lout = new QVBoxLayout();
    lout->addWidget(label);
    lout->addWidget(qlwidget);
    lout->addWidget(search);
    lout->addWidget(close_button);
    // l->addWidget(status);

    widget->setWindowTitle("Select super-class(es) for " + class_name);
    widget->setLayout(lout);
    widget->setParent(this, Qt::Dialog);
    widget->show();
  }
}

void dbse::SchemaClassEditor::AddNewAttribute()
{
  if (ShouldOpenAttributeEditor("")) {
    SchemaAttributeEditor * Editor = new SchemaAttributeEditor ( SchemaClass );
    connect ( Editor, SIGNAL ( RebuildModel() ), this, SLOT ( BuildAttributeModelSlot() ) );
    Editor->show();
  }
}

void dbse::SchemaClassEditor::AddNewRelationship()
{
  if (ShouldOpenRelationshipEditor("")) {
    SchemaRelationshipEditor * Editor = new SchemaRelationshipEditor ( SchemaClass );
    connect ( Editor, SIGNAL ( RebuildModel() ), this, SLOT ( BuildRelationshipModelSlot() ) );
    Editor->show();
  }
}

void dbse::SchemaClassEditor::AddNewMethod()
{
  SchemaMethodEditor * Editor = new SchemaMethodEditor ( SchemaClass );
  connect ( Editor, SIGNAL ( RebuildModel() ), this, SLOT ( BuildMethodModelSlot() ) );
  Editor->show();
}

void dbse::SchemaClassEditor::OpenAttributeEditor ( QModelIndex Index )
{
  QStringList Row = AttributeModel->getRowFromIndex ( Index );
  bool ShouldOpen = ShouldOpenAttributeEditor ( Row.at ( 0 ) );

  if ( !Row.isEmpty() && ShouldOpen )
  {
    SchemaAttributeEditor * Editor = new SchemaAttributeEditor (
      SchemaClass, SchemaClass->find_attribute ( Row.at ( 0 ).toStdString() ) );
    connect ( Editor, SIGNAL ( RebuildModel() ), this, SLOT ( BuildAttributeModelSlot() ) );
    Editor->show();
  }
}

void dbse::SchemaClassEditor::OpenRelationshipEditor ( QModelIndex Index )
{
  QStringList Row = RelationshipModel->getRowFromIndex ( Index );
  bool ShouldOpen = ShouldOpenRelationshipEditor ( Row.at ( 0 ) );

  if ( !Row.isEmpty() && ShouldOpen )
  {
    SchemaRelationshipEditor * Editor = new SchemaRelationshipEditor (
      SchemaClass, SchemaClass->find_relationship ( Row.at ( 0 ).toStdString() ) );
    connect ( Editor, SIGNAL ( RebuildModel() ), this, SLOT ( BuildRelationshipModelSlot() ) );
    Editor->show();
  }
}

void dbse::SchemaClassEditor::OpenMethodEditor ( QModelIndex Index )
{
  QStringList Row = MethodModel->getRowFromIndex ( Index );
  auto method = Row.at(0).toStdString();
  if (SchemaClass->find_direct_method(method) == nullptr) {
    std::string message{"Method '" + method + "' is not a direct method of " +
      SchemaClass->get_name() +
". Please open the method editor from the base class or add a local implementation to overrride it."};
    QMessageBox::warning (0,
                          "Schema editor",
                          QString::fromStdString(message));
    return;
  }

  bool ShouldOpen = ShouldOpenMethodEditor ( Row.at ( 0 ) );

  if ( !Row.isEmpty() && ShouldOpen )
  {
    SchemaMethodEditor * Editor = new SchemaMethodEditor (
      SchemaClass, SchemaClass->find_method ( method ) );
    connect ( Editor, SIGNAL ( RebuildModel() ), this, SLOT ( BuildMethodModelSlot() ) );
    Editor->show();
  }
}

void dbse::SchemaClassEditor::OpenSuperClass ( QModelIndex Index )
{
    QStringList Row = SuperClassModel->getRowFromIndex ( Index );

    if ( !Row.isEmpty() )
    {
      QString ClassName = Row.at ( 0 );
      OpenNewClassEditor(ClassName);
    }
}

void dbse::SchemaClassEditor::OpenSubClass ( QModelIndex Index )
{
    QStringList Row = SubClassModel->getRowFromIndex ( Index );

    if ( !Row.isEmpty() )
    {
      QString ClassName = Row.at ( 0 );
      OpenNewClassEditor(ClassName);
    }
}

void dbse::SchemaClassEditor::OpenNewClassEditor ( const QString& ClassName )
{
  bool WidgetFound = false;
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

void dbse::SchemaClassEditor::BuildAttributeModelSlot()
{
  QStringList AttributeHeaders
  { "Name", "Type" };

  if ( AttributeModel != nullptr ) {
    delete AttributeModel;
  }
  AttributeModel = new CustomAttributeModel (
    SchemaClass,
    AttributeHeaders,
    ui->ShowDerivedAttributes->isChecked());

  ui->AttributeView->setModel ( AttributeModel );
  ui->AttributeView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  ui->AttributeView->setSelectionBehavior ( QAbstractItemView::SelectRows );
}

void dbse::SchemaClassEditor::BuildRelationshipModelSlot()
{
  QStringList RelationshipHeaders
  { "Name", "Type", "Low cc", "High cc" };

  if ( RelationshipModel == nullptr ) RelationshipModel = new CustomRelationshipModel (
      SchemaClass, RelationshipHeaders, ui->ShowDerivedRelationships->isChecked() );
  else
  {
    delete RelationshipModel;
    RelationshipModel = new CustomRelationshipModel ( SchemaClass, RelationshipHeaders, ui->ShowDerivedRelationships->isChecked() );
  }

  ui->RelationshipView->setModel ( RelationshipModel );
  ui->RelationshipView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  ui->RelationshipView->setSelectionBehavior ( QAbstractItemView::SelectRows );
}

void dbse::SchemaClassEditor::BuildMethodModelSlot()
{
  QStringList MethodHeaders
  { "Method Name" };

  if ( MethodModel == nullptr )
  {
    MethodModel = new CustomMethodModel ( SchemaClass, MethodHeaders, ui->ShowDerivedMethods->isChecked() );
  }
  else
  {
    delete MethodModel;
    MethodModel = new CustomMethodModel ( SchemaClass, MethodHeaders, ui->ShowDerivedMethods->isChecked() );
  }

  ui->MethodsView->setModel ( MethodModel );
  ui->MethodsView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  ui->MethodsView->setSelectionBehavior ( QAbstractItemView::SelectRows );
}

void dbse::SchemaClassEditor::BuildSuperClassModelSlot()
{
  QStringList SuperClassHeaders
  { "Class Name" };

  if ( SuperClassModel == nullptr )
  {
    SuperClassModel = new CustomSuperClassModel ( SchemaClass, SuperClassHeaders, ui->ShowAllSuperClasses->isChecked() );
  }
  else
  {
    delete SuperClassModel;
    SuperClassModel = new CustomSuperClassModel ( SchemaClass, SuperClassHeaders, ui->ShowAllSuperClasses->isChecked() );
  }

  ui->SuperClassView->setModel ( SuperClassModel );
  ui->SuperClassView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  ui->SuperClassView->setSelectionBehavior ( QAbstractItemView::SelectRows );
}

void dbse::SchemaClassEditor::BuildSubClassModelSlot()
{
  QStringList SubClassHeaders
  { "Class Name" };

  if ( SubClassModel == nullptr )
  {
    SubClassModel = new CustomSubClassModel ( SchemaClass, SubClassHeaders );
  }
  else
  {
    delete SubClassModel;
    SubClassModel = new CustomSubClassModel ( SchemaClass, SubClassHeaders );
  }

  ui->SubClassView->setModel ( SubClassModel );
  ui->SubClassView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  ui->SubClassView->setSelectionBehavior ( QAbstractItemView::SelectRows );
}

void dbse::SchemaClassEditor::CustomMenuClassView ( QPoint pos )
{
  if ( ContextMenuClass == nullptr )
  {
    ContextMenuClass = new QMenu ( this );

    QAction * Add = new QAction ( tr ( "&Add" ), this );
    Add->setShortcut ( tr ( "Ctrl+A" ) );
    Add->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Add, SIGNAL ( triggered() ), this, SLOT ( AddNewSuperClass() ) );

    QAction * Remove = new QAction ( tr ( "&Remove" ), this );
    Remove->setShortcut ( tr ( "Ctrl+R" ) );
    Remove->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Remove, SIGNAL ( triggered() ), this, SLOT ( RemoveSuperClass() ) );

    ContextMenuClass->addAction ( Add );
    ContextMenuClass->addAction ( Remove );
  }

  QModelIndex Index = ui->SuperClassView->currentIndex();

  if ( Index.isValid() )
  {
    CurrentRow = SuperClassModel->getRowFromIndex ( Index );
    ContextMenuClass->exec ( ui->SuperClassView->mapToGlobal ( pos ) );
  }
}

void dbse::SchemaClassEditor::CustomMenuAttributeView ( QPoint pos )
{
  if ( ContextMenuAttribute == nullptr )
  {
    ContextMenuAttribute = new QMenu ( this );

    QAction * Add = new QAction ( tr ( "&Add" ), this );
    Add->setShortcut ( tr ( "Ctrl+A" ) );
    Add->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Add, SIGNAL ( triggered() ), this, SLOT ( AddNewAttribute() ) );

    QAction * Remove = new QAction ( tr ( "&Remove" ), this );
    Remove->setShortcut ( tr ( "Ctrl+R" ) );
    Remove->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Remove, SIGNAL ( triggered() ), this, SLOT ( RemoveAttribute() ) );

    ContextMenuAttribute->addAction ( Add );
    ContextMenuAttribute->addAction ( Remove );
  }

  QModelIndex Index = ui->AttributeView->currentIndex();

  if ( Index.isValid() )
  {
    CurrentRow = AttributeModel->getRowFromIndex ( Index );
    ContextMenuAttribute->exec ( ui->AttributeView->mapToGlobal ( pos ) );
  }
}

void dbse::SchemaClassEditor::CustomMenuRelationshipView ( QPoint pos )
{
  if ( ContextMenuRelationship == nullptr )
  {
    ContextMenuRelationship = new QMenu ( this );

    QAction * Add = new QAction ( tr ( "&Add" ), this );
    Add->setShortcut ( tr ( "Ctrl+A" ) );
    Add->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Add, SIGNAL ( triggered() ), this, SLOT ( AddNewRelationship() ) );

    QAction * Remove = new QAction ( tr ( "&Remove" ), this );
    Remove->setShortcut ( tr ( "Ctrl+R" ) );
    Remove->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Remove, SIGNAL ( triggered() ), this, SLOT ( RemoveRelationship() ) );

    ContextMenuRelationship->addAction ( Add );
    ContextMenuRelationship->addAction ( Remove );
  }

  QModelIndex Index = ui->RelationshipView->currentIndex();

  if ( Index.isValid() )
  {
    CurrentRow = RelationshipModel->getRowFromIndex ( Index );
    ContextMenuRelationship->exec ( ui->RelationshipView->mapToGlobal ( pos ) );
  }
}

void dbse::SchemaClassEditor::CustomMenuMethodView ( QPoint pos )
{
  if ( ContextMenuMethod == nullptr )
  {
    ContextMenuMethod = new QMenu ( this );

    QAction * Add = new QAction ( tr ( "&Add" ), this );
    Add->setShortcut ( tr ( "Ctrl+A" ) );
    Add->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Add, SIGNAL ( triggered() ), this, SLOT ( AddNewMethod() ) );

    QAction * Remove = new QAction ( tr ( "&Remove" ), this );
    Remove->setShortcut ( tr ( "Ctrl+R" ) );
    Remove->setShortcutContext ( Qt::WidgetShortcut );
    connect ( Remove, SIGNAL ( triggered() ), this, SLOT ( RemoveMethod() ) );

    ContextMenuMethod->addAction ( Add );
    ContextMenuMethod->addAction ( Remove );
  }

  QModelIndex Index = ui->MethodsView->currentIndex();

  if ( Index.isValid() )
  {
    CurrentRow = MethodModel->getRowFromIndex ( Index );
    ContextMenuMethod->exec ( ui->MethodsView->mapToGlobal ( pos ) );
  }
}

QString dbse::SchemaClassEditor::createNewClass ()
{
    auto createNewClass = [] (const std::string& className) -> bool {
        if ( !KernelWrapper::GetInstance().IsActive() )
        {
          QMessageBox::warning (
            0,
            "Schema editor",
            QString (
              "There is no active schema set!\n\nPlease set a schema file as active and continue!" ) );

          return false;
        }

        if ( !KernelWrapper::GetInstance().FindClass ( className ) )
        {
          KernelWrapper::GetInstance().PushCreateClassCommand ( className,
                                                                "", false );
          return true;
        }
        else
        {
          QMessageBox::warning ( 0, "Schema editor",
                                 QString ( "Can not create class because class already exist !" ) );

          return false;
        }
    };

    bool ok;
    QString text = QInputDialog::getText(nullptr, "Schema editor: create new class",
                                         "New class name:", QLineEdit::Normal,
                                         "NewOksClass", &ok);

    if(ok && !text.isEmpty()) {
        if(createNewClass(text.toStdString())) {
           SchemaClassEditor * Editor = new SchemaClassEditor(KernelWrapper::GetInstance().FindClass(text.toStdString()));
           Editor->show();
        }
    }
    return text;
}

void dbse::SchemaClassEditor::launch(QString class_name) {
  bool widgetFound=false;
  dunedaq::oks::OksClass* class_info =
    KernelWrapper::GetInstance().FindClass (class_name.toStdString());
  for ( QWidget* widget : QApplication::allWidgets() ) {
    SchemaClassEditor* editor = dynamic_cast<SchemaClassEditor *> (widget);
    if (editor != nullptr) {
      if ((editor->objectName()).compare(class_name) == 0 ) {
          widget->raise();
          widget->setVisible ( true );
          widget->activateWindow();
          widgetFound = true;
          break;
        }
      }
    }

    if ( !widgetFound ) {
      SchemaClassEditor* editor = new SchemaClassEditor (class_info);
      editor->show();
    }
}

