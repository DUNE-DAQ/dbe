#ifndef BATCHCHANGEWIDGET_H
#define BATCHCHANGEWIDGET_H

#include "dbe/confaccessor.hpp"

#include <QWidget>
#include <QTableWidget>

#include <memory>


namespace dbe
{

namespace Ui
{
class BatchChangeWidget;
}  // namespace Ui

class BatchChangeWidget: public QWidget
{
  Q_OBJECT

public:
  ~BatchChangeWidget();

  explicit BatchChangeWidget ( QWidget * parent = nullptr );

  BatchChangeWidget ( bool ObjectsFromTable, QString ClassName,
                      std::vector<dref> & Objects, QWidget * parent = nullptr );

private:
  std::unique_ptr<dbe::Ui::BatchChangeWidget> ui;
  std::vector<dref> TableObjects;

  bool UseTable, ClassChanged;

  QString TableClass;
  std::unique_ptr<QTableWidget> m_filter_table;

  void SetController();
  void filter ( std::vector<dref> & Objects, const QString & ClassName );
  void keyPressEvent(QKeyEvent* event) override;

private slots:
  void FillInfo ( const QString & Name );

  void MakeChanges();
  void FindMatching();

  void EnableCheckBox ( QString RelationshipName );

  void UpdateRelationshipFilter ( int );
  void UpdateRelationshipNewValues ( int );

signals:
  void sig_batch_change_start();
  void sig_batch_change_stop(const QList<QPair<QString, QString>>&);
};

}  // namespace dbe

#endif // BATCHCHANGEWIDGET_H
