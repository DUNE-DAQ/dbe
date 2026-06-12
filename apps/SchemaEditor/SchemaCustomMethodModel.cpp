/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from src/SchemaEditor/SchemaCustomMethodModel.cpp to apps/SchemaEditor/SchemaCustomMethodModel.cpp).
 */

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

  if ( SchemaDerived )
  {
    MethodList = SchemaClass->all_methods();
  }
  else
  {
    MethodList = SchemaClass->direct_methods();
  }

  if ( MethodList )
  {
    for ( OksMethod * Method : *MethodList )
    {
      QStringList Row;
      Row.append ( QString::fromStdString ( Method->get_name() ) );
      Data.append ( Row );
    }
  }
}

dbse::CustomMethodModel::~CustomMethodModel() = default;
