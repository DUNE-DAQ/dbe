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
#include "dbe/string_attr_text_edit.hpp"
#include "dbe/StyleUtility.hpp"

dbe::string_attr_text_edit::string_attr_text_edit ( QWidget * parent, bool IsNull, bool IsMulti )
  : QTextEdit ( parent ),
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
  connect ( this, SIGNAL ( textChanged (  ) ), this,
            SLOT ( TryValidate (  ) ) );
  //this->setFixedHeight ( 30 );
  //this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

dbe::string_attr_text_edit::~string_attr_text_edit()
{
  if ( Validator != nullptr )
  {
    delete Validator;
  }
}

void dbe::string_attr_text_edit::SetNullCheck ( bool IsNull )
{
  NullCheck = IsNull;

  if ( NullCheck == false )
  {
    setPalette ( QApplication::palette ( this ) );
  }
}

void dbe::string_attr_text_edit::SetMultiCheck ( bool IsMulti )
{
  IsMultiValue = IsMulti;
}

bool dbe::string_attr_text_edit::IsNullCheck()
{
  return NullCheck;
}

bool dbe::string_attr_text_edit::IsValid()
{
  return Valid;
}

void dbe::string_attr_text_edit::SetCheckDefault ( bool Default )
{
  CheckDefault = Default;
}

void dbe::string_attr_text_edit::SetDefaultValue ( QString ValueDefault )
{
  DefaultValue = ValueDefault;
}

void dbe::string_attr_text_edit::ValidateText()
{
}

void dbe::string_attr_text_edit::SetLoadedDefaultFlag ( bool Loaded )
{
  setProperty ( "loadedDefaults", Loaded );
}

void dbe::string_attr_text_edit::SetValidator ( QValidator * ValidatorSet )
{
  Validator = ValidatorSet;
}

void dbe::string_attr_text_edit::SetPopupMenu()
{
  PopupMenu = true;
}

void dbe::string_attr_text_edit::CreateActions()
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

void dbe::string_attr_text_edit::TryValidate ( )
{
  QString tmp_st = this->toPlainText();
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

void dbe::string_attr_text_edit::EmitDecSlot()
{
  Dec->setChecked ( true );
  Oct->setChecked ( false );
  Hex->setChecked ( false );
  emit DecChange();
}

void dbe::string_attr_text_edit::EmitOctSlot()
{
  Dec->setChecked ( false );
  Oct->setChecked ( true );
  Hex->setChecked ( false );
  emit OctChange();
}

void dbe::string_attr_text_edit::EmitHexSlot()
{
  Dec->setChecked ( false );
  Oct->setChecked ( false );
  Hex->setChecked ( true );
  emit HexChange();
}

void dbe::string_attr_text_edit::contextMenuEvent ( QContextMenuEvent * Event )
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
