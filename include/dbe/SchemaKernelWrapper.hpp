#ifndef KERNELWRAPPER_H
#define KERNELWRAPPER_H

/// Including Qt
#include <QStringList>
#include <QObject>
/// Including c++
#include <vector>
#include <set>
/// Including Schema
#include "dbe/SchemaCommand.hpp"

namespace dunedaq {
  namespace oks {
    class OksKernel;
    class OksClass;
  }
}

namespace dbse
{

class KernelWrapper: public QObject
{
  friend class SchemaMainWindow;
  friend class SchemaClassEditor;
  friend class SchemaAttributeEditor;
  friend class SchemaMethodEditor;
  friend class SchemaRelationshipEditor;

  Q_OBJECT
public:
  static KernelWrapper & GetInstance();
  dunedaq::oks::OksKernel * GetKernel();
  void SetActiveSchema ( const std::string & ActiveSchema );
  void GetClassList ( std::vector<dunedaq::oks::OksClass *> & ClassList ) const;
  void GetClassListString ( QStringList & ClassListString ) const;
  void GetSchemaFiles ( std::vector<std::string> & SchemaFiles );
  void GetIncludedList ( const std::string & FileName,
                         std::set<std::string> & IncludedFiles );
  bool IsFileWritable ( const std::string & FileName ) const;
  bool IsActive() const;
  dunedaq::oks::OksClass * FindClass ( std::string ClassName ) const;
  void LoadSchema ( const std::string & SchemaName ) const;
  void SaveAllSchema() const;
  void SaveSchema ( const std::string& file ) const;
  std::string ModifiedSchemaFiles() const;
  std::string SaveModifiedSchema() const;
  std::string GetActiveSchema() const;
  void AddInclude( std::string schemaFile, std::string IncludeFile ) const;
  void RemoveInclude( std::string schemaFile, std::string IncludeFile ) const;
  void CloseAllSchema() const;
  void CreateNewSchema ( const std::string & SchemaName ) const;
  bool AnyClassReferenceThis ( dunedaq::oks::OksClass * SchemaClass );
  QString GetCardinalityStringRelationship ( dunedaq::oks::OksRelationship * SchemaRelationship ) const;
  /// Editor Functions
  void SetInheritanceMode ( bool Mode );
  bool GetInheritanceMode() const;
  /// Stack Functions
  QUndoStack * GetUndoStack();
  /// Commands Functions
  /// Class commands
  void PushSetAbstractClassCommand ( dunedaq::oks::OksClass * Class, bool Value );
  void PushSetDescriptionClassCommand ( dunedaq::oks::OksClass * Class, std::string Description );
  void PushAddSuperClassCommand ( dunedaq::oks::OksClass * Class, std::string SuperClass );
  void PushRemoveSuperClassCommand ( dunedaq::oks::OksClass * Class, std::string SuperClass );
  void PushCreateClassCommand ( std::string ClassName, std::string ClassDescription,
                                bool Abstract );
  void PushRemoveClassCommand ( dunedaq::oks::OksClass * Class, std::string ClassName,
                                std::string ClassDescription, bool Abstract );
  /// Relationship commands
  void PushSetNameRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, std::string Name );
  void PushSetClassTypeRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship,
                                             std::string ClassType );
  void PushSetDescriptionRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship,
                                               std::string Description );
  void PushSetLowCcRelationshipCommand (
    dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, dunedaq::oks::OksRelationship::CardinalityConstraint NewCardinality );
  void PushSetHighCcRelationshipCommand (
    dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, dunedaq::oks::OksRelationship::CardinalityConstraint NewCardinality );
  void PushSetIsCompositeRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, bool Value );
  void PushSetIsDependentRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, bool Value );
  void PushSetIsExclusiveRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, bool Value );
  void PushAddRelationship ( dunedaq::oks::OksClass * Class, std::string Name, std::string Description,
                             std::string Type, bool Composite, bool Exclusive, bool Dependent,
                             dunedaq::oks::OksRelationship::CardinalityConstraint LowCc,
                             dunedaq::oks::OksRelationship::CardinalityConstraint HighCc );
  void PushRemoveRelationship ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksRelationship * Relationship,
                                std::string Name, std::string Description, std::string Type,
                                bool Composite, bool Exclusive, bool Dependent,
                                dunedaq::oks::OksRelationship::CardinalityConstraint LowCc,
                                dunedaq::oks::OksRelationship::CardinalityConstraint HighCc );
  /// Method implementation commands
  void PushSetMethodImplementationLanguage ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, dunedaq::oks::OksMethodImplementation * Implementation,
                                             std::string Language );
  void PushSetMethodImplementationPrototype ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, dunedaq::oks::OksMethodImplementation * Implementation,
                                              std::string Prototype );
  void PushSetMethodImplementationBody ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, dunedaq::oks::OksMethodImplementation * Implementation,
                                         std::string Body );
  void PushAddMethodImplementationComand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string Language,
                                           std::string Prototype, std::string Body );
  void PushRemoveMethodImplementationComand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string Language,
                                              std::string Prototype, std::string Body );
  /// Method commands
  void PushSetNameMethodCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string name );
  void PushSetDescriptionMethodCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string description );
  void PushAddMethodCommand ( dunedaq::oks::OksClass * Class, std::string name, std::string description );
  void PushRemoveMethodCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string name,
                                 std::string description );
  /// Attribute commands
  void PushSetAttributeNameCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string NewName );
  void PushSetAttributeDescriptionCommand ( dunedaq::oks::OksClass * Class,
                                            dunedaq::oks::OksAttribute * Attribute,
                                            std::string NewDescription );
  void PushSetAttributeTypeCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string NewType );
  void PushSetAttributeRangeCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string NewRange );
  void PushSetAttributeFormatCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute,
                                       dunedaq::oks::OksAttribute::Format NewFormat );
  void PushSetAttributeMultiCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, bool NewIsMulti );
  void PushSetAttributeIsNullCommand ( dunedaq::oks::OksClass * Class,dunedaq::oks::OksAttribute * Attribute, bool NewIsNull );
  void PushSetAttributeInitialValuesCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute,
                                              std::string NewValues );
  void PushAddAttributeCommand ( dunedaq::oks::OksClass * Class, std::string name, std::string type,
                                 bool is_mv, std::string range, std::string init_values,
                                 std::string description, bool is_null,
                                 dunedaq::oks::OksAttribute::Format format = dunedaq::oks::OksAttribute::Format::Dec );
  void PushRemoveAttributeCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute,
                                    std::string name,
                                    std::string type, bool is_mv, std::string range,
                                    std::string init_values, std::string description,
                                    bool is_null, dunedaq::oks::OksAttribute::Format format =
                                      dunedaq::oks::OksAttribute::Format::Dec );
private:
  KernelWrapper ( QObject * parent = nullptr );
  dunedaq::oks::OksKernel * Kernel;
  QUndoStack * CommandStack;
  bool InheritanceMode;
signals:
  void ClassCreated ( QString ClassName );
  void ClassRemoved ( QString ClassName );
  void ClassUpdated ( QString ClassName );
  void RebuildAttributeModel();
};

}  // namespace dbse
#endif // KERNELWRAPPER_H
