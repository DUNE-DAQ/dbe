/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from src/SchemaEditor/SchemaTab.cpp to apps/SchemaEditor/SchemaTab.cpp).
 */

/// Including QT Headers
#include <QVBoxLayout>
/// Including Schema Editor
#include "dbe/SchemaTab.hpp"

dbse::SchemaTab::SchemaTab ( QWidget * parent )
  : QWidget ( parent )
{
  /// Generating view/schema
  GraphView = new QGraphicsView();
  GraphScene = new SchemaGraphicsScene(this);
  GraphView->setScene ( GraphScene );
  GraphView->centerOn ( 0, 0 );
  /// Adjusting layout
  QVBoxLayout * TabLayout = new QVBoxLayout ( this );
  TabLayout->addWidget ( GraphView );
}

dbse::SchemaGraphicsScene * dbse::SchemaTab::GetScene() const
{
  return GraphScene;
}

QGraphicsView * dbse::SchemaTab::GetView() const
{
  return GraphView;
}

void dbse::SchemaTab::setName(const QString& name) {
  m_name = name;
}

void dbse::SchemaTab::setFileName(const QString& name) {
  m_file_name = name;
}

QString dbse::SchemaTab::getFileName() {
  if (m_file_name == "") {
    QString fn = "./";
    fn.append(m_name);
    //fn.append(".view");
    return fn;
  }
  else {
    return m_file_name;
  }
}

dbse::SchemaTab::~SchemaTab() = default;
