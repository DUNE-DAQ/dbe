// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from src/SchemaEditor/SchemaCustomMethodModel.cpp to apps/SchemaEditor/SchemaCustomMethodModel.cpp).

#include "dbe/SchemaCustomMethodModel.hpp"

using namespace dunedaq::oks;

dbse::CustomMethodModel::CustomMethodModel ( OksClass * ClassInfo, QStringList Headers,
                                             bool Derived )
  : CustomModelInterface ( Headers ),
    SchemaClass ( ClassInfo ),
    SchemaDerived ( Derived )
{
  setupModel();
}

void dbse::CustomMethodModel::setupModel()
{
  Data.clear();
  const std::list<OksMethod *> * MethodList;
  std::set<std::string> direct_methods;
  auto methods = SchemaClass->direct_methods();
  if (methods != nullptr) {
    for (auto method : *methods) {
      direct_methods.insert(method->get_name());
    }
  }

  if ( SchemaDerived )
  {
    MethodList = SchemaClass->all_methods();
  }
  else
  {
    MethodList = SchemaClass->direct_methods();
  }

  if ( MethodList != nullptr )
  {
    for ( OksMethod * Method : *MethodList )
    {
      QStringList row;
      auto name = Method->get_name();
      row.append ( QString::fromStdString ( name ) );
      if (direct_methods.contains(name)) {
        row.append("D");
      }
      else {
        row.append("I");
      }
      Data.append ( row );
    }
  }
}

dbse::CustomMethodModel::~CustomMethodModel() = default;
