#ifndef DBE_SCHEMAFILEINFO_H
#define DBE_SCHEMAFILEINFO_H

#include <QDialog>
#include <QKeyEvent>
#include <QList>
#include <QListWidgetItem>
#include <QMenu>
#include <QPushButton>
#include <QUrl>
#include <set>
#include <string>

namespace dunedaq::oks {
  class OksClass;
} // namespace dunedaq::oks

namespace dbse
{
  namespace Ui
  {
    class SchemaFileInfo;
  }  // namespace Ui

  class SchemaFileInfo : public QWidget {
    Q_OBJECT
  public:
    SchemaFileInfo(std::string filename, QWidget* parent=0);
    ~SchemaFileInfo() = default;

    void show_file_info(QString fn);
    void keyPressEvent(QKeyEvent* event) override;
  signals:
    void files_updated();
    void new_window(SchemaFileInfo* win);
  private slots:
    void accept();
    void reject();
//    void close();
    void add_include();
    void remove_include();
    void set_active();
    void add_missing_includes();
    void launch_class_editor(QListWidgetItem*);
    void show_file_info();
    void show_file_info(QListWidgetItem*);
    void activate_include_context_menu(QPoint point);
    void activate_class_context_menu(QPoint point);
    void add_new_class();
    void edit_class();
    void remove_class();
    void set_schemafile_active();
    void show_status();
  private:
    void get_includes();
    void add_file(std::string file);
    bool check_relationships(dunedaq::oks::OksClass* cls);
    bool check_superclasses(dunedaq::oks::OksClass* cls);
    std::string prune_path(std::string file);
    bool set_active(std::string filename);
    void remove_include(std::string filename);

    Ui::SchemaFileInfo* m_ui;
    std::string m_filename;
    std::set<std::string> m_missing_includes;
    std::set<std::string> m_all_includes;
    QMenu* m_include_menu;
    QMenu* m_class_menu;

    QStringList m_path_list;
    QList<QUrl> m_path_urls;
    QPushButton* m_add_button;
    QPushButton* m_missing_button;
  };
}
#endif // DBE_SCHEMAFILEINFO_H
