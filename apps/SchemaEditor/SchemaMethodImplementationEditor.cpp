#include <QMessageBox>
/// Including Schema
#include "dbe/SchemaMethodImplementationEditor.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include "ui_SchemaMethodImplementationEditor.h"

using namespace dunedaq::oks;

dbse::SchemaMethodImplementationEditor::~SchemaMethodImplementationEditor() = default;

dbse::SchemaMethodImplementationEditor::SchemaMethodImplementationEditor (
  OksClass * Class, OksMethod * Method, OksMethodImplementation * Implementation, QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaMethodImplementationEditor ),
    m_class ( Class ),
    m_method ( Method ),
    m_implementation ( Implementation ),
    UsedNew ( false )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  InitialSettings();
  SetController();
}

dbse::SchemaMethodImplementationEditor::SchemaMethodImplementationEditor (
  OksClass * Class,
  OksMethod * Method,
  QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaMethodImplementationEditor ),
    m_class ( Class ),
    m_method ( Method ),
    m_implementation ( nullptr ),
    UsedNew ( true )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  InitialSettings();
  SetController();
}

void dbse::SchemaMethodImplementationEditor::SetController()
{
  connect ( ui->SaveButton, SIGNAL ( clicked() ), this, SLOT ( ProxySlot() ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassUpdated ( QString ) ), this,
            SLOT ( ClassUpdated ( QString ) ) );
}

void dbse::SchemaMethodImplementationEditor::ClassUpdated( QString class_name )
{
  if(!UsedNew && class_name.toStdString() == m_class->get_name()) {
      if(m_method->find_implementation(m_implementation->get_language()) != nullptr) {
          FillInfo();
      } else {
          QWidget::close();
      }
  }
}

void dbse::SchemaMethodImplementationEditor::FillInfo()
{
  ui->MethodImplementationLanguage->setText (
    QString::fromStdString ( m_implementation->get_language() ) );
  ui->MethodImplementationPrototype->setText (
    QString::fromStdString ( m_implementation->get_prototype() ) );
  ui->MethodImplementationDescription->setPlainText (
    QString::fromStdString ( m_implementation->get_body() ) );

  std::string name = m_class->get_name() + m_method->get_name()
    + m_implementation->get_language();
  setObjectName ( QString::fromStdString(name) );
}

void dbse::SchemaMethodImplementationEditor::InitialSettings()
{
  std::string title = "Method Implementation for " + m_method->get_name() ;
  setWindowTitle (QString::fromStdString(title));
  std::string name = m_method->get_name();
  setObjectName ( QString::fromStdString(name) );

  if ( !UsedNew ) {
      FillInfo();
  }
}

void dbse::SchemaMethodImplementationEditor::ParseToSave()
{
  bool changed = false;
  std::string MethodLanguage = ui->MethodImplementationLanguage->text().toStdString();
  std::string MethodPrototype = ui->MethodImplementationPrototype->text().toStdString();
  std::string MethodDescription = ui->MethodImplementationDescription->toPlainText()
                                  .toStdString();

  if ( MethodLanguage != m_implementation->get_language() )
  {
    KernelWrapper::GetInstance().PushSetMethodImplementationLanguage ( m_class,
                                                                       m_method,
                                                                       m_implementation,
                                                                       MethodLanguage );
    changed = true;
  }

  if ( MethodPrototype != m_implementation->get_prototype() )
  {
    KernelWrapper::GetInstance().PushSetMethodImplementationPrototype ( m_class,
                                                                        m_method,
                                                                        m_implementation,
                                                                        MethodPrototype );
    changed = true;
  }

  if ( MethodDescription != m_implementation->get_body() )
  {
    KernelWrapper::GetInstance().PushSetMethodImplementationBody ( m_class,
                                                                   m_method,
                                                                   m_implementation,
                                                                   MethodDescription );
    changed = true;
  }

  if ( changed )
  {
    emit RebuildModel();
  }

  close();
}

void dbse::SchemaMethodImplementationEditor::ParseToCreate()
{
  if ( ui->MethodImplementationLanguage->text().isEmpty()
       && ui->MethodImplementationLanguage->text().isEmpty()
       &&  ui->MethodImplementationDescription->toPlainText().isEmpty() ) {
    // Close empty dialogue box
    close();
    return;
  }

  if ( ui->MethodImplementationLanguage->text().isEmpty() )
  {
    QMessageBox::warning (
      0, "Schema editor",
      QString ( "Please Provide a Language for the method implementation !" ) );
    return;
  }

  if ( ui->MethodImplementationPrototype->text().isEmpty() )
  {
    QMessageBox::warning (
      0, "Schema editor",
      QString ( "Please Provide a prototype for the method implementation !" ) );
    return;
  }

  std::string MethodLanguage = ui->MethodImplementationLanguage->text().toStdString();
  std::string MethodPrototype = ui->MethodImplementationPrototype->text().toStdString();
  std::string MethodDescription = ui->MethodImplementationDescription->toPlainText()
                                  .toStdString();

  KernelWrapper::GetInstance().PushAddMethodImplementationComand ( m_class,
                                                                   m_method,
                                                                   MethodLanguage,
                                                                   MethodPrototype,
                                                                   MethodDescription );
  emit RebuildModel();
  close();
}

void dbse::SchemaMethodImplementationEditor::ProxySlot()
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
