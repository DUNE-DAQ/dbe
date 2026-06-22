// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/SchemaCommand.h to include/dbe/SchemaCommand.hpp).

#ifndef SCHEMACOMMAND_H
#define SCHEMACOMMAND_H

/// Including Qt Headers
#include <QUndoCommand>
#include "oks/class.hpp"
#include "oks/relationship.hpp"
#include "oks/method.hpp"
#include "oks/attribute.hpp"

namespace dunedaq {
  namespace oks {
    class OksClass;
  }
}

namespace dbse
{

/// Class Commands
class SetAbstractClassCommand: public QUndoCommand
{
public:
  SetAbstractClassCommand ( dunedaq::oks::OksClass * Class, bool Value );
  ~SetAbstractClassCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  bool NewValue;
  bool OldValue;
};

class SetDescriptionClassCommand: public QUndoCommand
{
public:
  SetDescriptionClassCommand ( dunedaq::oks::OksClass * Class, std::string Description );
  ~SetDescriptionClassCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  std::string NewDescription;
  std::string OldDescription;
};

class AddSuperClassCommand: public QUndoCommand
{
public:
  AddSuperClassCommand ( dunedaq::oks::OksClass * Class, std::string SuperClass );
  ~AddSuperClassCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  std::string NewSuperClass;
};

class RemoveSuperClassCommand: public QUndoCommand
{
public:
  RemoveSuperClassCommand ( dunedaq::oks::OksClass * Class, std::string SuperClass );
  ~RemoveSuperClassCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  std::string SuperClass;
};

class CreateClassCommand: public QUndoCommand
{
public:
  CreateClassCommand ( std::string ClassName, std::string ClassDescription, bool Abstract );
  ~CreateClassCommand();
  void redo();
  void undo();
private:
  dunedaq::oks::OksClass * SchemaClass;
  std::string SchemaClassName;
  std::string SchemaClassDescription;
  bool SchemaAbstract;
};

class RemoveClassCommand: public QUndoCommand
{
public:
  RemoveClassCommand ( dunedaq::oks::OksClass * Class, std::string ClassName, std::string ClassDescription,
                       bool Abstract );
  ~RemoveClassCommand();
  void redo();
  void undo();
private:
  dunedaq::oks::OksClass * SchemaClass;
  std::string SchemaClassName;
  std::string SchemaClassDescription;
  bool SchemaAbstract;
};

/// Relationships Commands
class SetNameRelationshipCommand: public QUndoCommand
{
public:
  SetNameRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, std::string Name );
  ~SetNameRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string NewRelationshipName;
  std::string OldRelationshipName;
};

class SetClassTypeRelationshipCommand: public QUndoCommand
{
public:
  SetClassTypeRelationshipCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksRelationship * Relationship, std::string ClassType );
  ~SetClassTypeRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  std::string NewRelationshipType;
  std::string OldRelationshipType;
};

class SetDescriptionRelationshipCommand: public QUndoCommand
{
public:
  SetDescriptionRelationshipCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksRelationship * Relationship,
                                      std::string Description );
  ~SetDescriptionRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  std::string NewDescription;
  std::string OldDescription;
};

class SetLowCcRelationshipCommand: public QUndoCommand
{
public:
  SetLowCcRelationshipCommand ( dunedaq::oks::OksClass* Class,
                                dunedaq::oks::OksRelationship * Relationship,
                                dunedaq::oks::OksRelationship::CardinalityConstraint NewCardinality );
  ~SetLowCcRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  dunedaq::oks::OksRelationship::CardinalityConstraint NewLowCc;
  dunedaq::oks::OksRelationship::CardinalityConstraint OldLowCc;
};

class SetHighCcRelationshipCommand: public QUndoCommand
{
public:
  SetHighCcRelationshipCommand ( dunedaq::oks::OksClass * Class,
                                 dunedaq::oks::OksRelationship * Relationship,
                                 dunedaq::oks::OksRelationship::CardinalityConstraint NewCardinality );
  ~SetHighCcRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  dunedaq::oks::OksRelationship::CardinalityConstraint NewHighCc;
  dunedaq::oks::OksRelationship::CardinalityConstraint OldHighCc;
};

class SetIsCompositeRelationshipCommand: public QUndoCommand
{
public:
  SetIsCompositeRelationshipCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksRelationship * Relationship, bool Value );
  ~SetIsCompositeRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  bool NewValue;
  bool OldValue;
};

class SetIsDependentRelationshipCommand: public QUndoCommand
{
public:
  SetIsDependentRelationshipCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksRelationship * Relationship, bool Value );
  ~SetIsDependentRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  bool NewValue;
  bool OldValue;
};

class SetIsExclusiveRelationshipCommand: public QUndoCommand
{
public:
  SetIsExclusiveRelationshipCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksRelationship * Relationship, bool Value );
  ~SetIsExclusiveRelationshipCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  bool NewValue;
  bool OldValue;
};

class AddRelationship: public QUndoCommand
{
public:
  AddRelationship ( dunedaq::oks::OksClass * Class, std::string Name, std::string Description,
                    std::string Type, bool Composite, bool Exclusive, bool Dependent,
                    dunedaq::oks::OksRelationship::CardinalityConstraint LowCc,
                    dunedaq::oks::OksRelationship::CardinalityConstraint HighCc );
  ~AddRelationship();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  std::string RelationshipDescription;
  std::string RelationshipType;
  bool IsComposite;
  bool IsExclusive;
  bool IsDependent;
  dunedaq::oks::OksRelationship::CardinalityConstraint RelationshipLowCc;
  dunedaq::oks::OksRelationship::CardinalityConstraint RelationshipHighCc;
};

class RemoveRelationship: public QUndoCommand
{
public:
  RemoveRelationship ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksRelationship * Relationship, std::string Name,
                       std::string Description, std::string Type, bool Composite,
                       bool Exclusive, bool Dependent,
                       dunedaq::oks::OksRelationship::CardinalityConstraint LowCc,
                       dunedaq::oks::OksRelationship::CardinalityConstraint HighCc );
  ~RemoveRelationship();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksRelationship * SchemaRelationship;
  std::string RelationshipName;
  std::string RelationshipDescription;
  std::string RelationshipType;
  bool IsComposite;
  bool IsExclusive;
  bool IsDependent;
  dunedaq::oks::OksRelationship::CardinalityConstraint RelationshipLowCc;
  dunedaq::oks::OksRelationship::CardinalityConstraint RelationshipHighCc;
};

/// Methods implementation commands
class SetMethodImplementationLanguage: public QUndoCommand
{
public:
  SetMethodImplementationLanguage ( dunedaq::oks::OksClass * Class,
                                    dunedaq::oks::OksMethod * Method,
                                    dunedaq::oks::OksMethodImplementation * Implementation,
                                    std::string Language );
  ~SetMethodImplementationLanguage();
  void redo();
  void undo();
private:
  std::string ClassName;
  std::string MethodName;
  dunedaq::oks::OksMethodImplementation * SchemaImplementation;
  std::string NewLanguage;
  std::string OldLanguage;
};

class SetMethodImplementationPrototype: public QUndoCommand
{
public:
  SetMethodImplementationPrototype ( dunedaq::oks::OksClass * Class,
                                     dunedaq::oks::OksMethod * Method,
                                     dunedaq::oks::OksMethodImplementation * Implementation,
                                     std::string Prototype );
  ~SetMethodImplementationPrototype();
  void redo();
  void undo();
private:
  std::string ClassName;
  std::string MethodName;
  dunedaq::oks::OksMethodImplementation * SchemaImplementation;
  std::string ImplementationLanguage;
  std::string NewPrototype;
  std::string OldPrototype;
};

class SetMethodImplementationBody: public QUndoCommand
{
public:
  SetMethodImplementationBody ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, dunedaq::oks::OksMethodImplementation * Implementation, std::string Body );
  ~SetMethodImplementationBody();
  void redo();
  void undo();
private:
  std::string ClassName;
  std::string MethodName;
  dunedaq::oks::OksMethodImplementation * SchemaImplementation;
  std::string ImplementationLanguage;
  std::string NewBody;
  std::string OldBody;
};

class AddMethodImplementationComand: public QUndoCommand
{
public:
  AddMethodImplementationComand ( dunedaq::oks::OksClass * Class,
                                  dunedaq::oks::OksMethod * Method, std::string Language,
                                  std::string Prototype, std::string Body );
  ~AddMethodImplementationComand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksMethod * SchemaMethod;
  std::string MethodName;
  std::string SchemaImplementationLanguage;
  std::string SchemaImplementationPrototype;
  std::string SchemaImplementationBody;
};

class RemoveMethodImplementationComand: public QUndoCommand
{
public:
  RemoveMethodImplementationComand ( dunedaq::oks::OksClass * Class,
                                     dunedaq::oks::OksMethod * Method, std::string Language,
                                     std::string Prototype, std::string Body );
  ~RemoveMethodImplementationComand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksMethod * SchemaMethod;
  std::string MethodName;
  std::string SchemaImplementationLanguage;
  std::string SchemaImplementationPrototype;
  std::string SchemaImplementationBody;
};

/// Method commands
class SetNameMethodCommand: public QUndoCommand
{
public:
  SetNameMethodCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string name );
  ~SetNameMethodCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksMethod * SchemaMethod;
  std::string NewMethodName;
  std::string OldMethodName;
};

class SetDescriptionMethodCommand: public QUndoCommand
{
public:
  SetDescriptionMethodCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string description );
  ~SetDescriptionMethodCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksMethod * SchemaMethod;
  std::string MethodName;
  std::string NewMethodDescription;
  std::string OldMethodDescription;
};

class AddMethodCommand: public QUndoCommand
{
public:
  AddMethodCommand ( dunedaq::oks::OksClass * Class, std::string name, std::string description );
  ~AddMethodCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksMethod * SchemaMethod;
  std::string SchemaName;
  std::string SchemaDescription;
};

class RemoveMethodCommand: public QUndoCommand
{
public:
  RemoveMethodCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksMethod * Method, std::string name,
                        std::string description );
  ~RemoveMethodCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksMethod * SchemaMethod;
  std::string SchemaName;
  std::string SchemaDescription;
};

/// Attributes commands
class SetAttributeNameCommand: public QUndoCommand
{
public:
  SetAttributeNameCommand ( dunedaq::oks::OksClass* Class, dunedaq::oks::OksAttribute * Attribute, std::string NewName );
  ~SetAttributeNameCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string NewAttributeName;
  std::string OldAttributeName;
};

class SetAttributeDescriptionCommand: public QUndoCommand
{
public:
  SetAttributeDescriptionCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string NewDescription );
  ~SetAttributeDescriptionCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string AttributeName;
  std::string NewAttributeDescription;
  std::string OldAttributeDescription;
};

class SetAttributeTypeCommand: public QUndoCommand
{
public:
  SetAttributeTypeCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string NewType );
  ~SetAttributeTypeCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string AttributeName;
  std::string NewAttributeType;
  std::string OldAttributeType;
};

class SetAttributeRangeCommand: public QUndoCommand
{
public:
  SetAttributeRangeCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string NewRange );
  ~SetAttributeRangeCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string AttributeName;
  std::string NewAttributeRange;
  std::string OldAttributeRange;
};

class SetAttributeFormatCommand: public QUndoCommand
{
public:
  SetAttributeFormatCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, dunedaq::oks::OksAttribute::Format NewFormat );
  ~SetAttributeFormatCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string AttributeName;
  dunedaq::oks::OksAttribute::Format NewAttributeFormat;
  dunedaq::oks::OksAttribute::Format OldAttributeFormat;
};

class SetAttributeMultiCommand: public QUndoCommand
{
public:
  SetAttributeMultiCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, bool NewIsMulti );
  ~SetAttributeMultiCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string AttributeName;
  bool NewAttributeMulti;
  bool OldAttributeMulti;
};

class SetAttributeIsNullCommand: public QUndoCommand
{
public:
  SetAttributeIsNullCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, bool NewIsNull );
  ~SetAttributeIsNullCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string AttributeName;
  bool NewAttributeIsNull;
  bool OldAttributeIsNull;
};

class SetAttributeInitialValuesCommand: public QUndoCommand
{
public:
  SetAttributeInitialValuesCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string NewValues );
  ~SetAttributeInitialValuesCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string AttributeName;
  std::string NewAttributeInitialValues;
  std::string OldAttributeInitialValues;
};

class AddAttributeCommand: public QUndoCommand
{
public:
  AddAttributeCommand ( dunedaq::oks::OksClass * Class, std::string name, std::string type, bool is_mv,
                        std::string range, std::string init_values, std::string description,
                        bool is_null, dunedaq::oks::OksAttribute::Format format = dunedaq::oks::OksAttribute::Format::Dec );
  ~AddAttributeCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string SchemaName;
  std::string SchemaType;
  bool SchemaIsMulti;
  std::string SchemaRange;
  std::string SchemaInitValues;
  std::string SchemaDescription;
  bool SchemaIsNull;
  dunedaq::oks::OksAttribute::Format SchemaFormat;
};

class RemoveAttributeCommand: public QUndoCommand
{
public:
  RemoveAttributeCommand ( dunedaq::oks::OksClass * Class, dunedaq::oks::OksAttribute * Attribute, std::string name,
                           std::string type, bool is_mv, std::string range,
                           std::string init_values, std::string description, bool is_null,
                           dunedaq::oks::OksAttribute::Format format = dunedaq::oks::OksAttribute::Format::Dec );
  ~RemoveAttributeCommand();
  void redo();
  void undo();
private:
  std::string ClassName;
  dunedaq::oks::OksAttribute * SchemaAttribute;
  std::string SchemaName;
  std::string SchemaType;
  bool SchemaIsMulti;
  std::string SchemaRange;
  std::string SchemaInitValues;
  std::string SchemaDescription;
  bool SchemaIsNull;
  dunedaq::oks::OksAttribute::Format SchemaFormat;
};

}  // namespace dbse

#endif // SCHEMACOMMAND_H
