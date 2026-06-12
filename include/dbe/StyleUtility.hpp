/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/StyleUtility.h to include/dbe/StyleUtility.hpp).
 */

#ifndef STYLEUTILITY_H
#define STYLEUTILITY_H

/// Including DBE
#include <QColor>
#include <QPalette>

namespace dbe
{

class StyleUtility
{
public:
  StyleUtility();
  static void InitColorManagement();
  static QColor TableColorAttribute;
  static QColor TableAttributeBackground;
  static QColor TableAttributeHighlightBackground;
  static QColor TableColorRelationship;

  static QColor FileReadOnlyForeground;
  static QColor FileReadOnlyBackground;

  static QPalette AlertStatusBarPallete;
  static QPalette AlertStatusBarPalleteWindow;
  static QPalette WarningStatusBarPallete;
  static QPalette WarningStatusBarPalleteWindow;
  static QPalette PaleGreenPalleteButton;
  static QPalette LoadedDefault;
};

}  // namespace dbe
#endif // STYLEUTILITY_H
