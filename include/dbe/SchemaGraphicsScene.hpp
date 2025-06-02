#ifndef SCHEMAGRAPHICSSCENE_H
#define SCHEMAGRAPHICSSCENE_H

/// Include QT Headers
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QAction>
/// Including Schema Editor
#include "dbe/SchemaGraphicNote.hpp"
#include "dbe/SchemaGraphicObject.hpp"

namespace dbse
{

class SchemaGraphicsScene: public QGraphicsScene
{
  Q_OBJECT
public:
  explicit SchemaGraphicsScene ( QObject * parent = nullptr );
  ~SchemaGraphicsScene();
  void CreateActions();
  /// Drag & Drop
  void dragEnterEvent ( QGraphicsSceneDragDropEvent * event );
  void dragMoveEvent ( QGraphicsSceneDragDropEvent * event );
  void dropEvent ( QGraphicsSceneDragDropEvent * event );

  void contextMenuEvent ( QGraphicsSceneContextMenuEvent * event );
  QStringList AddItemsToScene ( QStringList SchemaClasses, QList<QPointF> Positions );
  void CleanItemMap();
  void RemoveClassObject ( SchemaGraphicObject * Object );
  void add_notes (QStringList notes, QList<QPointF> positions );
  void remove_note_object(SchemaGraphicNote* obj);

  [[nodiscard]] bool IsModified () const {return m_modified;};
  void ClearModified();
signals:
  void sceneModified(bool);
protected:
  // bool event ( QEvent* event );
  void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
  void mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent );
  void mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent );
  void RemoveItemFromScene ( QGraphicsItem* item );
  void modified(bool state);
private slots:
  void new_class_slot();
  void add_class_slot(QString);
  void new_note_slot();
  void add_note_slot(SchemaGraphicNote*);
  void cancel_note_slot(SchemaGraphicNote*);
  void edit_note_slot();
  void remove_note_slot();
  void modified_slot();
  void EditClassSlot();
  void ToggleIndirectInfos();
  void ToggleHighlightActive();
  void AddDirectSuperClassesSlot();
  void AddAllSuperClassesSlot();
  void AddAllSubClassesSlot();
  void AddDirectRelationshipClassesSlot();
  void AddAllRelationshipClassesSlot();
  void RemoveClassSlot();
  void RemoveArrowSlot();
  void DrawArrow ( QString ClassName, QString RelationshipType, QString RelationshipName );
private:
  QMap<QString, SchemaGraphicObject *> ItemMap;
  QGraphicsLineItem * m_line;
  QMenu * m_context_menu;
  int m_seperator_pos;
  int m_class_pos;
  int m_arrow_pos;
  int m_note_pos;
  QAction * m_add_class;
  QAction * m_edit_class;
  QAction * m_toggle_indirect_infos;
  QAction * m_toggle_highlight_active;
  QAction * m_add_direct_super_classes;
  QAction * m_add_direct_relationship_classes;
  QAction * m_add_all_super_classes;
  QAction * m_add_all_sub_classes;
  QAction * m_add_all_relationship_classes;
  QAction * m_add_note;
  QAction * m_edit_note;
  QAction * m_remove_note;
  QAction * m_remove_class;
  QAction * m_remove_arrow;
  QMetaObject::Connection m_addclass_connection;
  SchemaGraphicObject * CurrentObject;
  SchemaGraphicSegmentedArrow * m_current_arrow;
  SchemaGraphicNote* m_current_note;
  QPointF m_current_pos;
  QPointF m_mouse_item_pos;
  int m_next_note{0};
  bool m_inherited_properties_visible;
  bool m_highlight_active;
  bool m_modified;
};

}  // namespace dbse
#endif // SCHEMAGRAPHICSSCENE_H
