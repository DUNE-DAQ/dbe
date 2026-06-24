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
#include <set>

namespace dbe
{
  namespace Ui
  {
    class FileInfo;
  }  // namespace Ui


  class FileInfo {
  public:
    explicit FileInfo(const QString& filename);

    const QString& name(){return m_filename;};
    const QString& short_name(){return m_short_name;};

    static void setup_paths();
    static QList<QUrl> get_path_urls();
    static QStringList get_path_list();
    static QString prune_path(QString file);

    static const QString& data_path();
    static void update_data_path(const QString& path);
    static const QString& schema_path();
    static void update_schema_path(const QString& path);
    static bool match_path(const QString& file, 
                           const QString& top_file,
                           const QStringList& includes);
    bool check_includes();
    const QString& message() const {return m_message;};
    const std::map<QString, const tref>& objects() const {return m_objects;};

    const std::set<QString>& missing_schema() const {return m_missing_schema;};
    const std::set<QString>& missing_data() const {return m_missing_data;};

    void parse_objects();

  private:
    static QStringList s_path_list;
    static QList<QUrl> s_path_urls;
    static QString s_schema_path;
    static QString s_data_path;

    QString m_filename;
    QString m_short_name;
    QString m_message;
    std::map<QString, const tref> m_objects;
    std::set<QString> m_missing_schema;
    std::set<QString> m_missing_data;
  };


  class FileInfoWidget : public QWidget {
    Q_OBJECT
  public:
    FileInfoWidget(QString filename, QWidget* parent=0);
    ~FileInfoWidget() = default;

    void keyPressEvent(QKeyEvent* event) override;

    static void show_file_info(const QString& filename);

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
    void add_missing_datafiles();
    void add_missing_schemafiles();

    void remove_datafile_slot();
    void remove_schemafile_slot();
    void remove_includefile(const QString& file);

    void filemodel_updated();
  private:
    void parse_objects();
    void parse_includes();

    Ui::FileInfo* m_ui;

    FileInfo m_file;
    bool m_readonly{false};
    QMenu* m_schema_menu{nullptr};
    QMenu* m_data_menu{nullptr};
    QMenu* m_object_menu{nullptr};
    QUuid const m_uuid;

    bool m_updating{false};
  };

} //namespace dbe
#endif // DBE_FILEINFO_H
