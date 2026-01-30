#ifndef DBE_PREFERENCES_H
#define DBE_PREFERENCES_H

#include <QDialog>
#include <QKeyEvent>
#include <QList>
#include <QListWidgetItem>

//#include "ui_Preferences.h"

namespace dbe
{
namespace Ui
{
  class Preferences;
}  // namespace Ui

  class Preferences : public QDialog {
    Q_OBJECT
  public:
    explicit Preferences(QWidget* parent=0);
    ~Preferences() = default;
  // signals:
  //   void settings_updated();
  private slots:
    void set_color(QListWidgetItem*);

  private:
    Ui::Preferences* m_ui;
  };
} //namespace dbe

#endif // DBE_PREFERENCES_H
