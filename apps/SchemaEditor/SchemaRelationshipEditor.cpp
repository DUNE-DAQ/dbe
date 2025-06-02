#include <QMessageBox>
/// Including Schema
#include "dbe/SchemaRelationshipEditor.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include "ui_SchemaRelationshipEditor.h"

using namespace dunedaq::oks;

dbse::SchemaRelationshipEditor::~SchemaRelationshipEditor() = default;

dbse::SchemaRelationshipEditor::SchemaRelationshipEditor ( OksClass * Class,
                                                           OksRelationship * Relationship,
                                                           QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaRelationshipEditor ),
    SchemaRelationship ( Relationship ),
    SchemaClass ( Class ),
    UsedNew ( false ),
    GraphScene ( false )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  setWindowTitle (
    QString ( "Relationship Editor : %1" ).arg ( SchemaRelationship->get_name().c_str() ) );
  InitialSettings();
  SetController();
}

dbse::SchemaRelationshipEditor::SchemaRelationshipEditor ( OksClass * Class,
                                                           QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaRelationshipEditor ),
    SchemaRelationship ( nullptr ),
    SchemaClass ( Class ),
    UsedNew ( true ),
    GraphScene ( false )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  setWindowTitle ( "New Relationship" );
  InitialSettings();
  SetController();
}

dbse::SchemaRelationshipEditor::SchemaRelationshipEditor ( OksClass * Class,
                                                           QString ClassType,
                                                           QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaRelationshipEditor ),
    SchemaRelationship ( nullptr ),
    SchemaClass ( Class ),
    UsedNew ( true ),
    GraphScene ( true )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  setWindowTitle ( "New Relationship" );
  InitialSettings();
  SetController();
  ui->RelationshipTypeComboBox->setCurrentIndex (
    ui->RelationshipTypeComboBox->findData ( ClassType, Qt::DisplayRole ) );
}
void dbse::SchemaRelationshipEditor::ClassUpdated( QString ClassName )
{
    if(!UsedNew && ClassName.toStdString() == SchemaClass->get_name()) {
        if(SchemaClass->find_direct_relationship(SchemaRelationship->get_name()) == nullptr) {
            QWidget::close();
        } else {
            FillInfo();
        }
    }
}

void dbse::SchemaRelationshipEditor::FillInfo()
{
    setObjectName ( QString::fromStdString ( SchemaRelationship->get_name() ) );
    ui->RelationshipNameLineEdit->setText (
      QString::fromStdString ( SchemaRelationship->get_name() ) );
    ui->RelationshipTypeComboBox->setCurrentIndex (
      ui->RelationshipTypeComboBox->findData (
        QString::fromStdString ( SchemaRelationship->get_type() ), Qt::DisplayRole ) );
    ui->RelationshipDescriptionTextEdit->setPlainText (
      QString::fromStdString ( SchemaRelationship->get_description() ) );

    if ( SchemaRelationship->get_is_composite() )
    {
      ui->IsCompositeCombo->setCurrentIndex ( 0 );
    }
    else
    {
      ui->IsCompositeCombo->setCurrentIndex ( 1 );
    }

    if ( SchemaRelationship->get_is_exclusive() )
    {
      ui->IsExclusiveCombo->setCurrentIndex ( 0 );
    }
    else
    {
      ui->IsExclusiveCombo->setCurrentIndex ( 1 );
    }

    if ( SchemaRelationship->get_is_dependent() )
    {
      ui->IsDependentCombo->setCurrentIndex ( 0 );
    }
    else
    {
      ui->IsDependentCombo->setCurrentIndex ( 1 );
    }

    if ( SchemaRelationship->get_low_cardinality_constraint() ==
         OksRelationship::CardinalityConstraint::Zero ) ui
      ->LowCcCombo->setCurrentIndex ( 0 );
    else if ( SchemaRelationship->get_low_cardinality_constraint()
              == OksRelationship::CardinalityConstraint::One )
    {
      ui->LowCcCombo->setCurrentIndex ( 1 );
    }
    else
    {
      ui->LowCcCombo->setCurrentIndex ( 2 );
    }

    if ( SchemaRelationship->get_high_cardinality_constraint() ==
         OksRelationship::CardinalityConstraint::Zero ) ui
      ->HighCcCombo->setCurrentIndex ( 0 );
    else if ( SchemaRelationship->get_high_cardinality_constraint()
              == OksRelationship::CardinalityConstraint::One )
    {
      ui->HighCcCombo->setCurrentIndex ( 1 );
    }
    else
    {
      ui->HighCcCombo->setCurrentIndex ( 2 );
    }
}
void dbse::SchemaRelationshipEditor::InitialSettings()
{
  QStringList ClassList;
  KernelWrapper::GetInstance().GetClassListString ( ClassList );
  ui->RelationshipTypeComboBox->addItems ( ClassList );
  setObjectName ( "NEW" );

  if ( !UsedNew )
  {
      FillInfo();
  }
}

void dbse::SchemaRelationshipEditor::SetController()
{
  connect ( ui->buttonBox, SIGNAL ( accepted() ), this, SLOT ( ProxySlot() ) );
  connect ( ui->buttonBox, SIGNAL ( rejected() ), this, SLOT ( close() ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassCreated() ), this,
            SLOT ( UpdateClassCombo() ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassUpdated ( QString ) ), this,
            SLOT ( ClassUpdated ( QString ) ) );
}

void dbse::SchemaRelationshipEditor::ParseToSave()
{
  bool changed = false;
  QString RelationshipName = ui->RelationshipNameLineEdit->text();
  QString RelationshipType = ui->RelationshipTypeComboBox->currentText();
  QString RelationshipDescription = ui->RelationshipDescriptionTextEdit->toPlainText();

  bool IsComposite;

  if ( ui->IsCompositeCombo->currentIndex() == 0 )
  {
    IsComposite = true;
  }
  else
  {
    IsComposite = false;
  }

  bool IsExclusive;

  if ( ui->IsExclusiveCombo->currentIndex() == 0 )
  {
    IsExclusive = true;
  }
  else
  {
    IsExclusive = false;
  }

  bool IsDependent;

  if ( ui->IsDependentCombo->currentIndex() == 0 )
  {
    IsDependent = true;
  }
  else
  {
    IsDependent = false;
  }

  OksRelationship::CardinalityConstraint RelationshipHighCardinality;
  OksRelationship::CardinalityConstraint RelationshipLowCardinality;

  if ( ui->HighCcCombo->currentIndex() == 0 ) RelationshipHighCardinality =
      OksRelationship::CardinalityConstraint::Zero;
  else if ( ui->HighCcCombo->currentIndex() == 1 ) RelationshipHighCardinality =
      OksRelationship::CardinalityConstraint::One;
  else
  {
    RelationshipHighCardinality = OksRelationship::CardinalityConstraint::Many;
  }

  if ( ui->LowCcCombo->currentIndex() == 0 ) RelationshipLowCardinality =
      OksRelationship::CardinalityConstraint::Zero;
  else if ( ui->LowCcCombo->currentIndex() == 1 ) RelationshipLowCardinality =
      OksRelationship::CardinalityConstraint::One;
  else
  {
    RelationshipLowCardinality = OksRelationship::CardinalityConstraint::Many;
  }

  if ( RelationshipName != QString::fromStdString ( SchemaRelationship->get_name() ) )
  {
    KernelWrapper::GetInstance().PushSetNameRelationshipCommand (
      SchemaClass, SchemaRelationship, RelationshipName.toStdString() );
    changed = true;
  }

  if ( RelationshipType != QString::fromStdString ( SchemaRelationship->get_type() ) )
  {
    KernelWrapper::GetInstance().PushSetClassTypeRelationshipCommand (
      SchemaClass, SchemaRelationship, RelationshipType.toStdString() );
    changed = true;
  }

  if ( RelationshipDescription != QString::fromStdString (
         SchemaRelationship->get_description() ) )
  {
    KernelWrapper::GetInstance().PushSetDescriptionRelationshipCommand (
      SchemaClass, SchemaRelationship, RelationshipDescription.toStdString() );
    changed = true;
  }

  if ( SchemaRelationship->get_is_composite() != IsComposite )
  {
    KernelWrapper::GetInstance().PushSetIsCompositeRelationshipCommand ( SchemaClass,
                                                                         SchemaRelationship,
                                                                         IsComposite );
    changed = true;
  }

  if ( SchemaRelationship->get_is_exclusive() != IsExclusive )
  {
    KernelWrapper::GetInstance().PushSetIsExclusiveRelationshipCommand ( SchemaClass,
                                                                         SchemaRelationship,
                                                                         IsExclusive );
    changed = true;
  }

  if ( SchemaRelationship->get_is_dependent() != IsDependent )
  {
    KernelWrapper::GetInstance().PushSetIsDependentRelationshipCommand ( SchemaClass,
                                                                         SchemaRelationship,
                                                                         IsDependent );
    changed = true;
  }

  if ( RelationshipHighCardinality != SchemaRelationship->get_high_cardinality_constraint() )
  {
    KernelWrapper::GetInstance().PushSetHighCcRelationshipCommand (
      SchemaClass, SchemaRelationship, RelationshipHighCardinality );
    changed = true;
  }

  if ( RelationshipLowCardinality != SchemaRelationship->get_low_cardinality_constraint() )
  {
    KernelWrapper::GetInstance().PushSetLowCcRelationshipCommand (
      SchemaClass, SchemaRelationship, RelationshipLowCardinality );
    changed = true;
  }

  if ( changed )
  {
    emit RebuildModel();
  }

  close();
}

void dbse::SchemaRelationshipEditor::ParseToCreate()
{
  if ( ui->RelationshipNameLineEdit->text().isEmpty() )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Please Provide a name for the relationship !" ) );
    return;
  }

  QString RelationshipName = ui->RelationshipNameLineEdit->text();
  QString RelationshipType = ui->RelationshipTypeComboBox->currentText();
  QString RelationshipDescription = ui->RelationshipDescriptionTextEdit->toPlainText();

  bool IsComposite;

  if ( ui->IsCompositeCombo->currentIndex() == 0 )
  {
    IsComposite = true;
  }
  else
  {
    IsComposite = false;
  }

  bool IsExclusive;

  if ( ui->IsExclusiveCombo->currentIndex() == 0 )
  {
    IsExclusive = true;
  }
  else
  {
    IsExclusive = false;
  }

  bool IsDependent;

  if ( ui->IsDependentCombo->currentIndex() == 0 )
  {
    IsDependent = true;
  }
  else
  {
    IsDependent = false;
  }

  OksRelationship::CardinalityConstraint RelationshipHighCardinality;
  OksRelationship::CardinalityConstraint RelationshipLowCardinality;

  if ( ui->HighCcCombo->currentIndex() == 0 ) RelationshipHighCardinality =
      OksRelationship::CardinalityConstraint::Zero;
  else if ( ui->HighCcCombo->currentIndex() == 1 ) RelationshipHighCardinality =
      OksRelationship::CardinalityConstraint::One;
  else
  {
    RelationshipHighCardinality = OksRelationship::CardinalityConstraint::Many;
  }

  if ( ui->LowCcCombo->currentIndex() == 0 ) RelationshipLowCardinality =
      OksRelationship::CardinalityConstraint::Zero;
  else if ( ui->LowCcCombo->currentIndex() == 1 ) RelationshipLowCardinality =
      OksRelationship::CardinalityConstraint::One;
  else
  {
    RelationshipLowCardinality = OksRelationship::CardinalityConstraint::Many;
  }

  KernelWrapper::GetInstance().PushAddRelationship ( SchemaClass,
                                                     RelationshipName.toStdString(),
                                                     RelationshipDescription.toStdString(),
                                                     RelationshipType.toStdString(),
                                                     IsComposite, IsExclusive, IsDependent,
                                                     RelationshipLowCardinality,
                                                     RelationshipHighCardinality );
  emit RebuildModel();

  if ( GraphScene ) emit MakeGraphConnection ( QString::fromStdString (
                                                   SchemaClass->get_name() ),
                                                 ui->RelationshipTypeComboBox->currentText(),
                                                 RelationshipName );

  close();
}

void dbse::SchemaRelationshipEditor::ProxySlot()
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

void dbse::SchemaRelationshipEditor::UpdateClassCombo()
{
  QStringList ClassList;
  KernelWrapper::GetInstance().GetClassListString ( ClassList );
  ui->RelationshipTypeComboBox->clear();
  ui->RelationshipTypeComboBox->addItems ( ClassList );
  ui->RelationshipTypeComboBox->setCurrentIndex (
    ui->RelationshipTypeComboBox->findData (
      QString::fromStdString ( SchemaRelationship->get_type() ), Qt::DisplayRole ) );
}
