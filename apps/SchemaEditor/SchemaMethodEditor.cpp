/// Including Qt
#include <QMessageBox>
/// Including Schema
#include "dbe/SchemaMethodEditor.hpp"
#include "dbe/SchemaMethodImplementationEditor.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
/// Including auto-generated ui
#include "ui_SchemaMethodEditor.h"

using namespace dunedaq::oks;

dbse::SchemaMethodEditor::~SchemaMethodEditor() = default;

dbse::SchemaMethodEditor::SchemaMethodEditor ( OksClass * ClassInfo, OksMethod * Method,
                                               QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaMethodEditor ),
    m_class ( ClassInfo ),
    m_method ( Method ),
    ImplementationModel ( nullptr ),
    UsedNew ( false )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  InitialSettings();
  BuildModels();
  SetController();
}

dbse::SchemaMethodEditor::SchemaMethodEditor ( OksClass * ClassInfo, QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaMethodEditor ),
    m_class ( ClassInfo ),
    m_method ( nullptr ),
    ImplementationModel ( nullptr ),
    UsedNew ( true )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  InitialSettings();
  SetController();
}

void dbse::SchemaMethodEditor::SetController()
{
  connect ( ui->buttonBox, SIGNAL ( accepted() ), this, SLOT ( ProxySlot() ) );
  connect ( ui->buttonBox, SIGNAL ( rejected() ), this, SLOT ( close() ) );
  connect ( ui->AddButton, SIGNAL ( clicked() ), this,
            SLOT ( AddNewMethodImplementation() ) );
  connect ( ui->ImplementationsView, SIGNAL ( doubleClicked ( QModelIndex ) ), this,
            SLOT ( OpenMethodImplementationEditor ( QModelIndex ) ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassUpdated ( QString ) ), this,
            SLOT ( ClassUpdated ( QString ) ) );
}

void dbse::SchemaMethodEditor::ClassUpdated( QString ClassName)
{
    if(!UsedNew && ClassName.toStdString() == m_class->get_name()) {
        if(m_class->find_direct_method(m_method->get_name()) != nullptr) {
            FillInfo();
            BuildModels();
        } else {
            QWidget::close();
        }
    }
}

void dbse::SchemaMethodEditor::FillInfo()
{
  auto name = QString::fromStdString (
    m_class->get_name() + "::" + m_method->get_name() );
  setObjectName ( name );
  setWindowTitle ( QString ( "Method Editor : %1" ).arg ( name ) );
  ui->MethodName->setText ( QString::fromStdString ( m_method->get_name() ) );
  ui->DescriptionTextBox->setPlainText (
    QString::fromStdString ( m_method->get_description() ) );
}

void dbse::SchemaMethodEditor::InitialSettings()
{
  if ( UsedNew )
  {
    setWindowTitle ( "New Method" );
    setObjectName ( "NEW" );
    ui->AddButton->setEnabled ( true );
  }
  else
  {
      FillInfo();
  }
}

void dbse::SchemaMethodEditor::ParseToSave()
{
  bool changed = false;
  std::string method_name = ui->MethodName->text().toStdString();
  std::string method_description = ui->DescriptionTextBox->toPlainText().toStdString();

  if ( method_name != m_method->get_name() )
  {
    KernelWrapper::GetInstance().PushSetNameMethodCommand ( m_class, m_method, method_name );
    changed = true;
  }

  if ( method_description != m_method->get_description() )
  {
    KernelWrapper::GetInstance().PushSetDescriptionMethodCommand ( m_class,
                                                                   m_method,
                                                                   method_description );
    changed = true;
  }

  if ( changed )
  {
    emit RebuildModel();
  }

  close();
}

bool dbse::SchemaMethodEditor::create() {
  std::string method_name = ui->MethodName->text().toStdString();
  if (method_name.empty()) {
    return false;
  }
  std::string method_description = ui->DescriptionTextBox->toPlainText().toStdString();
  KernelWrapper::GetInstance().PushAddMethodCommand ( m_class, method_name,
                                                      method_description );
  emit RebuildModel();
  UsedNew = false;
  return true;
}
void dbse::SchemaMethodEditor::ParseToCreate()
{
  create();
  close();
}

void dbse::SchemaMethodEditor::BuildModels()
{
  QStringList MethodHeaders { "Language", "Prototype" };

  if ( ImplementationModel != nullptr ) {
    delete ImplementationModel;
  }
  ImplementationModel = new CustomMethodImplementationModel ( m_method, MethodHeaders );

  ui->ImplementationsView->setModel ( ImplementationModel );
  ui->ImplementationsView->horizontalHeader()->setSectionResizeMode ( QHeaderView::ResizeToContents );
}

bool dbse::SchemaMethodEditor::ShouldOpenMethodImplementationEditor ( QString Name )
{
  bool WidgetFound = false;

  for ( QWidget * Editor : QApplication::allWidgets() )
  {
    SchemaMethodImplementationEditor * Widget =
      dynamic_cast<SchemaMethodImplementationEditor *> ( Editor );

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

void dbse::SchemaMethodEditor::ProxySlot()
{
  if ( UsedNew )
  {
    ParseToCreate();
  }
  else
  {
    ParseToSave();
  }
}

void dbse::SchemaMethodEditor::AddNewMethodImplementation()
{
  if (UsedNew) {
    if (create()) {
      m_method = m_class->find_method(ui->MethodName->text().toStdString());
    }
    else {
      QMessageBox::question (
        0, tr ( "SchemaEditor" ),
        tr ( "You must set the method name before you can set an implementation" ),
             QMessageBox::Ok );
      return;
    }
  }
  SchemaMethodImplementationEditor * Editor = new SchemaMethodImplementationEditor (
    m_class, m_method );
  connect ( Editor, SIGNAL ( RebuildModel() ), this,
            SLOT ( BuildModelImplementationSlot() ) );
  Editor->show();
}

void dbse::SchemaMethodEditor::OpenMethodImplementationEditor ( QModelIndex Index )
{
  QStringList Row = ImplementationModel->getRowFromIndex ( Index );
  if ( !Row.isEmpty() ) {
    QString name = QString::fromStdString(m_method->get_name()).append(Row.at ( 0 ));
    if (ShouldOpenMethodImplementationEditor ( name )) {
      SchemaMethodImplementationEditor * Editor = new SchemaMethodImplementationEditor (
        m_class, m_method, m_method->find_implementation ( Row.at ( 0 ).toStdString() ) );
      Editor->show();
    }
  }
}

void dbse::SchemaMethodEditor::BuildModelImplementationSlot()
{
    BuildModels();
}
