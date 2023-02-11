#include "dbe/SchemaCustomSuperClassModel.hpp"

dbse::CustomSuperClassModel::CustomSuperClassModel ( OksClass * ClassInfo,
                                                     QStringList Headers, bool Derived )
  : CustomModelInterface ( Headers ),
    SchemaClass ( ClassInfo ),
    SchemaDerived ( Derived )
{
  setupModel();
}

void dbse::CustomSuperClassModel::setupModel()
{
  Data.clear();
  std::list<std::string> SuperClassList;

  if ( SchemaDerived )
  {
    const OksClass::FList* allClasses = SchemaClass->all_super_classes();
    if(allClasses != nullptr) {
        for(const OksClass* cl : *allClasses) {
            SuperClassList.push_back(cl->get_name());
        }
    }
  }
  else
  {
    const auto& directClasses = SchemaClass->direct_super_classes();
    if(directClasses != nullptr) {
        for(const std::string* cl : *directClasses) {
            SuperClassList.push_back(*cl);
        }
    }
  }

  for ( std::string Class : SuperClassList )
  {
    QStringList Row;
    Row.append ( QString::fromStdString ( Class ) );
    Data.append ( Row );
  }
}

dbse::CustomSuperClassModel::~CustomSuperClassModel() = default;
