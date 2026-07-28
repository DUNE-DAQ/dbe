// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/// Include Qt Headers
#include <QValidator>
#include <QApplication>
#include <QMenu>
#include <QContextMenuEvent>
/// Include DBE
#include "dbe/CustomLineEdit.hpp"
#include "dbe/StyleUtility.hpp"

dbe::CustomLineEdit::CustomLineEdit ( QWidget * parent, bool IsNull, bool IsMulti )
  : QLineEdit ( parent ),
    Valid ( false ),
    NullCheck ( IsNull ),
    CheckDefault ( true ),
    PopupMenu ( false ),
    IsMultiValue ( IsMulti ),
    ContextMenu ( nullptr ),
    Dec ( nullptr ),
    Oct ( nullptr ),
    Hex ( nullptr ),
    Validator ( nullptr )
{
  connect ( this, SIGNAL ( textChanged ( QString ) ), this,
            SLOT ( TryValidate ( QString ) ) );
  this->setFixedHeight ( 30 );
  //this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

dbe::CustomLineEdit::~CustomLineEdit()
{
  if ( Validator != nullptr )
  {
    delete Validator;
  }
}

void dbe::CustomLineEdit::SetNullCheck ( bool IsNull )
{
  NullCheck = IsNull;

  if ( NullCheck == false )
  {
    setPalette ( QApplication::palette ( this ) );
  }
}

void dbe::CustomLineEdit::SetMultiCheck ( bool IsMulti )
{
  IsMultiValue = IsMulti;
}

bool dbe::CustomLineEdit::IsNullCheck()
{
  return NullCheck;
}

bool dbe::CustomLineEdit::IsValid()
{
  return Valid;
}

void dbe::CustomLineEdit::SetCheckDefault ( bool Default )
{
  CheckDefault = Default;
}

void dbe::CustomLineEdit::SetDefaultValue ( QString ValueDefault )
{
  DefaultValue = ValueDefault;
}

void dbe::CustomLineEdit::ValidateText()
{
  QString Dummy ( "Dummy" );
  TryValidate ( Dummy );
}

void dbe::CustomLineEdit::SetLoadedDefaultFlag ( bool Loaded )
{
  setProperty ( "loadedDefaults", Loaded );
}

void dbe::CustomLineEdit::SetValidator ( QValidator * ValidatorSet )
{
  Validator = ValidatorSet;
}

void dbe::CustomLineEdit::SetPopupMenu()
{
  PopupMenu = true;
}

void dbe::CustomLineEdit::CreateActions()
{
  Dec = new QAction ( tr ( "Dec" ), this );
  Dec->setShortcutContext ( Qt::WidgetShortcut );
  connect ( Dec, SIGNAL ( triggered() ), this, SLOT ( EmitDecSlot() ) );
  Dec->setCheckable ( true );
  Dec->setChecked ( false );
  ContextMenu->addAction ( Dec );

  Oct = new QAction ( tr ( "Oct" ), this );
  Oct->setShortcutContext ( Qt::WidgetShortcut );
  connect ( Oct, SIGNAL ( triggered() ), this, SLOT ( EmitOctSlot() ) );
  Oct->setCheckable ( true );
  Oct->setChecked ( false );
  ContextMenu->addAction ( Oct );

  Hex = new QAction ( tr ( "Hex" ), this );
  Hex->setShortcutContext ( Qt::WidgetShortcut );
  Hex->setCheckable ( true );
  Hex->setChecked ( false );
  connect ( Hex, SIGNAL ( triggered() ), this, SLOT ( EmitHexSlot() ) );
  ContextMenu->addAction ( Hex );
}

void dbe::CustomLineEdit::TryValidate ( QString Dummy )
{
  Q_UNUSED ( Dummy )

  QString tmp_st = this->text();
  int i = 0;

  if ( tmp_st.isEmpty() && NullCheck )
  {
    Valid = false;

    if ( !IsMultiValue )
    {
      this->setPalette ( StyleUtility::WarningStatusBarPallete );
      setToolTip ( QString ( "Field cannot be empty!" ) );
    }

    emit StringValidated(); // it is caught by EditStringAttrWidget
  }
  else if ( Validator != 0 )
  {
    if ( ( Validator->validate ( tmp_st, i ) ) == QValidator::Acceptable )
    {
      Valid = true;
      this->setPalette ( QApplication::palette ( this ) );
      setToolTip ( QString ( "This new object ID is unique" ) );
    }
    else if ( ( Validator->validate ( tmp_st, i ) ) == QValidator::Intermediate )
    {
      Valid = false;
      this->setPalette ( StyleUtility::AlertStatusBarPallete );
      setToolTip ( QString ( "This UID is already used" ) );
    }
  }
  else if ( Validator == 0 )
  {
    Valid = true;
    setPalette ( QApplication::palette ( this ) );

    if ( tmp_st == DefaultValue && CheckDefault )
    {
      this->setPalette ( StyleUtility::LoadedDefault );
    }
    else
    {
      this->setPalette ( QApplication::palette ( this ) );
    }

    emit StringValidated();
  }
}

void dbe::CustomLineEdit::EmitDecSlot()
{
  Dec->setChecked ( true );
  Oct->setChecked ( false );
  Hex->setChecked ( false );
  emit DecChange();
}

void dbe::CustomLineEdit::EmitOctSlot()
{
  Dec->setChecked ( false );
  Oct->setChecked ( true );
  Hex->setChecked ( false );
  emit OctChange();
}

void dbe::CustomLineEdit::EmitHexSlot()
{
  Dec->setChecked ( false );
  Oct->setChecked ( false );
  Hex->setChecked ( true );
  emit HexChange();
}

void dbe::CustomLineEdit::contextMenuEvent ( QContextMenuEvent * Event )
{
  if ( !PopupMenu )
  {
    return;
  }

  if ( ContextMenu == nullptr )
  {
    ContextMenu = new QMenu ( this );
    CreateActions();
  }

  ContextMenu->exec ( Event->globalPos() );
}
