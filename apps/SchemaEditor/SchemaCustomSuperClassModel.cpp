#include "dbe/SchemaCustomSuperClassModel.hpp"

using namespace dunedaq::oks;

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

  std::set<std::string> direct_classes;
  const auto& directClasses = SchemaClass->direct_super_classes();
  if(directClasses != nullptr) {
    for(const std::string* cl : *directClasses) {
      direct_classes.insert(*cl);
    }
  }

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
    for (auto cls : direct_classes) {
      SuperClassList.push_back(cls);
    }
  }

  for ( std::string cls : SuperClassList )
  {
    QStringList row;
    row.append ( QString::fromStdString ( cls ) );
    if (direct_classes.contains(cls)) {
      row.append("D");
    }
    else {
      row.append("I");
    }
    Data.append ( row );
  }
}

dbse::CustomSuperClassModel::~CustomSuperClassModel() = default;
