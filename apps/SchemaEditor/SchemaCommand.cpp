/// Including Qt
#include <QMessageBox>
/// Including Schema Editor
#include "dbe/SchemaCommand.hpp"
#include "dbe/SchemaKernelWrapper.hpp"

using namespace dunedaq::oks;


namespace dbse
{

SetAbstractClassCommand::SetAbstractClassCommand ( OksClass * Class, bool Value )
  : ClassName ( Class->get_name() ),
    NewValue ( Value ),
    OldValue ( Class->get_is_abstract() )
{
  setText ( QString::fromStdString("Changed abstract flag for class \"" + ClassName + "\"" ) );
}

SetAbstractClassCommand::~SetAbstractClassCommand()
{
}

void SetAbstractClassCommand::redo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->set_is_abstract ( NewValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAbstractClassCommand::undo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->set_is_abstract ( OldValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetDescriptionClassCommand::SetDescriptionClassCommand ( OksClass * Class,
                                                         std::string Description )
  : ClassName ( Class->get_name() ),
    NewDescription ( Description ),
    OldDescription ( Class->get_description() )
{
  setText ( QString::fromStdString("Changed description for class \"" + ClassName + "\"") );
}

SetDescriptionClassCommand::~SetDescriptionClassCommand()
{
}

void SetDescriptionClassCommand::redo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->set_description ( NewDescription );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetDescriptionClassCommand::undo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->set_description ( OldDescription );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

CreateClassCommand::CreateClassCommand ( std::string ClassName,
                                         std::string ClassDescription,
                                         bool Abstract )
  : SchemaClass ( nullptr ),
    SchemaClassName ( ClassName ),
    SchemaClassDescription ( ClassDescription ),
    SchemaAbstract ( Abstract )
{
  setText ( QString::fromStdString("Created new class \"" + ClassName + "\"") );
}

CreateClassCommand::~CreateClassCommand()
{
}

void CreateClassCommand::redo()
{
  SchemaClass = new OksClass ( SchemaClassName, SchemaClassDescription, SchemaAbstract,
                               KernelWrapper::GetInstance().GetKernel() );
  emit KernelWrapper::GetInstance().ClassCreated(QString::fromStdString(SchemaClassName));
}

void CreateClassCommand::undo()
{
  try
  {
    OksClass::destroy ( SchemaClass );
    emit KernelWrapper::GetInstance().ClassRemoved(QString::fromStdString(SchemaClassName));
  }
  catch ( ... )
  {
    QMessageBox::warning ( 0, "Schema editor", QString ( "Error" ) );
  }
}

RemoveClassCommand::RemoveClassCommand ( OksClass * Class, std::string ClassName,
                                         std::string ClassDescription, bool Abstract )
  : SchemaClass ( Class ),
    SchemaClassName ( ClassName ),
    SchemaClassDescription ( ClassDescription ),
    SchemaAbstract ( Abstract )
{
  setText ( QString::fromStdString("Removed class \"" + ClassName + "\"" ));
}

RemoveClassCommand::~RemoveClassCommand()
{
}

void RemoveClassCommand::redo()
{
  OksClass::destroy ( SchemaClass );
  emit KernelWrapper::GetInstance().ClassRemoved(QString::fromStdString(SchemaClassName));
}

void RemoveClassCommand::undo()
{
  SchemaClass = new OksClass ( SchemaClassName, SchemaClassDescription, SchemaAbstract,
                               KernelWrapper::GetInstance().GetKernel() );
  emit KernelWrapper::GetInstance().ClassCreated(QString::fromStdString(SchemaClassName));
}

AddSuperClassCommand::AddSuperClassCommand ( OksClass * Class, std::string SuperClass )
  : ClassName ( Class->get_name() ),
    NewSuperClass ( SuperClass )
{
  setText ( QString::fromStdString("Added new super-class \"" + SuperClass + "\" to class \"" + ClassName + "\"") );
}

AddSuperClassCommand::~AddSuperClassCommand()
{
}

void AddSuperClassCommand::redo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->add_super_class ( NewSuperClass );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void AddSuperClassCommand::undo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->remove_super_class ( NewSuperClass );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

RemoveSuperClassCommand::RemoveSuperClassCommand ( OksClass * Class, std::string superClass )
  : ClassName ( Class->get_name() ),
    SuperClass ( superClass )
{
  setText ( QString::fromStdString("Removed super-class \"" + superClass + "\" from class \"" + ClassName + "\"" ) );
}

RemoveSuperClassCommand::~RemoveSuperClassCommand()
{
}

void RemoveSuperClassCommand::redo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->remove_super_class ( SuperClass );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void RemoveSuperClassCommand::undo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->add_super_class ( SuperClass );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetNameRelationshipCommand::SetNameRelationshipCommand ( OksClass* Class,
                                                         OksRelationship * Relationship,
                                                         std::string Name )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    NewRelationshipName ( Name ),
    OldRelationshipName ( SchemaRelationship->get_name() )
{
  setText ( QString::fromStdString("Changed relationship \"" + Relationship->get_name() + "\" to \"" + Name + "\" for class \"" + ClassName + "\"") );
}

SetNameRelationshipCommand::~SetNameRelationshipCommand()
{
}

void SetNameRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(OldRelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_name ( NewRelationshipName );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetNameRelationshipCommand::undo()
{
  SchemaRelationship->set_name ( OldRelationshipName );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetClassTypeRelationshipCommand::SetClassTypeRelationshipCommand (
  OksClass* Class, OksRelationship * Relationship, std::string ClassType )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    RelationshipName ( Relationship->get_name()),
    NewRelationshipType ( ClassType ),
    OldRelationshipType ( SchemaRelationship->get_type() )
{
    setText ( QString::fromStdString("Relationship \"" + SchemaRelationship->get_name() +
                                     "\" set to type \"" + NewRelationshipType + "\" for class \"" + ClassName + "\""));
}

SetClassTypeRelationshipCommand::~SetClassTypeRelationshipCommand()
{
}

void SetClassTypeRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(RelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_type ( NewRelationshipType );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetClassTypeRelationshipCommand::undo()
{
  SchemaRelationship->set_type ( OldRelationshipType );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetDescriptionRelationshipCommand::SetDescriptionRelationshipCommand (
  OksClass* Class, OksRelationship * Relationship, std::string Description )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    RelationshipName (Relationship->get_name()),
    NewDescription ( Description ),
    OldDescription ( SchemaRelationship->get_description() )
{
    setText (QString::fromStdString("Changed description of relationship \"" + Relationship->get_name() + "\" for class \"" + ClassName + "\""));
}

SetDescriptionRelationshipCommand::~SetDescriptionRelationshipCommand()
{
}

void SetDescriptionRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(RelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_description ( NewDescription );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetDescriptionRelationshipCommand::undo()
{
  SchemaRelationship->set_description ( OldDescription );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetLowCcRelationshipCommand::SetLowCcRelationshipCommand (
  OksClass * Class, OksRelationship * Relationship, OksRelationship::CardinalityConstraint NewCardinality )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    RelationshipName ( Relationship->get_name()),
    NewLowCc ( NewCardinality ),
    OldLowCc ( SchemaRelationship->get_low_cardinality_constraint() )
{
    setText( QString::fromStdString("Changed low cardinality constraint for relationship \"" + Relationship->get_name() + "\" of class \"" + ClassName + "\""));
}

SetLowCcRelationshipCommand::~SetLowCcRelationshipCommand()
{
}

void SetLowCcRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(RelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_low_cardinality_constraint ( NewLowCc );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetLowCcRelationshipCommand::undo()
{
  SchemaRelationship->set_low_cardinality_constraint ( OldLowCc );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetHighCcRelationshipCommand::SetHighCcRelationshipCommand (
  OksClass * Class, OksRelationship * Relationship, OksRelationship::CardinalityConstraint NewCardinality )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    RelationshipName ( Relationship->get_name() ),
    NewHighCc ( NewCardinality ),
    OldHighCc ( SchemaRelationship->get_high_cardinality_constraint() )
{
    setText( QString::fromStdString("Changed high cardinality constraint for relationship \"" + Relationship->get_name() + "\" of class \"" + ClassName + "\""));
}

SetHighCcRelationshipCommand::~SetHighCcRelationshipCommand()
{

}

void SetHighCcRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(RelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_high_cardinality_constraint ( NewHighCc );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetHighCcRelationshipCommand::undo()
{
  SchemaRelationship->set_high_cardinality_constraint ( OldHighCc );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetIsCompositeRelationshipCommand::SetIsCompositeRelationshipCommand (
  OksClass * Class, OksRelationship * Relationship, bool Value )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    RelationshipName ( Relationship->get_name() ),
    NewValue ( Value ),
    OldValue ( SchemaRelationship->get_is_composite() )
{
    setText( QString::fromStdString("Changed the \"is composite\" constraint for relationship \"" + Relationship->get_name() + "\" of class \"" + ClassName + "\""));
}

SetIsCompositeRelationshipCommand::~SetIsCompositeRelationshipCommand()
{
}

void SetIsCompositeRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(RelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_is_composite ( NewValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetIsCompositeRelationshipCommand::undo()
{
  SchemaRelationship->set_is_composite ( OldValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetIsDependentRelationshipCommand::SetIsDependentRelationshipCommand (
  OksClass * Class, OksRelationship * Relationship, bool Value )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    RelationshipName ( Relationship->get_name() ),
    NewValue ( Value ),
    OldValue ( SchemaRelationship->get_is_dependent() )
{
    setText( QString::fromStdString("Changed the \"is dependent\" constraint for relationship \"" + Relationship->get_name() + "\" of class \"" + ClassName + "\""));
}

SetIsDependentRelationshipCommand::~SetIsDependentRelationshipCommand()
{
}

void SetIsDependentRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(RelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_is_dependent ( NewValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetIsDependentRelationshipCommand::undo()
{
  SchemaRelationship->set_is_dependent ( OldValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetIsExclusiveRelationshipCommand::SetIsExclusiveRelationshipCommand (
  OksClass * Class, OksRelationship * Relationship, bool Value )
  : ClassName(Class->get_name()),
    SchemaRelationship ( Relationship ),
    RelationshipName ( Relationship->get_name() ),
    NewValue ( Value ),
    OldValue ( SchemaRelationship->get_is_exclusive() )
{
    setText( QString::fromStdString("Changed the \"is exclusive\" constraint for relationship \"" + Relationship->get_name() + "\" of class \"" + ClassName + "\""));
}

SetIsExclusiveRelationshipCommand::~SetIsExclusiveRelationshipCommand()
{
}

void SetIsExclusiveRelationshipCommand::redo()
{
  OksRelationship* r = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_relationship(RelationshipName);
  if(r != nullptr) {
      SchemaRelationship = r;
  }

  SchemaRelationship->set_is_exclusive ( NewValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetIsExclusiveRelationshipCommand::undo()
{
  SchemaRelationship->set_is_exclusive ( OldValue );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

AddRelationship::AddRelationship ( OksClass * Class, std::string Name,
                                   std::string Description,
                                   std::string Type, bool Composite, bool Exclusive,
                                   bool Dependent,
                                   OksRelationship::CardinalityConstraint LowCc,
                                   OksRelationship::CardinalityConstraint HighCc )
  : ClassName ( Class->get_name() ),
    SchemaRelationship ( nullptr ),
    RelationshipName ( Name ),
    RelationshipDescription ( Description ),
    RelationshipType ( Type ),
    IsComposite ( Composite ),
    IsExclusive ( Exclusive ),
    IsDependent ( Dependent ),
    RelationshipLowCc ( LowCc ),
    RelationshipHighCc ( HighCc )
{
    setText( QString::fromStdString("Added relationship \"" + Name + "\" to class \"" + Class->get_name() + "\""));
}

AddRelationship::~AddRelationship()
{
}

void AddRelationship::redo()
{
  if ( SchemaRelationship == nullptr )
  {
      SchemaRelationship = new OksRelationship (RelationshipName, RelationshipType, RelationshipLowCc, RelationshipHighCc,
                                                IsComposite, IsExclusive, IsDependent, RelationshipDescription );
  }

  KernelWrapper::GetInstance().FindClass(ClassName)->add ( SchemaRelationship );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void AddRelationship::undo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->remove ( SchemaRelationship, false );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

RemoveRelationship::RemoveRelationship ( OksClass * Class, OksRelationship * Relationship,
                                         std::string Name, std::string Description,
                                         std::string Type, bool Composite, bool Exclusive,
                                         bool Dependent,
                                         OksRelationship::CardinalityConstraint LowCc,
                                         OksRelationship::CardinalityConstraint HighCc )
  : ClassName ( Class->get_name() ),
    SchemaRelationship ( Relationship ),
    RelationshipName ( Name ),
    RelationshipDescription ( Description ),
    RelationshipType ( Type ),
    IsComposite ( Composite ),
    IsExclusive ( Exclusive ),
    IsDependent ( Dependent ),
    RelationshipLowCc ( LowCc ),
    RelationshipHighCc ( HighCc )
{
    setText( QString::fromStdString("Removed relationship \"" + Name + "\" to class \"" + Class->get_name() + "\""));
}

RemoveRelationship::~RemoveRelationship()
{
}

void RemoveRelationship::redo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->remove ( SchemaRelationship, false );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void RemoveRelationship::undo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->add ( SchemaRelationship );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetMethodImplementationLanguage::SetMethodImplementationLanguage (
  OksClass * Class, OksMethod* Method, OksMethodImplementation * Implementation, std::string Language )
  : ClassName(Class->get_name()),
    MethodName ( Method->get_name() ),
    SchemaImplementation ( Implementation ),
    NewLanguage ( Language ),
    OldLanguage ( SchemaImplementation->get_language() )
{
    setText( QString::fromStdString("Changed language implementation for \"" +  SchemaImplementation->get_prototype() + "\" of class \"" + Class->get_name() + "\""));
}

SetMethodImplementationLanguage::~SetMethodImplementationLanguage()
{
}

void SetMethodImplementationLanguage::redo()
{
  OksMethodImplementation* mi = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_method(MethodName)->find_implementation(OldLanguage);
  if(mi != nullptr) {
      SchemaImplementation = mi;
  }

  SchemaImplementation->set_language ( NewLanguage );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetMethodImplementationLanguage::undo()
{
  SchemaImplementation->set_language ( OldLanguage );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetMethodImplementationPrototype::SetMethodImplementationPrototype (
  OksClass * Class, OksMethod * Method, OksMethodImplementation * Implementation, std::string Prototype )
  : ClassName(Class->get_name()),
    MethodName ( Method->get_name() ),
    SchemaImplementation ( Implementation ),
    ImplementationLanguage ( Implementation->get_language()),
    NewPrototype ( Prototype ),
    OldPrototype ( SchemaImplementation->get_prototype() )
{
    setText( QString::fromStdString("Changed prototype for method \"" +  SchemaImplementation->get_prototype() + "\" of class \"" + ClassName + "\""));
}

SetMethodImplementationPrototype::~SetMethodImplementationPrototype()
{
}

void SetMethodImplementationPrototype::redo()
{
  OksMethodImplementation* mi = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_method(MethodName)->find_implementation(ImplementationLanguage);
  if(mi != nullptr) {
      SchemaImplementation = mi;
  }

  SchemaImplementation->set_prototype ( NewPrototype );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetMethodImplementationPrototype::undo()
{
  SchemaImplementation->set_prototype ( OldPrototype );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetMethodImplementationBody::SetMethodImplementationBody (
  OksClass * Class, OksMethod * Method, OksMethodImplementation * Implementation, std::string Body )
  : ClassName(Class->get_name()),
    MethodName ( Method->get_name()),
    SchemaImplementation ( Implementation ),
    ImplementationLanguage ( Implementation->get_language() ),
    NewBody ( Body ),
    OldBody ( SchemaImplementation->get_body() )
{
    setText( QString::fromStdString("Changed body implementation for method \"" +  SchemaImplementation->get_prototype() + "\" of class \"" + Class->get_name() + "\""));
}

SetMethodImplementationBody::~SetMethodImplementationBody()
{
}

void SetMethodImplementationBody::redo()
{
  OksMethodImplementation* mi = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_method(MethodName)->find_implementation(ImplementationLanguage);
  if(mi != nullptr) {
      SchemaImplementation = mi;
  }

  SchemaImplementation->set_body ( NewBody );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetMethodImplementationBody::undo()
{
  SchemaImplementation->set_body ( OldBody );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

AddMethodImplementationComand::AddMethodImplementationComand ( OksClass * Class,
                                                               OksMethod * Method,
                                                               std::string Language,
                                                               std::string Prototype,
                                                               std::string Body )
  : ClassName(Class->get_name()),
    SchemaMethod ( Method ),
    MethodName ( Method->get_name() ),
    SchemaImplementationLanguage ( Language ),
    SchemaImplementationPrototype ( Prototype ),
    SchemaImplementationBody ( Body )
{
    setText( QString::fromStdString("Added implementation for method \"" +  SchemaMethod->get_name() + "\" for class \"" + Class->get_name() + "\""));
}

AddMethodImplementationComand::~AddMethodImplementationComand()
{
}

void AddMethodImplementationComand::redo()
{
  OksMethod* m = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_method(MethodName);
  if(m != nullptr) {
      SchemaMethod = m;
  }

  SchemaMethod->add_implementation ( SchemaImplementationLanguage,
                                     SchemaImplementationPrototype,
                                     SchemaImplementationBody );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void AddMethodImplementationComand::undo()
{
  SchemaMethod->remove_implementation ( SchemaImplementationLanguage );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

RemoveMethodImplementationComand::RemoveMethodImplementationComand ( OksClass * Class,
                                                                     OksMethod * Method,
                                                                     std::string Language,
                                                                     std::string Prototype,
                                                                     std::string Body )
  : ClassName(Class->get_name()),
    SchemaMethod ( Method ),
    MethodName ( Method->get_name() ),
    SchemaImplementationLanguage ( Language ),
    SchemaImplementationPrototype ( Prototype ),
    SchemaImplementationBody ( Body )
{
    setText( QString::fromStdString("Removed implementation for method \"" +  SchemaMethod->get_name() + "\" for class \"" + Class->get_name() + "\""));
}

RemoveMethodImplementationComand::~RemoveMethodImplementationComand()
{
}

void RemoveMethodImplementationComand::redo()
{
  OksMethod* m = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_method(MethodName);
  if(m != nullptr) {
      SchemaMethod = m;
  }

  SchemaMethod->remove_implementation ( SchemaImplementationLanguage );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void RemoveMethodImplementationComand::undo()
{
  SchemaMethod->add_implementation ( SchemaImplementationLanguage,
                                     SchemaImplementationPrototype,
                                     SchemaImplementationBody );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetNameMethodCommand::SetNameMethodCommand ( OksClass * Class, OksMethod * Method, std::string name )
  : ClassName(Class->get_name()),
    SchemaMethod ( Method ),
    NewMethodName ( name ),
    OldMethodName ( SchemaMethod->get_name() )
{
    setText( QString::fromStdString("Set name for method \"" +  SchemaMethod->get_name() + "\" of class \"" + Class->get_name() + "\""));
}

SetNameMethodCommand::~SetNameMethodCommand()
{
}

void SetNameMethodCommand::redo()
{
  OksMethod * m = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_method(OldMethodName);
  if(m != nullptr) {
      SchemaMethod = m;
  }

  SchemaMethod->set_name ( NewMethodName );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetNameMethodCommand::undo()
{
  SchemaMethod->set_name ( OldMethodName );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetDescriptionMethodCommand::SetDescriptionMethodCommand ( OksClass * Class,
                                                           OksMethod * Method,
                                                           std::string description )
  : ClassName(Class->get_name()),
    SchemaMethod ( Method ),
    MethodName ( Method->get_name() ),
    NewMethodDescription ( description ),
    OldMethodDescription ( SchemaMethod->get_description() )
{
    setText( QString::fromStdString("Set description for method \"" +  SchemaMethod->get_name() + "\" of class \"" + Class->get_name() + "\""));
}

SetDescriptionMethodCommand::~SetDescriptionMethodCommand()
{
}

void SetDescriptionMethodCommand::redo()
{
  OksMethod * m = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_method(MethodName);
  if(m != nullptr) {
      SchemaMethod = m;
  }
  SchemaMethod->set_description ( NewMethodDescription );

  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetDescriptionMethodCommand::undo()
{
  SchemaMethod->set_description ( OldMethodDescription );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

AddMethodCommand::AddMethodCommand ( OksClass * Class, std::string name,
                                     std::string description )
  : ClassName ( Class->get_name() ),
    SchemaMethod ( nullptr ),
    SchemaName ( name ),
    SchemaDescription ( description )
{
    setText( QString::fromStdString("Added method \"" + name + "\" to class \"" +  Class->get_name() + "\""));
}

AddMethodCommand::~AddMethodCommand()
{
}

void AddMethodCommand::redo()
{
  SchemaMethod = new OksMethod ( SchemaName, SchemaDescription );
  KernelWrapper::GetInstance().FindClass(ClassName)->add ( SchemaMethod );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void AddMethodCommand::undo()
{
  // Do not delete SchemaMethod (probably deleted internally by the OKS library)
  KernelWrapper::GetInstance().FindClass(ClassName)->remove ( SchemaMethod );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

RemoveMethodCommand::RemoveMethodCommand ( OksClass * Class, OksMethod * Method,
                                           std::string name, std::string description )
  : ClassName ( Class->get_name() ),
    SchemaMethod ( Method ),
    SchemaName ( name ),
    SchemaDescription ( description )
{
    setText( QString::fromStdString("Removed method \"" + name + "\" to class \"" + Class->get_name() + "\""));
}

RemoveMethodCommand::~RemoveMethodCommand()
{
}

void RemoveMethodCommand::redo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->remove ( SchemaMethod );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void RemoveMethodCommand::undo()
{
  SchemaMethod = new OksMethod ( SchemaName, SchemaDescription );
  KernelWrapper::GetInstance().FindClass(ClassName)->add ( SchemaMethod );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeNameCommand::SetAttributeNameCommand ( OksClass * Class,
                                                   OksAttribute * Attribute,
                                                   std::string NewName )
  : ClassName( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    NewAttributeName ( NewName ),
    OldAttributeName ( SchemaAttribute->get_name() )
{
    setText( QString::fromStdString("Set new name \"" + NewName + "\" to attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeNameCommand::~SetAttributeNameCommand()
{
}

void SetAttributeNameCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(OldAttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_name ( NewAttributeName );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeNameCommand::undo()
{
  SchemaAttribute->set_name ( OldAttributeName );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeTypeCommand::SetAttributeTypeCommand ( OksClass * Class,
                                                   OksAttribute * Attribute,
                                                   std::string NewType )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    AttributeName ( Attribute->get_name() ),
    NewAttributeType ( NewType ),
    OldAttributeType ( SchemaAttribute->get_type() )
{
    setText( QString::fromStdString("Set new type \"" + NewType + "\" to attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeTypeCommand::~SetAttributeTypeCommand()
{
}

void SetAttributeTypeCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(AttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_type ( NewAttributeType );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeTypeCommand::undo()
{
  SchemaAttribute->set_type ( OldAttributeType );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeRangeCommand::SetAttributeRangeCommand ( OksClass * Class,
                                                     OksAttribute * Attribute,
                                                     std::string NewRange )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    AttributeName ( Attribute->get_name() ),
    NewAttributeRange ( NewRange ),
    OldAttributeRange ( SchemaAttribute->get_range() )
{
    setText( QString::fromStdString("Set new range \"" + NewRange + "\" to attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeRangeCommand::~SetAttributeRangeCommand()
{
}

void SetAttributeRangeCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(AttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_range ( NewAttributeRange );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeRangeCommand::undo()
{
  SchemaAttribute->set_range ( OldAttributeRange );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeFormatCommand::SetAttributeFormatCommand ( OksClass * Class,
                                                       OksAttribute * Attribute,
                                                       OksAttribute::Format NewFormat )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    AttributeName ( Attribute->get_name() ),
    NewAttributeFormat ( NewFormat ),
    OldAttributeFormat ( SchemaAttribute->get_format() )
{
    setText( QString::fromStdString("Set new format for attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeFormatCommand::~SetAttributeFormatCommand()
{
}

void SetAttributeFormatCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(AttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_format ( NewAttributeFormat );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeFormatCommand::undo()
{
  SchemaAttribute->set_format ( OldAttributeFormat );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeMultiCommand::SetAttributeMultiCommand ( OksClass* Class,
                                                     OksAttribute * Attribute,
                                                     bool NewIsMulti )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    AttributeName ( Attribute->get_name() ),
    NewAttributeMulti ( NewIsMulti ),
    OldAttributeMulti ( SchemaAttribute->get_is_multi_values() )
{
    setText( QString::fromStdString("Set multiplicity for attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeMultiCommand::~SetAttributeMultiCommand()
{
}

void SetAttributeMultiCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(AttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_is_multi_values ( NewAttributeMulti );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeMultiCommand::undo()
{
  SchemaAttribute->set_is_multi_values ( OldAttributeMulti );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeIsNullCommand::SetAttributeIsNullCommand ( OksClass* Class,
                                                       OksAttribute * Attribute,
                                                       bool NewIsNull )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    AttributeName ( Attribute->get_name() ),
    NewAttributeIsNull ( NewIsNull ),
    OldAttributeIsNull ( SchemaAttribute->get_is_no_null() )
{
    setText( QString::fromStdString("Set null-ness for attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeIsNullCommand::~SetAttributeIsNullCommand()
{
}

void SetAttributeIsNullCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(AttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_is_no_null ( NewAttributeIsNull );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeIsNullCommand::undo()
{
  SchemaAttribute->set_is_no_null ( OldAttributeIsNull );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeInitialValuesCommand::SetAttributeInitialValuesCommand (
  OksClass * Class,
  OksAttribute * Attribute,
  std::string NewValues )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    AttributeName ( Attribute->get_name() ),
    NewAttributeInitialValues ( NewValues ),
    OldAttributeInitialValues ( SchemaAttribute->get_init_value() )
{
    setText( QString::fromStdString("Set new initial values to attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeInitialValuesCommand::~SetAttributeInitialValuesCommand()
{
}

void SetAttributeInitialValuesCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(AttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_init_value ( NewAttributeInitialValues );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeInitialValuesCommand::undo()
{
  SchemaAttribute->set_init_value ( OldAttributeInitialValues );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

AddAttributeCommand::AddAttributeCommand ( OksClass * Class, std::string name,
                                           std::string type, bool is_mv, std::string range,
                                           std::string init_values, std::string description,
                                           bool is_null, OksAttribute::Format format )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( nullptr ),
    SchemaName ( name ),
    SchemaType ( type ),
    SchemaIsMulti ( is_mv ),
    SchemaRange ( range ),
    SchemaInitValues ( init_values ),
    SchemaDescription ( description ),
    SchemaIsNull ( is_null ),
    SchemaFormat ( format )
{
    setText( QString::fromStdString("Added attribute \"" + name + "\" to class \"" +  Class->get_name() + "\""));
}

AddAttributeCommand::~AddAttributeCommand()
{
}

void AddAttributeCommand::redo()
{
  SchemaAttribute = new OksAttribute ( SchemaName, SchemaType, SchemaIsMulti, SchemaRange,
                                       SchemaInitValues, SchemaDescription, SchemaIsNull,
                                       SchemaFormat );
  KernelWrapper::GetInstance().FindClass(ClassName)->add ( SchemaAttribute );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void AddAttributeCommand::undo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->remove ( SchemaAttribute );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

RemoveAttributeCommand::RemoveAttributeCommand ( OksClass * Class, OksAttribute * Attribute,
                                                 std::string name, std::string type,
                                                 bool is_mv, std::string range,
                                                 std::string init_values,
                                                 std::string description, bool is_null,
                                                 OksAttribute::Format format )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    SchemaName ( name ),
    SchemaType ( type ),
    SchemaIsMulti ( is_mv ),
    SchemaRange ( range ),
    SchemaInitValues ( init_values ),
    SchemaDescription ( description ),
    SchemaIsNull ( is_null ),
    SchemaFormat ( format )
{
    setText( QString::fromStdString("Removed attribute \"" + name + "\" from class \"" +  Class->get_name() + "\""));
}

RemoveAttributeCommand::~RemoveAttributeCommand()
{
}

void RemoveAttributeCommand::redo()
{
  KernelWrapper::GetInstance().FindClass(ClassName)->remove ( SchemaAttribute );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void RemoveAttributeCommand::undo()
{
  SchemaAttribute = new OksAttribute ( SchemaName, SchemaType, SchemaIsMulti, SchemaRange,
                                       SchemaInitValues, SchemaDescription, SchemaIsNull,
                                       SchemaFormat );
  KernelWrapper::GetInstance().FindClass(ClassName)->add ( SchemaAttribute );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

SetAttributeDescriptionCommand::SetAttributeDescriptionCommand ( OksClass * Class,
                                                                 OksAttribute * Attribute,
                                                                 std::string NewDescription )
  : ClassName ( Class->get_name() ),
    SchemaAttribute ( Attribute ),
    AttributeName ( Attribute->get_name() ),
    NewAttributeDescription ( NewDescription ),
    OldAttributeDescription ( SchemaAttribute->get_description() )
{
    setText( QString::fromStdString("Set description for attribute \"" +  Attribute->get_name() + "\" of class \"" + ClassName + "\""));
}

SetAttributeDescriptionCommand::~SetAttributeDescriptionCommand()
{

}

void SetAttributeDescriptionCommand::redo()
{
  OksAttribute * a = KernelWrapper::GetInstance().FindClass(ClassName)->find_direct_attribute(AttributeName);
  if(a != nullptr) {
      SchemaAttribute = a;
  }

  SchemaAttribute->set_description ( NewAttributeDescription );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

void SetAttributeDescriptionCommand::undo()
{
  SchemaAttribute->set_description ( OldAttributeDescription );
  emit KernelWrapper::GetInstance().ClassUpdated ( QString::fromStdString(ClassName) );
}

} // end namespace dbse
