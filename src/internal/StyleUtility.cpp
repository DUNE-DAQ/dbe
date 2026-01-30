/// Including DBE
#include "dbe/StyleUtility.hpp"

QColor dbe::StyleUtility::TableColorAttribute;
QColor dbe::StyleUtility::TableAttributeBackground;

QColor dbe::StyleUtility::FileReadOnlyForeground;
QColor dbe::StyleUtility::FileReadOnlyBackground;

QColor dbe::StyleUtility::TableColorRelationship;
QColor dbe::StyleUtility::TableRelationshipBackground;

QColor dbe::StyleUtility::ObjectForeground;
QColor dbe::StyleUtility::ObjectBackground;

QColor dbe::StyleUtility::DefaultValueBackground;

QPalette dbe::StyleUtility::AlertStatusBarPallete;
QPalette dbe::StyleUtility::AlertStatusBarPalleteWindow;
QPalette dbe::StyleUtility::WarningStatusBarPallete;
QPalette dbe::StyleUtility::WarningStatusBarPalleteWindow;
QPalette dbe::StyleUtility::PaleGreenPalleteButton;
QPalette dbe::StyleUtility::LoadedDefault;


void dbe::StyleUtility::InitColorManagement()
{
  QSettings settings;

  settings.beginGroup("attribute");
  if (!settings.contains("foreground")) {
    settings.setValue("foreground", QColor ( "#1B676B" ));
  }
  if (!settings.contains("background")) {
    settings.setValue("background", QColor ( "#ffffff" ));
  }
  if (!settings.contains("default-background")) {
    settings.setValue("default-background", QColor ("#b8f4ff" ));
  }

  DefaultValueBackground = settings.value("default-background").value<QColor>();
  TableColorAttribute = settings.value("foreground").value<QColor>();
  TableAttributeBackground = settings.value("background").value<QColor>();
  settings.endGroup();

  settings.beginGroup("relationship");
  if (!settings.contains("foreground")) {
    settings.setValue("foreground", QColor ( "#AD4713" ));
  }
  if (!settings.contains("background")) {
    settings.setValue("background", QColor ( Qt::white ));
  }
  TableColorRelationship = settings.value("foreground").value<QColor>();
  TableRelationshipBackground = settings.value("background").value<QColor>();
  settings.endGroup();


  settings.beginGroup("readonly");
  if (!settings.contains("background")) {
    settings.setValue("background", QColor(Qt::lightGray));
  }
  if (!settings.contains("foreground")) {
    settings.setValue("foreground", QColor(Qt::darkRed));
  }
  FileReadOnlyForeground = settings.value("foreground").value<QColor>();
  FileReadOnlyBackground = settings.value("background").value<QColor>();
  settings.endGroup();

  settings.beginGroup("object");
  if (!settings.contains("foreground")) {
    settings.setValue("foreground", QColor(Qt::blue));
  }
  if (!settings.contains("background")) {
    settings.setValue("background", QColor(Qt::white));
  }
  ObjectForeground =  settings.value("foreground").value<QColor>();
  ObjectBackground =  settings.value("background").value<QColor>();
  settings.endGroup();


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
  LoadedDefault.setColor ( QPalette::Active, QPalette::Base, DefaultValueBackground );
  LoadedDefault.setColor ( QPalette::Inactive, QPalette::Base, DefaultValueBackground );

  PaleGreenPalleteButton.setColor ( QPalette::Active, QPalette::Button,
                                    QColor::fromRgb ( 190, 238, 158 ) );
  PaleGreenPalleteButton.setColor ( QPalette::Inactive, QPalette::Button,
                                    QColor::fromRgb ( 190, 238, 158 ) );
}
