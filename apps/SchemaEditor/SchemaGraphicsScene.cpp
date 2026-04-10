/// Including QT Headers
#include <QGraphicsSceneDragDropEvent>
#include <QEvent>
#include <QSettings>
#include <QMimeData>

#include <QWidget>

#include <QMenu>
#include <QApplication>
/// Including Schema Editor
#include "dbe/SchemaGraphicsScene.hpp"
#include "dbe/SchemaGraphicNote.hpp"
#include "dbe/SchemaGraphicObject.hpp"
#include "dbe/SchemaGraphicSegmentedArrow.hpp"
#include "dbe/SchemaKernelWrapper.hpp"
#include "dbe/SchemaNoteEditor.hpp"
#include "dbe/SchemaClassEditor.hpp"
#include "dbe/SchemaRelationshipEditor.hpp"

using namespace dunedaq::oks;

dbse::SchemaGraphicsScene::SchemaGraphicsScene ( QObject * parent )
  : QGraphicsScene ( parent ),
    m_line ( nullptr ),
    m_context_menu ( nullptr ),
    CurrentObject ( nullptr ),
    m_current_arrow ( nullptr ),
    m_modified(false)
{
  setSceneRect ( QRectF ( 0, 0, 10000, 10000 ) );

  QSettings settings;
  settings.beginGroup("view defaults");

  m_inherited_properties_visible = settings.value("show_inherited", false).toBool();
  m_highlight_abstract = settings.value("highlight_abstract", false).toBool();
  m_highlight_active = settings.value("highlight_active", false).toBool();
  m_show_defaults = settings.value("show_default", false).toBool();

  CreateActions();
}

dbse::SchemaGraphicsScene::~SchemaGraphicsScene()
{
}

void dbse::SchemaGraphicsScene::CreateActions()
{
  // Add new class
  m_add_class = new QAction ( "&Add new class", this );
  connect ( m_add_class, SIGNAL ( triggered() ), this, SLOT ( new_class_slot() ) );

  // Edit current class
  m_edit_class = new QAction ( "&Edit class", this );
  connect ( m_edit_class, SIGNAL ( triggered() ), this, SLOT ( EditClassSlot() ) );

  // Toggle inherited properties of all classes in view
  m_toggle_indirect_infos = new QAction ( "Show &inherited properties", this );
  m_toggle_indirect_infos->setCheckable(true);
  m_toggle_indirect_infos->setChecked(m_inherited_properties_visible);
  connect ( m_toggle_indirect_infos, SIGNAL ( triggered() ), this, SLOT ( ToggleIndirectInfos() ) );

  // Toggle highlighting of all classes in active schema
  m_toggle_highlight_active = new QAction ( "&Highlight classes in active schema", this );
  m_toggle_highlight_active->setCheckable(true);
  m_toggle_highlight_active->setChecked(m_highlight_active);
  connect ( m_toggle_highlight_active, SIGNAL ( triggered() ), this, SLOT ( ToggleHighlightActive() ) );

  // Toggle highlighting of all abstract classes in view
  m_toggle_highlight_abstract = new QAction ( "&Highlight abstract classes in view", this );
  m_toggle_highlight_abstract->setCheckable(true);
  m_toggle_highlight_abstract->setChecked(m_highlight_abstract);
  connect ( m_toggle_highlight_abstract, SIGNAL ( triggered() ), this, SLOT ( ToggleHighlightAbstract() ) );

  // Toggle displaying default values of attributes
  m_toggle_default = new QAction ( "Show &default values of attributes", this );
  m_toggle_default->setCheckable(true);
  m_toggle_default->setChecked(m_show_defaults);
  connect ( m_toggle_default, SIGNAL ( triggered() ), this, SLOT ( ToggleDefault() ) );

  // Toggle highlighting of current class
  m_toggle_highlight_class = new QAction ( "&Highlight this class", this );
  m_toggle_highlight_class->setCheckable(true);
  connect ( m_toggle_highlight_class, SIGNAL ( triggered() ), this, SLOT ( ToggleHighlightClass() ) );

  m_add_note = new QAction ( "&Add note to view", this );
  connect ( m_add_note, SIGNAL ( triggered() ), this, SLOT ( new_note_slot() ) );

  m_edit_note = new QAction ( "&Edit note", this );
  connect ( m_edit_note, SIGNAL ( triggered() ), this, SLOT ( edit_note_slot() ) );

  m_remove_note = new QAction ( "&Remove note", this );
  connect ( m_remove_note, SIGNAL ( triggered() ), this, SLOT ( remove_note_slot() ) );

  // Show superclasses of the current class
  m_add_direct_super_classes = new QAction ( "Add direct &superclasses to view", this );
  connect ( m_add_direct_super_classes, SIGNAL ( triggered() ), this, SLOT ( AddDirectSuperClassesSlot() ) );

  // Show relationship classes of the current clas
  m_add_direct_relationship_classes = new QAction ( "Add direct &relationship classes to view", this );
  connect ( m_add_direct_relationship_classes, SIGNAL ( triggered() ), this, SLOT ( AddDirectRelationshipClassesSlot() ) );
  
  // Show superclasses of the current class
  m_add_all_super_classes = new QAction ( "Add all &superclasses to view", this );
  connect ( m_add_all_super_classes, SIGNAL ( triggered() ), this, SLOT ( AddAllSuperClassesSlot() ) );

  // Show subclasses of the current clas
  m_add_all_sub_classes = new QAction ( "Add all s&ubclasses to view", this );
  connect ( m_add_all_sub_classes, SIGNAL ( triggered() ), this, SLOT ( AddAllSubClassesSlot() ) );

  // Show indirect relationship classes of the current class
  m_add_all_relationship_classes = new QAction ( "Add all &relationship classes to view", this );
  connect ( m_add_all_relationship_classes, SIGNAL ( triggered() ), this, SLOT ( AddAllRelationshipClassesSlot() ) );

  // Remove class
  m_remove_class = new QAction ( "&Remove Class from view", this );
  connect ( m_remove_class, SIGNAL ( triggered() ), this, SLOT ( RemoveClassSlot() ) );

  // Remove arrow
  m_remove_arrow = new QAction ( "&Remove Arrow", this );
  connect ( m_remove_arrow, SIGNAL ( triggered() ), this, SLOT ( RemoveArrowSlot() ) );

  m_save = new QAction("&Save view", this);
  connect(m_save, SIGNAL(triggered()), this, SLOT(requestSave()));

  m_move = new QAction("&Move scene", this);
  connect(m_move, SIGNAL(triggered()), this, SLOT(moveScene()));
}

void dbse::SchemaGraphicsScene::dragEnterEvent ( QGraphicsSceneDragDropEvent * event )
{
  if ( event->mimeData()->hasFormat ( "application/vnd.text.list" ) )
  {
    event->accept();
  }
}

void dbse::SchemaGraphicsScene::dragMoveEvent ( QGraphicsSceneDragDropEvent * event )
{
  if ( event->mimeData()->hasFormat ( "application/vnd.text.list" ) )
  {
    event->accept();
  }
}

void dbse::SchemaGraphicsScene::dropEvent ( QGraphicsSceneDragDropEvent * event )
{
  QByteArray encodedData = event->mimeData()->data ( "application/vnd.text.list" );
  QDataStream stream ( &encodedData, QIODevice::ReadOnly );

  if (stream.atEnd()) {
    return;
  }

  QStringList schema_classes;
  while ( !stream.atEnd() )
  {
    QString class_name;
    stream >> class_name;
    schema_classes.append ( class_name );
  }

  QList<QPointF> positions;
  for ( int i = 0; i < schema_classes.size(); ++i )
  {
    positions.push_back ( event->scenePos() );
  }

  AddItemsToScene ( schema_classes, positions );
}

void dbse::SchemaGraphicsScene::contextMenuEvent ( QGraphicsSceneContextMenuEvent * event )
{
  if ( m_context_menu == nullptr ) {
    m_context_menu = new QMenu();
    m_context_menu->addAction ( m_add_class );
    m_context_menu->addAction ( m_add_note );
    m_context_menu->addAction ( m_toggle_indirect_infos );
    m_context_menu->addAction ( m_toggle_highlight_abstract );
    m_context_menu->addAction ( m_toggle_highlight_active );
    m_context_menu->addAction ( m_toggle_default );

    m_seperator_pos = m_context_menu->actions().size();
    m_context_menu->addSeparator();

    m_class_pos = m_context_menu->actions().size();
    m_context_menu->addAction ( m_edit_class );
    m_context_menu->addAction ( m_remove_class );
    m_context_menu->addAction ( m_toggle_highlight_class );
    m_context_menu->addSeparator();

    m_context_menu->addAction ( m_add_direct_super_classes );
    m_context_menu->addAction ( m_add_direct_relationship_classes );
    m_context_menu->addAction ( m_add_all_super_classes );
    m_context_menu->addAction ( m_add_all_sub_classes );
    m_context_menu->addAction ( m_add_all_sub_classes );
    m_context_menu->addAction ( m_add_all_relationship_classes );

    m_arrow_pos = m_context_menu->actions().size();
    m_context_menu->addAction ( m_remove_arrow );

    m_note_pos = m_context_menu->actions().size();
    m_context_menu->addAction ( m_edit_note );
    m_context_menu->addAction ( m_remove_note );

    m_context_menu->addSeparator();
    m_save_pos = m_context_menu->actions().size();
    m_context_menu->addAction(m_save);
    m_context_menu->addAction(m_move);
  }

  bool active = KernelWrapper::GetInstance().IsActive ( );
  m_context_menu->actions().at ( 0 )->setVisible ( active );

  for (int item=1; item<m_seperator_pos; item++) {
    m_context_menu->actions().at ( item )->setVisible ( true );
  }

  // Set all other items invisible
  const auto nitems = m_context_menu->actions().size();
  for (int item=m_seperator_pos; item<nitems; item++) {
    m_context_menu->actions().at ( item )->setVisible ( false );
  }

  if (ItemMap.size()>0) {
    m_context_menu->actions().at(m_save_pos-1)->setVisible(true);
    m_context_menu->actions().at(m_save_pos+1)->setVisible(true);
  }
  if (m_modified && ItemMap.size()>0) {
    m_context_menu->actions().at(m_save_pos)->setVisible(true);
  }

  if ( itemAt ( event->scenePos(), QTransform() ) ) {
    // Something under mouse pointer, set additional items visible
    // depending on what it is
    m_context_menu->actions().at ( m_seperator_pos )->setVisible ( true );

    auto object = dynamic_cast<SchemaGraphicObject *> (
      itemAt ( event->scenePos(), QTransform() ) );
    auto arrow = dynamic_cast<SchemaGraphicSegmentedArrow *> (
      itemAt ( event->scenePos(), QTransform() ) );
    auto note = dynamic_cast<SchemaGraphicNote *> (
      itemAt ( event->scenePos(), QTransform() ) );

    if ( object != nullptr) {
      CurrentObject = object;
      m_toggle_highlight_class->setChecked(CurrentObject->highlighted());

      auto filename =
        CurrentObject->GetClass()->get_file()->get_full_file_name();
      bool writable = KernelWrapper::GetInstance().IsFileWritable ( filename );
      m_context_menu->actions().at ( m_class_pos )->setVisible ( writable );

      for (int item=m_class_pos+1; item<m_arrow_pos; item++) {
        m_context_menu->actions().at ( item )->setVisible ( true );
      }
    }
    else if ( arrow != nullptr ) {
      m_context_menu->actions().at ( m_arrow_pos )->setVisible ( true );
      m_current_arrow = arrow;
    }
    else if ( note != nullptr) {
      for (int item=m_note_pos; item<nitems; item++) {
        m_context_menu->actions().at ( item )->setVisible ( true );
      }
      m_current_note = note;
    }
  }
  m_current_pos = event->scenePos();
  m_context_menu->exec ( event->screenPos() );
}

QStringList dbse::SchemaGraphicsScene::AddItemsToScene (
  QStringList SchemaClasses,
  QList<QPointF> Positions )
{
  QStringList missingItems{};

  for ( QString & ClassName : SchemaClasses )
  {
    if ( !ItemMap.contains ( ClassName ) )
    {

      if ( !KernelWrapper::GetInstance().FindClass ( ClassName.toStdString() ) ) {
          std::cout << "ERROR: class " << ClassName.toStdString()  << " not found" << std::endl;
          missingItems.append(ClassName);
          continue;
      }

      SchemaGraphicObject * Object = new SchemaGraphicObject ( ClassName, this );
      Object->setPos ( Positions.at ( SchemaClasses.indexOf ( ClassName ) ) );
      addItem ( Object );
      /// Updating item list
      ItemMap.insert ( ClassName, Object );
    }
  }

  for ( QString & ClassName : ItemMap.keys() )
  {
    OksClass * ClassInfo = KernelWrapper::GetInstance().FindClass ( ClassName.toStdString() );

    const std::list<OksRelationship *> * DirectRelationshipList =
      ClassInfo->direct_relationships();
    const std::list<std::string *> * DirectSuperClassesList = ClassInfo->direct_super_classes();

    std::map<std::string, unsigned int> arrow_count;

    //// PLotting relationships
    if ( DirectRelationshipList != nullptr )
    {
      for ( OksRelationship * ClassRelationship : * ( DirectRelationshipList ) )
      {
        auto rct = ClassRelationship->get_class_type()->get_name();
        QString RelationshipClassType = QString::fromStdString (rct);

        if ( ItemMap.contains ( RelationshipClassType ) ) //&& !ItemMap[ClassName]->HasArrow (
          //ItemMap[RelationshipClassType] ) )
        {
          QString SchemaCardinality =
            KernelWrapper::GetInstance().GetCardinalityStringRelationship ( ClassRelationship ) + " ";
          SchemaGraphicSegmentedArrow * NewArrow = new SchemaGraphicSegmentedArrow (
            ItemMap[ClassName], ItemMap[RelationshipClassType],
            arrow_count[rct],
            false,
            ClassRelationship->get_is_composite(),
            QString::fromStdString ( ClassRelationship->get_name() ), SchemaCardinality );
          ItemMap[ClassName]->AddArrow ( NewArrow );
          ItemMap[RelationshipClassType]->AddArrow ( NewArrow );
          addItem ( NewArrow );
          //NewArrow->SetLabelScene(this);
          NewArrow->setZValue ( -1000.0 );
          NewArrow->UpdatePosition();
          arrow_count[rct]++;
        }
      }
    }

    /// Plotting the superclasses
    if ( DirectSuperClassesList != nullptr )
    {
      for ( std::string * SuperClassNameStd : * ( DirectSuperClassesList ) )
      {
        QString SuperClassName = QString::fromStdString ( *SuperClassNameStd );

        if ( ItemMap.contains ( SuperClassName ) ) // && !ItemMap[ClassName]->HasArrow (
          // ItemMap[SuperClassName] ) )
        {
          SchemaGraphicSegmentedArrow * NewArrow = new SchemaGraphicSegmentedArrow (
            ItemMap[ClassName],
            ItemMap[SuperClassName],
            arrow_count[*SuperClassNameStd],
            true,
            false, "", "" );
          ItemMap[ClassName]->AddArrow ( NewArrow );
          ItemMap[SuperClassName]->AddArrow ( NewArrow );
          addItem ( NewArrow );
          //NewArrow->SetLabelScene(this);
          NewArrow->setZValue ( -1000.0 );
          NewArrow->UpdatePosition();
          arrow_count[*SuperClassNameStd]++;
        }
      }
    }
  }
  modified(true);
  return missingItems;
}

void dbse::SchemaGraphicsScene::RemoveItemFromScene ( QGraphicsItem* item ) {
  removeItem ( item );
  modified(true);
}

void dbse::SchemaGraphicsScene::add_notes (QStringList notes,
                                           QList<QPointF> positions ) {
  for ( int index = 0; index<notes.size(); index++) {
    auto note = new SchemaGraphicNote (
      QString("#" + QString::number(m_next_note++)),
      notes.at(index) );
    note->setPos ( positions.at ( index ) );
    m_notes.insert(note);
    addItem(note);
  }

}
void dbse::SchemaGraphicsScene::remove_note_object (SchemaGraphicNote* note ) {
  if (note == nullptr) {
    return;
  }
  RemoveItemFromScene (note);
}

void dbse::SchemaGraphicsScene::RemoveClassObject ( SchemaGraphicObject * Object )
{
  if ( Object == nullptr )
  {
    return;
  }

  Object->RemoveArrows();
  RemoveItemFromScene ( Object );
  ItemMap.remove ( Object->GetClassName() );
}

void dbse::SchemaGraphicsScene::CleanItemMap()
{
  ItemMap.clear();
}

void dbse::SchemaGraphicsScene::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
  if ( itemAt ( mouseEvent->scenePos(), QTransform() ) ) {
    // Save position of item under mouse so we can see if it has been
    // moved in mouseReleaseEvent
    m_mouse_item_pos = itemAt(mouseEvent->scenePos(), QTransform())->pos();
  }
  if ( mouseEvent->button() != Qt::LeftButton )
  {
    return;
  }

  if ( mouseEvent->widget()->cursor().shape() == Qt::CrossCursor )
  {
    m_line = new QGraphicsLineItem ( QLineF ( mouseEvent->scenePos(), mouseEvent->scenePos() ) );
    m_line->setPen ( QPen ( Qt::black, 2 ) );
    addItem ( m_line );
    modified(true);
    return;
  }

  QGraphicsScene::mousePressEvent ( mouseEvent );
}

void dbse::SchemaGraphicsScene::mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
  if ( m_line != nullptr )
  {
    QLineF newLine ( m_line->line().p1(), mouseEvent->scenePos() );
    m_line->setLine ( newLine );
  }
  else
  {
    QGraphicsScene::mouseMoveEvent ( mouseEvent );
  }
}

void dbse::SchemaGraphicsScene::ClearModified() {
  modified(false);
}

void dbse::SchemaGraphicsScene::modified(bool state) {
  m_modified = state;
  emit sceneModified(state);
}

void dbse::SchemaGraphicsScene::moveScene() {
  qreal minx = 1e6;
  qreal miny = 1e6;
  for (auto obj : ItemMap) {
    auto pos = obj->pos();
    if (pos.x() < minx) {
      minx = pos.x();
    }
    if (pos.y() < miny) {
      miny = pos.y();
    }
  }
  qreal xoffset = m_current_pos.x() - minx;
  qreal yoffset = m_current_pos.y() - miny;
  for (auto obj : ItemMap) {
    obj->setX(obj->x()+xoffset);
    obj->setY(obj->y()+yoffset);
    obj->update_arrows();
  }
  for (auto obj : m_notes) {
    obj->setX(obj->x()+xoffset);
    obj->setY(obj->y()+yoffset);
  }
  update();
  modified(true);
}

void dbse::SchemaGraphicsScene::requestSave() {
  emit saveRequested();
}
void dbse::SchemaGraphicsScene::modified_slot() {
  modified(true);
}
void dbse::SchemaGraphicsScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
  if ( itemAt ( mouseEvent->scenePos(), QTransform() ) ) {
    auto item = itemAt(mouseEvent->scenePos(), QTransform() );
    if (!m_mouse_item_pos.isNull()) {
      if (m_mouse_item_pos != item->pos()) {
        modified(true);
      }
      m_mouse_item_pos = QPointF();
    }
  }
  if ( m_line != nullptr )
  {
    QList<QGraphicsItem *> startItems = items ( m_line->line().p1() );

    if ( startItems.count() && startItems.first() == m_line )
    {
      startItems.removeFirst();
    }

    QList<QGraphicsItem *> endItems = items ( m_line->line().p2() );

    if ( endItems.count() && endItems.first() == m_line )
    {
      endItems.removeFirst();
    }

    RemoveItemFromScene ( m_line );
    delete m_line;

    if ( startItems.count() > 0 && endItems.count() > 0
         && startItems.first() != endItems.first() )
    {

      bool Inheritance = KernelWrapper::GetInstance().GetInheritanceMode();
      SchemaGraphicObject * startItem = qgraphicsitem_cast<SchemaGraphicObject *> (
                                          startItems.first() );
      SchemaGraphicObject * endItem = qgraphicsitem_cast<SchemaGraphicObject *> (
                                        endItems.first() );

      if ( Inheritance )
      {
        startItem->GetClass()->add_super_class ( endItem->GetClassName().toStdString() );
        /// Create arrow
        SchemaGraphicSegmentedArrow * newArrow = new SchemaGraphicSegmentedArrow (
          startItem, endItem,
          0,
          Inheritance,
          true, "", "" );
        startItem->AddArrow ( newArrow );
        endItem->AddArrow ( newArrow );
        newArrow->setZValue ( -1000.0 );
        addItem ( newArrow );
        //newArrow->SetLabelScene(this);
        newArrow->UpdatePosition();
      }
      else
      {
        SchemaRelationshipEditor * Editor = new SchemaRelationshipEditor (
          startItem->GetClass(), endItem->GetClassName() );
        connect ( Editor, SIGNAL ( MakeGraphConnection ( QString, QString, QString ) ), this,
                  SLOT ( DrawArrow ( QString, QString, QString ) ) );
        Editor->show();
      }
    }
  }

  m_line = nullptr;
  QGraphicsScene::mouseReleaseEvent ( mouseEvent );
}

void dbse::SchemaGraphicsScene::edit_note_slot() {
  m_current_note->open_editor();
}

void dbse::SchemaGraphicsScene::remove_note_slot() {
  remove_note_object(m_current_note);
}

void dbse::SchemaGraphicsScene::add_note_slot(SchemaGraphicNote* note) {
  m_notes.insert(note);
  addItem(note);
  modified(true);
}
void dbse::SchemaGraphicsScene::cancel_note_slot(SchemaGraphicNote* note) {
  m_notes.erase(note);
  delete note;
}

void dbse::SchemaGraphicsScene::new_note_slot() {
  auto note = new SchemaGraphicNote (
    QString("#") + QString::number(m_next_note++), QString());
  note->setPos(m_current_pos);
  auto editor = new SchemaNoteEditor(note);
  connect(editor, SIGNAL(note_accepted(SchemaGraphicNote*)), this, SLOT(add_note_slot(SchemaGraphicNote*)));
  connect(editor, SIGNAL(cancelled(SchemaGraphicNote*)), this, SLOT(cancel_note_slot(SchemaGraphicNote*)));
  editor->show();
}

void dbse::SchemaGraphicsScene::add_class_slot(QString class_name) {
  disconnect(m_addclass_connection);

  auto object = new SchemaGraphicObject(class_name, this);
  object->setPos(m_current_pos);
  addItem (object);
  /// Updating item list
  ItemMap.insert(class_name, object);
}
void dbse::SchemaGraphicsScene::new_class_slot() {
  m_addclass_connection = connect (
    &KernelWrapper::GetInstance(), SIGNAL ( ClassCreated(QString) ),
    this, SLOT ( add_class_slot(QString) ) );
  SchemaClassEditor::createNewClass();
}

void dbse::SchemaGraphicsScene::EditClassSlot()
{
  QString class_name = QString::fromStdString ( CurrentObject->GetClass()->get_name() );
  SchemaClassEditor::launch(class_name);
}

void dbse::SchemaGraphicsScene::ToggleHighlightClass() {
  CurrentObject->toggle_highlight_class();
  this->update();
}

void dbse::SchemaGraphicsScene::ToggleHighlightActive() {
  m_highlight_active = !m_highlight_active;
  this->update();
}

void dbse::SchemaGraphicsScene::ToggleHighlightAbstract() {
  m_highlight_abstract = !m_highlight_abstract;
  this->update();
}

void dbse::SchemaGraphicsScene::ToggleDefault() {
  m_show_defaults = !m_show_defaults;
  for ( SchemaGraphicObject * item : ItemMap.values() ) {
      item->update_arrows();
  }
  this->update();
}

void dbse::SchemaGraphicsScene::ToggleIndirectInfos() {
  m_inherited_properties_visible = !m_inherited_properties_visible;

  for ( SchemaGraphicObject * item : ItemMap.values() ) {
      item->update_arrows();
  }
  this->update();
}

void dbse::SchemaGraphicsScene::AddDirectSuperClassesSlot() {

  QString class_name = QString::fromStdString ( CurrentObject->GetClass()->get_name() );
  OksClass * class_info = KernelWrapper::GetInstance().FindClass ( class_name.toStdString() );
  
  QStringList super_class_list;
  QList<QPointF> positions;

  const std::list<std::string *>* direct_classes = class_info->direct_super_classes();
  if(direct_classes != nullptr) {
      for(std::string * cl_name : *direct_classes) {
          super_class_list.push_back(QString::fromStdString(*cl_name));
          positions.push_back({0,0});
      }
  }

  this->AddItemsToScene ( super_class_list, positions );

}

void dbse::SchemaGraphicsScene::AddAllSuperClassesSlot() {

  QString class_name = QString::fromStdString ( CurrentObject->GetClass()->get_name() );
  OksClass * class_info = KernelWrapper::GetInstance().FindClass ( class_name.toStdString() );
  
  QStringList super_class_list;
  QList<QPointF> positions;

  const OksClass::FList* all_classes = class_info->all_super_classes();
  if(all_classes != nullptr) {
      for(const OksClass* cl : *all_classes) {
          super_class_list.push_back(QString::fromStdString(cl->get_name()));
          positions.push_back({0,0});
      }
  }


  this->AddItemsToScene ( super_class_list, positions );

}

void dbse::SchemaGraphicsScene::AddAllSubClassesSlot() {

  QString class_name = QString::fromStdString ( CurrentObject->GetClass()->get_name() );
  OksClass * class_info = KernelWrapper::GetInstance().FindClass ( class_name.toStdString() );
  
  QStringList sub_class_list;
  QList<QPointF> positions;

  const OksClass::FList* all_classes = class_info->all_sub_classes();
  if(all_classes != nullptr) {
      for(const OksClass* cl : *all_classes) {
          sub_class_list.push_back(QString::fromStdString(cl->get_name()));
          positions.push_back({0,0});
      }
  }

  this->AddItemsToScene ( sub_class_list, positions );

}

void dbse::SchemaGraphicsScene::AddDirectRelationshipClassesSlot() {

  QString class_name = QString::fromStdString ( CurrentObject->GetClass()->get_name() );
  OksClass * class_info = KernelWrapper::GetInstance().FindClass ( class_name.toStdString() );
  
  QStringList relationship_classes;
  QList<QPointF> positions;

  const std::list<OksRelationship *> * direct_relationship_list = class_info->direct_relationships();
  if ( direct_relationship_list != nullptr ) {
      for(const OksRelationship* rl : *direct_relationship_list) {
          relationship_classes.push_back(QString::fromStdString(rl->get_type()));
          positions.push_back({0,0});
      }

  }

  this->AddItemsToScene ( relationship_classes, positions );

}

void dbse::SchemaGraphicsScene::AddAllRelationshipClassesSlot() {

  QString class_name = QString::fromStdString ( CurrentObject->GetClass()->get_name() );
  OksClass * class_info = KernelWrapper::GetInstance().FindClass ( class_name.toStdString() );
  
  QStringList relationship_classes;
  QList<QPointF> positions;

  const std::list<OksRelationship *> * all_relationship_list = class_info->all_relationships();
  if ( all_relationship_list != nullptr ) {
      for(const OksRelationship* rl : *all_relationship_list) {
          relationship_classes.push_back(QString::fromStdString(rl->get_type()));
          positions.push_back({0,0});
      }

  }

  this->AddItemsToScene ( relationship_classes, positions );
}

void dbse::SchemaGraphicsScene::RemoveClassSlot()
{
  if ( CurrentObject == nullptr )
  {
    return;
  }

  CurrentObject->RemoveArrows();
  RemoveItemFromScene ( CurrentObject );
  ItemMap.remove ( CurrentObject->GetClassName() );
}

void dbse::SchemaGraphicsScene::RemoveArrowSlot()
{
  RemoveItemFromScene ( m_current_arrow );
  m_current_arrow->GetStartItem()->RemoveArrow ( m_current_arrow );
  m_current_arrow->GetEndItem()->RemoveArrow ( m_current_arrow );
  m_current_arrow->RemoveArrow();
}

void dbse::SchemaGraphicsScene::DrawArrow ( QString ClassName, QString RelationshipType,
                                            QString RelationshipName )
{
  if ( !ItemMap.contains ( ClassName ) || !ItemMap.contains ( RelationshipType ) )
  {
    return;
  }

  SchemaGraphicObject * startItem = ItemMap[ClassName];
  SchemaGraphicObject * endItem = ItemMap[RelationshipType];

  OksClass * SchemaClass = KernelWrapper::GetInstance().FindClass ( ClassName.toStdString() );
  OksRelationship * SchemaRelationship = SchemaClass->find_direct_relationship (
                                           RelationshipName.toStdString() );

  if ( SchemaRelationship != nullptr )
  {
    QString RelationshipCardinality =
      KernelWrapper::GetInstance().GetCardinalityStringRelationship ( SchemaRelationship );
    SchemaGraphicSegmentedArrow * newArrow = new SchemaGraphicSegmentedArrow (
      startItem, endItem,
      0,
      false, SchemaRelationship->get_is_composite(),
      QString::fromStdString ( SchemaRelationship->get_name() ), RelationshipCardinality );
    startItem->AddArrow ( newArrow );
    endItem->AddArrow ( newArrow );
    newArrow->setZValue ( -1000.0 );
    addItem ( newArrow );
    //newArrow->SetLabelScene(this);
    newArrow->UpdatePosition();
  }
}
