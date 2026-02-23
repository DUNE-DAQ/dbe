#ifndef DBE_SCHEMASETTINGS_H
#define DBE_SCHEMASETTINGS_H

#include <QDialog>
#include <QKeyEvent>
#include <QList>
#include <QListWidgetItem>

namespace Ui
{
  class SchemaSettings;
}  // namespace Ui

namespace dbse
{
  class SchemaSettings : public QDialog {
    Q_OBJECT
  public:
    explicit SchemaSettings(QWidget* parent=0);
    ~SchemaSettings() = default;
  signals:
    void settings_updated();
  private slots:
    void set_color(QListWidgetItem*);
    void set_font(QListWidgetItem*);
    void toggle_abstract(int);
    void toggle_active(int);
    void toggle_inherited(int);
    void toggle_default(int);
    void toggle_save(int);
  private:
    Ui::SchemaSettings* m_ui;
  };
} //namespace dbse

#endif // DBE_SCHEMASETTINGS_H
