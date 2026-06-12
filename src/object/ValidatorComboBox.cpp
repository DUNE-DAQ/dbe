/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/// Including QT Headers
#include <QApplication>
/// Including DBE
#include "dbe/ValidatorComboBox.hpp"
#include "dbe/StyleUtility.hpp"

dbe::ValidatorComboBox::ValidatorComboBox ( QWidget * parent )
  : QComboBox ( parent ),
    Valid ( false ),
    ValueChanged ( false )
{
  connect ( this, SIGNAL ( editTextChanged ( const QString & ) ), this,
            SLOT ( TryValidate ( const QString & ) ) );
  connect ( this, SIGNAL ( activated ( const QString & ) ), this,
            SLOT ( ChangeDetected ( const QString & ) ) );
  connect ( this, SIGNAL ( currentIndexChanged ( int ) ), this,
            SLOT ( CheckDefaults ( int ) ) );
}

dbe::ValidatorComboBox::~ValidatorComboBox()
{
}

bool dbe::ValidatorComboBox::IsValid() const
{
  return Valid;
}

bool dbe::ValidatorComboBox::IsChanged() const
{
  return ValueChanged;
}

QStringList dbe::ValidatorComboBox::GetDataList()
{
  DataList.clear();
  DataList << currentText();
  return DataList;
}

void dbe::ValidatorComboBox::wheelEvent ( QWheelEvent * event )
{
  ( void ) event;
  return;
}

void dbe::ValidatorComboBox::TryValidate ( const QString & ValidateString )
{
  QString Tmp = ValidateString;
  int index = 0;

  if ( this->validator() != nullptr )
  {
    if ( this->validator()->validate ( Tmp, index ) == QValidator::Acceptable )
    {
      Valid = true;

      if ( CompareDefaults() )
      {
        this->setPalette ( StyleUtility::LoadedDefault );
      }
      else
      {
        this->setPalette ( QApplication::palette ( this ) );
      }
    }
    else if ( this->validator()->validate ( Tmp, index ) == QValidator::Intermediate )
    {
      Valid = false;
      this->setPalette ( StyleUtility::WarningStatusBarPallete );
    }
  }
  else
  {
    if ( CompareDefaults() )
    {
      this->setPalette ( StyleUtility::LoadedDefault );
    }
    else
    {
      this->setPalette ( QApplication::palette ( this ) );
    }
  }
}

void dbe::ValidatorComboBox::ChangeDetected ( const QString & StringChange )
{
  Q_UNUSED ( StringChange )
  ValueChanged = true;
  emit ValueWasChanged();
}

void dbe::ValidatorComboBox::CheckDefaults ( int DefaultIndex )
{
  Q_UNUSED ( DefaultIndex )

  if ( !Default.isEmpty() )
  {
    TryValidate ( this->currentText() );
  }
}

bool dbe::ValidatorComboBox::CompareDefaults()
{
  if ( Default.isEmpty() )
  {
    return false;
  }

  if ( Default == this->currentText() )
  {
    return true;
  }

  return false;
}
