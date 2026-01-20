#ifndef DBE_FILEINFO_H
#define DBE_FILEINFO_H

#include "dbe/tref.hpp"
#include "dbe/dbcontroller.hpp"

#include <QFileDialog>
#include <QKeyEvent>
#include <QString>
#include <QListWidgetItem>
#include <QUuid>
#include <QWidget>

#include <map>

namespace dbe
{
  namespace Ui
  {
    class FileInfo;
  }  // namespace Ui

  class FileInfo : public QWidget {
    Q_OBJECT
  public:
    FileInfo(QString filename, QWidget* parent=0);
    ~FileInfo() = default;

    void keyPressEvent(QKeyEvent* event) override;

    static void show_file_info(const QString& filename);
    static void setup_paths();
    static QList<QUrl> get_path_urls();
    static QStringList get_path_list();
    static QString prune_path(QString file);
    static void parse_all_objects();
    static QString check_file_includes(const QString& file);

  private slots:
    void accept();
    void reject();
    void file_info_slot ();
    void file_info_slot (const QString& fn);
    void file_info_slot(QListWidgetItem*);
    void edit_object_slot();
    void edit_object_slot(QListWidgetItem*);
    void delete_object_slot();
    void rename_object_slot();
    void activate_schema_context_menu(QPoint point);
    void activate_data_context_menu(QPoint point);
    void activate_object_context_menu(QPoint point);

    void add_datafile();
    void add_schemafile();
    void add_includefile(QFileDialog* fd);

    void remove_datafile_slot();
    void remove_schemafile_slot();
    void remove_includefile(const QString& file);

    void filemodel_updated();
  private:
    void parse_objects();
    void parse_includes();
    bool check_includes();
    Ui::FileInfo* m_ui;
    std::map<QString, const tref> m_obj_map;
    QString m_filename;
    bool m_readonly{false};
    QMenu* m_schema_menu{nullptr};
    QMenu* m_data_menu{nullptr};
    QMenu* m_object_menu{nullptr};
    QUuid const m_uuid;

    static std::map<QString, std::map<QString, const tref>> s_obj_map;

    static QStringList s_path_list;
    static QList<QUrl> s_path_urls;
    static QString s_schema_path;
    static QString s_data_path;
  };
} //namespace dbe
#endif // DBE_FILEINFO_H
