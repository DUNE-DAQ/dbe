/// Including DBE
#include "StyleUtility.h"

QColor dbe::StyleUtility::TableColorAttribute;
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
  TableColorRelationship = QColor ( "#AD4713" );
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
