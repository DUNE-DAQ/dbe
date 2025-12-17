#ifndef DBE_FILEINFO_H
#define DBE_FILEINFO_H

#include "dbe/tref.hpp"
#include "dbe/dbcontroller.hpp"

#include <QDialog>
#include <QKeyEvent>
#include <QString>
#include <QListWidgetItem>

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

    static void show_file_info(QString filename);

  private slots:
    void accept();
    void reject();
    void file_info_slot (QString fn);
    void file_info_slot(QListWidgetItem*);
    void edit_object_slot(QListWidgetItem*);

  private:
    Ui::FileInfo* m_ui;
    std::map<QString, const tref> m_obj_map;
  };
} //namespace dbe
#endif // DBE_FILEINFO_H
