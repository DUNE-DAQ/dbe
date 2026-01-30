#ifndef STYLEUTILITY_H
#define STYLEUTILITY_H

/// Including DBE
#include <QColor>
#include <QPalette>
#include <QSettings>

namespace dbe
{

class StyleUtility
{
public:
  StyleUtility();
  static void InitColorManagement();
  static QColor TableColorAttribute;
  static QColor TableAttributeBackground;

  static QColor TableColorRelationship;
  static QColor TableRelationshipBackground;

  static QColor DefaultValueBackground;

  static QColor FileReadOnlyForeground;
  static QColor FileReadOnlyBackground;

  static QColor ObjectForeground;
  static QColor ObjectBackground;

  static QPalette AlertStatusBarPallete;
  static QPalette AlertStatusBarPalleteWindow;
  static QPalette WarningStatusBarPallete;
  static QPalette WarningStatusBarPalleteWindow;
  static QPalette PaleGreenPalleteButton;
  static QPalette LoadedDefault;
};

}  // namespace dbe
#endif // STYLEUTILITY_H
