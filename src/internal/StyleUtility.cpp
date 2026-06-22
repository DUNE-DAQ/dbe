// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/// Including DBE
#include "dbe/StyleUtility.hpp"

QColor dbe::StyleUtility::TableColorAttribute;
QColor dbe::StyleUtility::TableAttributeBackground;
QColor dbe::StyleUtility::TableAttributeHighlightBackground;
QColor dbe::StyleUtility::FileReadOnlyForeground;
QColor dbe::StyleUtility::FileReadOnlyBackground;

QColor dbe::StyleUtility::TableColorRelationship;
QPalette dbe::StyleUtility::AlertStatusBarPallete;
QPalette dbe::StyleUtility::AlertStatusBarPalleteWindow;
QPalette dbe::StyleUtility::WarningStatusBarPallete;
QPalette dbe::StyleUtility::WarningStatusBarPalleteWindow;
QPalette dbe::StyleUtility::PaleGreenPalleteButton;
QPalette dbe::StyleUtility::LoadedDefault;


void dbe::StyleUtility::InitColorManagement()
{
  TableColorAttribute = QColor ( "#1B676B" );
  TableAttributeBackground = QColor ( "#ffffff" );
  TableAttributeHighlightBackground = QColor ( "#f0f0ff" );
  TableColorRelationship = QColor ( "#AD4713" );

  FileReadOnlyForeground = QColor ("#a00000");
  FileReadOnlyBackground = QColor ("#f0f0f0");


  AlertStatusBarPallete.setColor ( QPalette::Active, QPalette::Base, QColor ( "red" ) );
  AlertStatusBarPallete.setColor ( QPalette::Inactive, QPalette::Base, QColor ( "red" ) );
  WarningStatusBarPallete.setColor ( QPalette::Active, QPalette::Base, QColor ( "yellow" ) );
  WarningStatusBarPallete.setColor ( QPalette::Inactive, QPalette::Base,
                                     QColor ( "yellow" ) );
  AlertStatusBarPalleteWindow.setColor ( QPalette::Active, QPalette::Window,
                                         QColor ( "red" ) );
  AlertStatusBarPalleteWindow.setColor ( QPalette::Inactive, QPalette::Window,
                                         QColor ( "red" ) );
  WarningStatusBarPalleteWindow.setColor ( QPalette::Active, QPalette::Window,
                                           QColor ( "yellow" ) );
  WarningStatusBarPalleteWindow.setColor ( QPalette::Inactive, QPalette::Window,
                                           QColor ( "yellow" ) );
  LoadedDefault.setColor ( QPalette::Active, QPalette::Base, QColor::fromRgb ( 184, 244,
                                                                               255 ) );
  LoadedDefault.setColor ( QPalette::Inactive, QPalette::Base, QColor::fromRgb ( 184, 244,
                                                                                 255 ) );
  PaleGreenPalleteButton.setColor ( QPalette::Active, QPalette::Button,
                                    QColor::fromRgb ( 190, 238, 158 ) );
  PaleGreenPalleteButton.setColor ( QPalette::Inactive, QPalette::Button,
                                    QColor::fromRgb ( 190, 238, 158 ) );
}
