/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/SchemaTab.h to include/dbe/SchemaTab.hpp).
 */

#ifndef SCHEMATAB_H
#define SCHEMATAB_H

/// Include QT Headers
#include <QWidget>
#include <QGraphicsView>
/// Include Schema Editor
#include "dbe/SchemaGraphicsScene.hpp"

namespace dbse
{

class SchemaTab: public QWidget
{
  Q_OBJECT
public:
  explicit SchemaTab ( QWidget * parent = nullptr );
  ~SchemaTab();
  SchemaGraphicsScene * GetScene() const;
  QGraphicsView * GetView() const;

  void setName(const QString&);
  QString getName() {return m_name;};
  void setFileName(const QString&);
  QString getFileName ();
private:
  QGraphicsView * GraphView;
  SchemaGraphicsScene * GraphScene;
  QString m_name{""};
  QString m_file_name{};
};

}  // namespace dbse
#endif // SCHEMATAB_H
