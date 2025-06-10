#ifndef SCHEMACLASSEDITOR_H
#define SCHEMACLASSEDITOR_H

#include <memory>
/// Including QT Headers
#include <QWidget>
#include <QMenu>
/// Including Oks Headers
#include "oks/class.hpp"
/// Including Schema
#include "dbe/SchemaCustomMethodModel.hpp"
#include "dbe/SchemaCustomAttributeModel.hpp"
#include "dbe/SchemaCustomRelationshipModel.hpp"
#include "dbe/SchemaCustomSuperClassModel.hpp"
#include "dbe/SchemaCustomSubClassModel.hpp"

namespace dbse
{

namespace Ui
{
class SchemaClassEditor;
} // namespace Ui

class SchemaClassEditor: public QWidget
{
  Q_OBJECT
public:
  ~SchemaClassEditor();

  explicit SchemaClassEditor ( dunedaq::oks::OksClass * ClassInfo, QWidget * parent = nullptr );

  static void createNewClass ();
private:
  void keyPressEvent(QKeyEvent* event);
  void SetController();
  void BuildModels();
  void InitialSettings();
  /// Helper functions
  bool ShouldOpenAttributeEditor ( QString Name );
  bool ShouldOpenRelationshipEditor ( QString Name );
  bool ShouldOpenMethodEditor ( QString Name );

  std::unique_ptr<dbse::Ui::SchemaClassEditor> ui;

  dunedaq::oks::OksClass * SchemaClass;
  CustomMethodModel * MethodModel;
  CustomAttributeModel * AttributeModel;
  CustomRelationshipModel * RelationshipModel;
  CustomSuperClassModel * SuperClassModel;
  CustomSubClassModel * SubClassModel;
  QMenu * ContextMenuAttribute;
  QMenu * ContextMenuRelationship;
  QMenu * ContextMenuMethod;
  QMenu * ContextMenuClass;
  QStringList CurrentRow;

  void OpenNewClassEditor( const QString& ClassName);

private slots:
  void close_slot(){close();}
  void OpenSuperClass( QModelIndex Index);
  void OpenSubClass( QModelIndex Index);

  void ParseToSave();
  void AddNewSuperClass();
  void AddNewAttribute();
  void AddNewRelationship();
  void AddNewMethod();
  void OpenAttributeEditor ( QModelIndex Index );
  void OpenRelationshipEditor ( QModelIndex Index );
  void OpenMethodEditor ( QModelIndex Index );
  void BuildAttributeModelSlot();
  void BuildRelationshipModelSlot();
  void BuildSuperClassModelSlot();
  void BuildSubClassModelSlot();
  void BuildMethodModelSlot();
  /// Context Menu Functions
  void CustomMenuAttributeView ( QPoint pos );
  void CustomMenuRelationshipView ( QPoint pos );
  void CustomMenuMethodView ( QPoint pos );
  void CustomMenuClassView ( QPoint pos );
  /// Remove Functions
  void RemoveAttribute();
  void RemoveRelationship();
  void RemoveMethod();
  void RemoveSuperClass();
  void ClassRemoved( QString className );
  void ClassUpdated( QString className );
};
}  // namespace dbse

#endif // SCHEMACLASSEDITOR_H
