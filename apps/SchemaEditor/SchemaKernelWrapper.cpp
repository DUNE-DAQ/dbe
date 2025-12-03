/// Including QT
#include <QMessageBox>
/// Including Schema
#include "dbe/SchemaKernelWrapper.hpp"
/// Include oks
#include "oks/kernel.hpp"
#include "oks/class.hpp"
#include "oks/file.hpp"

using namespace dunedaq;
using namespace dunedaq::oks;

dbse::KernelWrapper & dbse::KernelWrapper::GetInstance()
{
  static KernelWrapper KernelManager;
  return KernelManager;
}

OksKernel * dbse::KernelWrapper::GetKernel()
{
  return Kernel;
}

void dbse::KernelWrapper::SetActiveSchema ( const std::string & ActiveSchema )
{
  OksFile * File = Kernel->find_schema_file ( ActiveSchema );

  if ( File && IsFileWritable( ActiveSchema ))
  {
    Kernel->set_active_schema ( File );
    emit active_updated();
  }
}

void dbse::KernelWrapper::GetClassList ( std::vector<OksClass *> & ClassList ) const
{
  for ( OksClass::Map::const_iterator i = Kernel->classes().begin();
        i != Kernel->classes().end(); ++i )
  {
    ClassList.push_back ( i->second );
  }
}

void dbse::KernelWrapper::GetClassListString ( QStringList & ClassListString ) const
{
  std::vector<OksClass *> ClassList;

  for ( OksClass::Map::const_iterator i = Kernel->classes().begin();
        i != Kernel->classes().end(); ++i )
  {
    ClassList.push_back ( i->second );
  }

  for ( OksClass * ClassInfo : ClassList )
  {
    ClassListString.append ( QString::fromStdString ( ClassInfo->get_name() ) );
  }
}

std::string dbse::KernelWrapper::GetActiveSchema () const
{
  auto file = Kernel->get_active_schema();
  if ( file) {
    return file->get_full_file_name();
  }
  else {
    return "";
  }
}

// void dbse::KernelWrapper::AddInclude( std::string IncludeFile ) const
// {
//   auto ActiveSchema = Kernel->get_active_schema();
//   ActiveSchema->add_include_file( IncludeFile );
//   Kernel->load_schema( IncludeFile, ActiveSchema );
// }
void dbse::KernelWrapper::AddInclude( std::string schemaFile, std::string IncludeFile ) const
{
  auto ParentSchema = Kernel->find_schema_file( schemaFile );
  if ( ParentSchema != nullptr) {
    ParentSchema->add_include_file( IncludeFile );
    Kernel->load_schema( IncludeFile, ParentSchema );
  }
}

void dbse::KernelWrapper::RemoveInclude( std::string schemaFile, std::string IncludeFile ) const
{
  auto ParentSchema = Kernel->find_schema_file( schemaFile );
  if (ParentSchema != nullptr) {
    ParentSchema->remove_include_file( IncludeFile );
  }
}

void dbse::KernelWrapper::GetSchemaFiles ( std::vector<std::string> & SchemaFiles )
{
  for ( OksFile::Map::const_iterator i = Kernel->schema_files().begin();
        i != Kernel->schema_files().end(); ++i )
  {
    SchemaFiles.push_back ( * ( i->first ) );
  }
}

void dbse::KernelWrapper::GetSchemaFiles ( std::vector<OksFile*> & SchemaFiles )
{
  for (auto [name, file]:  Kernel->schema_files())
  {
    SchemaFiles.push_back ( file );
  }
}

std::vector<OksClass*>
dbse::KernelWrapper::get_schema_classes (std::string& filename)
{
  auto file = Kernel->find_schema_file ( filename );
  std::vector<OksClass*> list;
  auto klist = Kernel->create_list_of_schema_classes(file);
  if (klist != nullptr) {
    for (auto cls: *klist) {
      list.push_back(cls/*->get_name()*/);
    }
    delete klist;
  }
  return list;
}

void dbse::KernelWrapper::GetIncludedList ( const std::string & filename,
                                            std::set<std::string> & included_files )
{
  Kernel->get_includes ( filename, included_files );
}
void dbse::KernelWrapper::get_all_includes ( const std::string & filename,
                                            std::set<std::string> & included_files )
{
  auto parentschema = Kernel->find_schema_file( filename );
  if ( parentschema != nullptr) {
    std::set<OksFile*> includes;
    parentschema->get_all_include_files(Kernel, includes);
    for (auto file : includes) {
      included_files.insert(file->get_full_file_name());
    }
  }
}

void dbse::KernelWrapper::get_direct_includes ( const std::string & filename,
                                            std::set<std::string> & included_files )
{
  auto parentschema = Kernel->find_schema_file( filename );
  if ( parentschema != nullptr) {
    for (auto file : parentschema->get_include_files()) {
      included_files.insert(file);
    }
  }
}

bool dbse::KernelWrapper::IsFileWritable (const std::string & FileName ) const
{
  OksFile * File = Kernel->find_schema_file ( FileName );

  if ( File )
  {
    return ( !File->is_read_only() );
  }

  return false;
}

bool dbse::KernelWrapper::is_file_modified (const std::string & FileName ) const
{
  OksFile* file = Kernel->find_schema_file ( FileName );
  if (file != nullptr) {
    return file->is_updated();
  }
  return false;
}

OksClass * dbse::KernelWrapper::FindClass ( std::string ClassName ) const
{
  return Kernel->find_class ( ClassName );
}

OksFile* dbse::KernelWrapper::LoadSchema ( const std::string & SchemaName ) const
{
  return Kernel->load_schema ( SchemaName );
}

void dbse::KernelWrapper::SaveAllSchema() const
{
  Kernel->save_all_schema();
}

std::string dbse::KernelWrapper::ModifiedSchemaFiles() const
{
  std::string modified{""};
  for (auto [name, file] : Kernel->schema_files()) {
    if (file->is_updated()) {
      modified += file->get_full_file_name() + "\n\n";
    }
  }
  return modified;
}

std::vector<std::string> dbse::KernelWrapper::get_modified_schema_files() const
{
  std::vector<std::string> modified;
  for (auto [name, file] : Kernel->schema_files()) {
    if (file->is_updated()) {
      modified.push_back(file->get_full_file_name());
    }
  }
  return modified;
}

std::string dbse::KernelWrapper::SaveModifiedSchema() const
{
  std::string saved{""};
  for (auto [name, file] : Kernel->schema_files()) {
    if (file->is_updated()) {
      Kernel->save_schema(file);
      saved += file->get_full_file_name() + "\n\n";
    }
  }
  return saved;
}

void dbse::KernelWrapper::SaveSchema(const std::string& schema_file) const
{
  OksFile * file = Kernel->find_schema_file ( schema_file );
  Kernel->save_schema(file);
}


void dbse::KernelWrapper::CloseAllSchema() const
{
  Kernel->close_all_schema();
}

void dbse::KernelWrapper::CreateNewSchema ( const std::string & SchemaName ) const
{
  Kernel->new_schema ( SchemaName );
}

bool dbse::KernelWrapper::AnyClassReferenceThis ( OksClass * SchemaClass )
{
  std::vector<OksClass *> ClassList;
  GetClassList ( ClassList );

  for ( OksClass * ClassInfo : ClassList )
  {
    const std::list<OksRelationship *> * RelationshipList = ClassInfo->direct_relationships();

    if ( RelationshipList != nullptr )
    {
      for ( OksRelationship * RelationshipInfo : *RelationshipList )
      {
        if ( RelationshipInfo->get_class_type()->get_name() == SchemaClass->get_name() )
        {
          return true;
        }
      }
    }
  }

  return false;
}

QString dbse::KernelWrapper::GetCardinalityStringRelationship (
  OksRelationship * SchemaRelationship ) const
{
  QString RelationshipLowCc (
    OksRelationship::card2str ( SchemaRelationship->get_low_cardinality_constraint() ) );
  QString RelationshipHighCc (
    OksRelationship::card2str ( SchemaRelationship->get_high_cardinality_constraint() ) );

  if ( RelationshipLowCc == "zero" )
  {
    RelationshipLowCc = "0";
  }
  else if ( RelationshipLowCc == "one" )
  {
    RelationshipLowCc = "1";
  }
  else if ( RelationshipLowCc == "many" )
  {
    RelationshipLowCc = "n";
  }

  if ( RelationshipHighCc == "zero" )
  {
    RelationshipHighCc = "0";
  }
  else if ( RelationshipHighCc == "one" )
  {
    RelationshipHighCc = "1";
  }
  else if ( RelationshipHighCc == "many" )
  {
    RelationshipHighCc = "n";
  }

  return QString ( RelationshipLowCc + ':' + RelationshipHighCc );
}

bool dbse::KernelWrapper::IsActive() const
{
  OksFile * ActiveSchemaFile = Kernel->get_active_schema();
  return ( ActiveSchemaFile != nullptr );
}

void dbse::KernelWrapper::SetInheritanceMode ( bool Mode )
{
  InheritanceMode = Mode;
}

bool dbse::KernelWrapper::GetInheritanceMode() const
{
  return InheritanceMode;
}

QUndoStack * dbse::KernelWrapper::GetUndoStack()
{
  return CommandStack;
}

void dbse::KernelWrapper::PushSetAbstractClassCommand ( OksClass * Class, bool Value )
{
  try
  {
    CommandStack->push ( new SetAbstractClassCommand ( Class, Value ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetDescriptionClassCommand ( OksClass * Class,
                                                           std::string Description )
{
  try
  {
    CommandStack->push ( new SetDescriptionClassCommand ( Class, Description ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushAddSuperClassCommand ( OksClass * Class,
                                                     std::string SuperClass )
{
  try
  {
    CommandStack->push ( new AddSuperClassCommand ( Class, SuperClass ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushRemoveSuperClassCommand ( OksClass * Class,
                                                     std::string SuperClass )
{
  try
  {
    CommandStack->push ( new RemoveSuperClassCommand ( Class, SuperClass ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushCreateClassCommand ( std::string ClassName,
                                                   std::string ClassDescription,
                                                   bool Abstract )
{
  try
  {
    CommandStack->push ( new CreateClassCommand ( ClassName, ClassDescription, Abstract ) );
  }
  catch ( ... )
  {
    QMessageBox::warning ( 0, "Schema editor", QString ( "Error" ) );
  }
}

void dbse::KernelWrapper::PushRemoveClassCommand ( OksClass * Class, std::string ClassName,
                                                   std::string ClassDescription, bool Abstract )
{
  try
  {
    CommandStack->push ( new RemoveClassCommand ( Class, ClassName, ClassDescription,
                                                  Abstract ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetNameRelationshipCommand ( OksClass* Class,
                                                           OksRelationship * Relationship,
                                                           std::string Name )
{
  try
  {
    CommandStack->push ( new SetNameRelationshipCommand ( Class, Relationship, Name ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetClassTypeRelationshipCommand ( OksClass* Class,
                                                                OksRelationship * Relationship,
                                                                std::string ClassType )
{
  try
  {
    CommandStack->push ( new SetClassTypeRelationshipCommand ( Class, Relationship, ClassType ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetDescriptionRelationshipCommand (
  OksClass* Class,
  OksRelationship * Relationship,
  std::string Description )
{
  try
  {
    CommandStack->push ( new SetDescriptionRelationshipCommand ( Class, Relationship, Description ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetLowCcRelationshipCommand (
  OksClass* Class, OksRelationship * Relationship, OksRelationship::CardinalityConstraint NewCardinality )
{
  try
  {
    CommandStack->push ( new SetLowCcRelationshipCommand ( Class, Relationship, NewCardinality ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetHighCcRelationshipCommand (
  OksClass* Class, OksRelationship * Relationship, OksRelationship::CardinalityConstraint NewCardinality )
{
  try
  {
    CommandStack->push ( new SetHighCcRelationshipCommand ( Class, Relationship, NewCardinality ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetIsCompositeRelationshipCommand (
  OksClass* Class, OksRelationship * Relationship,
  bool Value )
{
  try
  {
    CommandStack->push ( new SetIsCompositeRelationshipCommand ( Class, Relationship, Value ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetIsDependentRelationshipCommand (
  OksClass* Class,
  OksRelationship * Relationship,
  bool Value )
{
  try
  {
    CommandStack->push ( new SetIsDependentRelationshipCommand ( Class, Relationship, Value ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetIsExclusiveRelationshipCommand (
  OksClass* Class,
  OksRelationship * Relationship,
  bool Value )
{
  try
  {
    CommandStack->push ( new SetIsExclusiveRelationshipCommand ( Class, Relationship, Value ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushAddRelationship ( OksClass * Class, std::string Name,
                                                std::string Description, std::string Type,
                                                bool Composite, bool Exclusive, bool Dependent,
                                                OksRelationship::CardinalityConstraint LowCc,
                                                OksRelationship::CardinalityConstraint HighCc )
{
  try
  {
    CommandStack->push (
      new AddRelationship ( Class, Name, Description, Type, Composite, Exclusive, Dependent,
                            LowCc, HighCc ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushRemoveRelationship ( OksClass * Class,
                                                   OksRelationship * Relationship,
                                                   std::string Name, std::string Description,
                                                   std::string Type, bool Composite, bool Exclusive,
                                                   bool Dependent,
                                                   OksRelationship::CardinalityConstraint LowCc,
                                                   OksRelationship::CardinalityConstraint HighCc )
{
  try
  {
    CommandStack->push (
      new RemoveRelationship ( Class, Relationship, Name, Description, Type, Composite,
                               Exclusive, Dependent, LowCc, HighCc ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetMethodImplementationLanguage ( OksClass * Class,
                                                                OksMethod * Method,
                                                                OksMethodImplementation * Implementation,
                                                                std::string Language )
{
  try
  {
    CommandStack->push ( new SetMethodImplementationLanguage ( Class, Method, Implementation, Language ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetMethodImplementationPrototype (
  OksClass * Class, OksMethod * Method, OksMethodImplementation * Implementation, std::string Prototype )
{
  try
  {
    CommandStack->push ( new SetMethodImplementationPrototype ( Class, Method, Implementation, Prototype ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetMethodImplementationBody ( OksClass * Class,
                                                            OksMethod * Method,
                                                            OksMethodImplementation * Implementation,
                                                            std::string Body )
{
  try
  {
    CommandStack->push ( new SetMethodImplementationBody ( Class, Method, Implementation, Body ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushAddMethodImplementationComand ( OksClass * Class,
                                                              OksMethod * Method,
                                                              std::string Language,
                                                              std::string Prototype, std::string Body )
{
  try
  {
    CommandStack->push ( new AddMethodImplementationComand ( Class, Method, Language, Prototype,
                                                             Body ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushRemoveMethodImplementationComand ( OksClass * Class,
                                                                 OksMethod * Method,
                                                                 std::string Language,
                                                                 std::string Prototype,
                                                                 std::string Body )
{
  try
  {
    CommandStack->push ( new RemoveMethodImplementationComand ( Class, Method, Language, Prototype,
                                                                Body ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetNameMethodCommand ( OksClass * Class, OksMethod * Method, std::string name )
{
  try
  {
    CommandStack->push ( new SetNameMethodCommand ( Class, Method, name ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetDescriptionMethodCommand ( OksClass * Class, OksMethod * Method,
                                                            std::string description )
{
  try
  {
    CommandStack->push ( new SetDescriptionMethodCommand ( Class, Method, description ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushAddMethodCommand ( OksClass * Class, std::string name,
                                                 std::string description )
{
  try
  {
    CommandStack->push ( new AddMethodCommand ( Class, name, description ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushRemoveMethodCommand ( OksClass * Class, OksMethod * Method,
                                                    std::string name, std::string description )
{
  try
  {
    CommandStack->push ( new RemoveMethodCommand ( Class, Method, name, description ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeNameCommand ( OksClass * Class,
                                                        OksAttribute * Attribute,
                                                        std::string NewName )
{
  try
  {
    CommandStack->push ( new SetAttributeNameCommand ( Class, Attribute, NewName ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeDescriptionCommand ( OksClass * Class,
                                                               OksAttribute * Attribute,
                                                               std::string NewDescription )
{
  try
  {
    CommandStack->push ( new SetAttributeDescriptionCommand ( Class, Attribute, NewDescription ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeTypeCommand ( OksClass * Class,
                                                        OksAttribute * Attribute,
                                                        std::string NewType )
{
  try
  {
    CommandStack->push ( new SetAttributeTypeCommand ( Class, Attribute, NewType ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeRangeCommand ( OksClass * Class,
                                                         OksAttribute * Attribute,
                                                         std::string NewRange )
{
  try
  {
    CommandStack->push ( new SetAttributeRangeCommand ( Class, Attribute, NewRange ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeFormatCommand ( OksClass * Class,
                                                          OksAttribute * Attribute,
                                                          OksAttribute::Format NewFormat )
{
  try
  {
    CommandStack->push ( new SetAttributeFormatCommand ( Class, Attribute, NewFormat ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeMultiCommand ( OksClass * Class,
                                                         OksAttribute * Attribute,
                                                         bool NewIsMulti )
{
  try
  {
    CommandStack->push ( new SetAttributeMultiCommand ( Class, Attribute, NewIsMulti ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeIsNullCommand ( OksClass * Class,
                                                          OksAttribute * Attribute,
                                                          bool NewIsNull )
{
  try
  {
    CommandStack->push ( new SetAttributeIsNullCommand ( Class, Attribute, NewIsNull ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushSetAttributeInitialValuesCommand ( OksClass * Class,
                                                                 OksAttribute * Attribute,
                                                                 std::string NewValues )
{
  try
  {
    CommandStack->push ( new SetAttributeInitialValuesCommand ( Class, Attribute, NewValues ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushAddAttributeCommand ( OksClass * Class, std::string name,
                                                    std::string type,
                                                    bool is_mv, std::string range,
                                                    std::string init_values, std::string description,
                                                    bool is_null, OksAttribute::Format format )
{
  try
  {
    CommandStack->push (
      new AddAttributeCommand ( Class, name, type, is_mv, range, init_values, description,
                                is_null, format ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

void dbse::KernelWrapper::PushRemoveAttributeCommand ( OksClass * Class,
                                                       OksAttribute * Attribute,
                                                       std::string name, std::string type, bool is_mv,
                                                       std::string range, std::string init_values,
                                                       std::string description, bool is_null,
                                                       OksAttribute::Format format )
{
  try
  {
    CommandStack->push (
      new RemoveAttributeCommand ( Class, Attribute, name, type, is_mv, range, init_values,
                                   description, is_null, format ) );
  }
  catch ( oks::exception & Ex )
  {
    QMessageBox::warning ( 0, "Schema editor",
                           QString ( "Error : \n\n %1" ).arg ( Ex.what() ) );
  }
}

dbse::KernelWrapper::KernelWrapper ( QObject * parent )
  : QObject ( parent ),
    Kernel ( new OksKernel() ),
    CommandStack ( new QUndoStack() ),
    InheritanceMode ( false )
{
}
