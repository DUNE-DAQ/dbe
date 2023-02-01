#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H

#include "confaccessor.h"
#include <QItemDelegate>
#include <QPainter>
#include <QModelIndex>

namespace dbe
{
class CustomDelegate: public QItemDelegate
{
  Q_OBJECT
public:
  explicit CustomDelegate ( QObject * parent = 0 );

  QWidget * createEditor ( QWidget * parent, const QStyleOptionViewItem & option,
                           const QModelIndex & index ) const;

  void setEditorData ( QWidget * editor, const QModelIndex & index ) const;

  void setModelData ( QWidget * editor, QAbstractItemModel * model,
                      const QModelIndex & index ) const;

  void updateEditorGeometry ( QWidget * editor, const QStyleOptionViewItem & option,
                              const QModelIndex & index ) const;

  bool eventFilter ( QObject * editor, QEvent * event );

  bool editorEvent ( QEvent * event, QAbstractItemModel * model,
                     const QStyleOptionViewItem & option, const QModelIndex & index );

  void paint ( QPainter * painter, const QStyleOptionViewItem & option,
               const QModelIndex & index ) const;

private slots:
  void CommitAndClose();
  void Close();

signals:
  void CreateObjectEditorSignal ( tref Object ) const;
};
} // end namespace dbe
#endif // CUSTOMDELEGATE_H
