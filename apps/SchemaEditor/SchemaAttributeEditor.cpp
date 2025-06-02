/// Including schema
#include "dbe/SchemaAttributeEditor.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
/// Including auto-generated ui
#include "ui_SchemaAttributeEditor.h"

using namespace dunedaq::oks;

dbse::SchemaAttributeEditor::SchemaAttributeEditor ( OksClass * ClassInfo,
                                                     OksAttribute * AttributeData,
                                                     QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaAttributeEditor ),
    SchemaClass ( ClassInfo ),
    SchemaAttribute ( AttributeData ),
    UsedNew ( false )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  InitialSettings();
  SetController();
}

dbse::SchemaAttributeEditor::SchemaAttributeEditor ( OksClass * ClassInfo,
                                                     QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::SchemaAttributeEditor ),
    SchemaClass ( ClassInfo ),
    SchemaAttribute ( nullptr ),
    UsedNew ( true )
{
  QWidget::setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi ( this );
  InitialSettings();
  SetController();
}

dbse::SchemaAttributeEditor::~SchemaAttributeEditor() = default;

void dbse::SchemaAttributeEditor::FillInfo()
{
    setWindowTitle ( QString ( "Attribute Editor : %1" ).arg (
                       SchemaAttribute->get_name().c_str() ) );
    setObjectName ( QString::fromStdString ( SchemaAttribute->get_name() ) );
    ui->AttributeNameLineEdit->setText ( QString::fromStdString (
                                           SchemaAttribute->get_name() ) );
    ui->AttributeTypeComboBox->setCurrentIndex (
      ui->AttributeTypeComboBox->findData ( QString::fromStdString (
                                              SchemaAttribute->get_type() ),
                                            Qt::DisplayRole ) );

    if ( SchemaAttribute->get_is_multi_values() )
      ui->AttributeIsMultivariable->setCurrentIndex (
        0 );
    else
    {
      ui->AttributeIsMultivariable->setCurrentIndex ( 1 );
    }

    if ( SchemaAttribute->get_is_no_null() )
    {
      ui->AttributeIsNotNull->setCurrentIndex ( 0 );
    }
    else
    {
      ui->AttributeIsNotNull->setCurrentIndex ( 1 );
    }

    ui->AttributeDescriptionTextBox->setPlainText (
      QString::fromStdString ( SchemaAttribute->get_description() ) );
    ui->AttributeRangeLineEdit->setText ( QString::fromStdString (
                                            SchemaAttribute->get_range() ) );
    ui->AttributeInitialValue->setText (
      QString::fromStdString ( SchemaAttribute->get_init_value() ) );

    int Index = ui->AttributeTypeComboBox->currentIndex();

    if ( Index == 0 || Index >= 9 )
    {
      ui->FormatLayout->setEnabled ( false );
      ui->FormatLabel->hide();
      ui->AttributeFormatComboBox->hide();
    }
    else
    {
      ui->FormatLayout->setEnabled ( true );
      ui->FormatLabel->show();
      ui->AttributeFormatComboBox->show();

      if ( SchemaAttribute->get_format() == OksAttribute::Dec )
        ui->AttributeFormatComboBox->setCurrentIndex (
          0 );
      else if ( SchemaAttribute->get_format() == OksAttribute::Oct )
        ui->AttributeFormatComboBox->setCurrentIndex (
          1 );
      else
      {
        ui->AttributeFormatComboBox->setCurrentIndex ( 2 );
      }
    }
}

void dbse::SchemaAttributeEditor::InitialSettings()
{
  QStringList Items
  {
    "bool", "s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64", "float", "double", "date",
    "time", "string", "enum", "class" };
  ui->AttributeTypeComboBox->addItems ( Items );
  setWindowTitle ( QString::fromStdString ( "New Attribute" ) );
  setObjectName ( "NEW" );

  if ( !UsedNew )
  {
      FillInfo();
  }
}

void dbse::SchemaAttributeEditor::SetController()
{
  connect ( ui->AttributeTypeComboBox, SIGNAL ( currentIndexChanged ( int ) ), this,
            SLOT ( ToggleFormat ( int ) ) );
  connect ( ui->buttonBox, SIGNAL ( accepted() ), this, SLOT ( ProxySlot() ) );
  connect ( ui->buttonBox, SIGNAL ( rejected() ), this, SLOT ( close() ) );
  connect ( &KernelWrapper::GetInstance(), SIGNAL ( ClassUpdated ( QString ) ), this,
            SLOT ( ClassUpdated ( QString ) ) );
}

void dbse::SchemaAttributeEditor::ClassUpdated( QString ClassName)
{
    if(!UsedNew && ClassName.toStdString() == SchemaClass->get_name()) {
        if(SchemaClass->find_direct_attribute(SchemaAttribute->get_name()) != nullptr) {
            FillInfo();
        } else {
            QWidget::close();
        }
    }
}

void dbse::SchemaAttributeEditor::ParseToSave()
{
  bool Changed = false;
  std::string AttributeName = ui->AttributeNameLineEdit->text().toStdString();
  std::string AttributeType = ui->AttributeTypeComboBox->currentText().toStdString();
  char AttributeFormatChar = ( ui->AttributeFormatComboBox->currentText().toStdString() ) [0];

  OksAttribute::Format AttributeFormat;

  switch ( AttributeFormatChar )
  {
  case 'D':
    AttributeFormat = OksAttribute::Format::Dec;
    break;

  case 'O':
    AttributeFormat = OksAttribute::Format::Oct;
    break;

  case 'H':
    AttributeFormat = OksAttribute::Format::Hex;
    break;

  default:
    AttributeFormat = OksAttribute::Format::Dec;
    break;
  }

  /// Making sure we dont get anything weird
  int Index = ui->AttributeTypeComboBox->currentIndex();

  if ( Index == 0 || Index >= 9 )
  {
    AttributeFormat = OksAttribute::Format::Dec;
  }

  bool IsMultivariable = false;

  switch ( ui->AttributeIsMultivariable->currentIndex() )
  {
  case 0:
    IsMultivariable = true;
    break;

  case 1:
    IsMultivariable = false;
    break;

  default:
    IsMultivariable = false;
    break;
  }

  bool IsNotNull = false;

  switch ( ui->AttributeIsNotNull->currentIndex() )
  {
  case 0:
    IsNotNull = true;
    break;

  case 1:
    IsNotNull = false;
    break;

  default:
    IsNotNull = false;
    break;
  }

  std::string AttributeDescription =
    ui->AttributeDescriptionTextBox->toPlainText().toStdString();
  std::string AttributeRange = ui->AttributeRangeLineEdit->text().toStdString();
  std::string AttributeInitial = ui->AttributeInitialValue->text().toStdString();

  if ( AttributeName != SchemaAttribute->get_name() )
  {
    KernelWrapper::GetInstance().PushSetAttributeNameCommand ( SchemaClass, SchemaAttribute, AttributeName );
    Changed = true;
  }

  if ( AttributeType != SchemaAttribute->get_type() )
  {
    KernelWrapper::GetInstance().PushSetAttributeTypeCommand ( SchemaClass, SchemaAttribute, AttributeType );
    Changed = true;
  }

  if ( AttributeFormat != SchemaAttribute->get_format() )
  {
    KernelWrapper::GetInstance().PushSetAttributeFormatCommand ( SchemaClass,
                                                                 SchemaAttribute,
                                                                 AttributeFormat );
    Changed = true;
  }

  if ( IsMultivariable != SchemaAttribute->get_is_multi_values() )
  {
    KernelWrapper::GetInstance().PushSetAttributeMultiCommand ( SchemaClass,
                                                                SchemaAttribute,
                                                                IsMultivariable );
    Changed = true;
  }

  if ( IsNotNull != SchemaAttribute->get_is_no_null() )
  {
    KernelWrapper::GetInstance().PushSetAttributeIsNullCommand ( SchemaClass, SchemaAttribute, IsNotNull );
    Changed = true;
  }

  if ( AttributeDescription != SchemaAttribute->get_description() )
  {
    KernelWrapper::GetInstance().PushSetAttributeDescriptionCommand ( SchemaClass,
                                                                      SchemaAttribute,
                                                                      AttributeDescription );
    Changed = true;
  }

  if ( AttributeRange != SchemaAttribute->get_range() )
  {
    KernelWrapper::GetInstance().PushSetAttributeRangeCommand ( SchemaClass,
                                                                SchemaAttribute,
                                                                AttributeRange );
    Changed = true;
  }

  if ( AttributeInitial != SchemaAttribute->get_init_value() )
  {
    KernelWrapper::GetInstance().PushSetAttributeInitialValuesCommand ( SchemaClass,
                                                                        SchemaAttribute,
                                                                        AttributeInitial );
    Changed = true;
  }

  if ( Changed )
  {
    emit RebuildModel();
  }

  close();
}

void dbse::SchemaAttributeEditor::ParseToCreate()
{
  std::string AttributeName = ui->AttributeNameLineEdit->text().toStdString();
  std::string AttributeType = ui->AttributeTypeComboBox->currentText().toStdString();
  char AttributeFormatChar = ( ui->AttributeFormatComboBox->currentText().toStdString() ) [0];

  OksAttribute::Format AttributeFormat;

  switch ( AttributeFormatChar )
  {
  case 'D':
    AttributeFormat = OksAttribute::Format::Dec;
    break;

  case 'O':
    AttributeFormat = OksAttribute::Format::Oct;
    break;

  case 'H':
    AttributeFormat = OksAttribute::Format::Hex;
    break;

  default:
    AttributeFormat = OksAttribute::Format::Dec;
    break;
  }

  /// Making sure we dont get anything weird
  int Index = ui->AttributeTypeComboBox->currentIndex();

  if ( Index == 0 || Index >= 9 )
  {
    AttributeFormat = OksAttribute::Format::Dec;
  }

  bool IsMultivariable = false;

  switch ( ui->AttributeIsMultivariable->currentIndex() )
  {
  case 0:
    IsMultivariable = true;
    break;

  case 1:
    IsMultivariable = false;
    break;

  default:
    IsMultivariable = false;
    break;
  }

  bool IsNotNull = false;

  switch ( ui->AttributeIsNotNull->currentIndex() )
  {
  case 0:
    IsNotNull = true;
    break;

  case 1:
    IsNotNull = false;
    break;

  default:
    IsNotNull = false;
    break;
  }

  std::string AttributeDescription =
    ui->AttributeDescriptionTextBox->toPlainText().toStdString();
  std::string AttributeRange = ui->AttributeRangeLineEdit->text().toStdString();
  std::string AttributeInitial = ui->AttributeInitialValue->text().toStdString();

  /// Creating attribute and updating
  KernelWrapper::GetInstance().PushAddAttributeCommand ( SchemaClass, AttributeName,
                                                         AttributeType, IsMultivariable,
                                                         AttributeRange, AttributeInitial,
                                                         AttributeDescription, IsNotNull,
                                                         AttributeFormat );
  emit RebuildModel();
  close();
}

void dbse::SchemaAttributeEditor::ProxySlot()
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

void dbse::SchemaAttributeEditor::ToggleFormat ( int )
{
  int Index = ui->AttributeTypeComboBox->currentIndex();

  if ( Index == 0 || Index >= 9 )
  {
    ui->FormatLayout->setEnabled ( false );
    ui->FormatLabel->hide();
    ui->AttributeFormatComboBox->hide();
  }
  else
  {
    ui->FormatLayout->setEnabled ( true );
    ui->FormatLabel->show();
    ui->AttributeFormatComboBox->show();
  }
}
