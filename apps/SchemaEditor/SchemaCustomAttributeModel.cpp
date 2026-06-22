// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from src/SchemaEditor/SchemaCustomAttributeModel.cpp to apps/SchemaEditor/SchemaCustomAttributeModel.cpp).

#include "dbe/SchemaCustomAttributeModel.hpp"

using namespace dunedaq::oks;

dbse::CustomAttributeModel::CustomAttributeModel ( OksClass * ClassInfo,
                                                   QStringList Headers,
                                                   bool Derived )
  : CustomModelInterface ( Headers ),
    SchemaClass ( ClassInfo ),
    SchemaDerived ( Derived )
{
  setupModel();
}

void dbse::CustomAttributeModel::setupModel()
{
  Data.clear();
  const std::list<OksAttribute *> * AttributeList;

  if ( SchemaDerived )
  {
    AttributeList = SchemaClass->all_attributes();
  }
  else
  {
    AttributeList = SchemaClass->direct_attributes();
  }

  if ( AttributeList )
  {
    for ( OksAttribute * Attribute : *AttributeList )
    {
      QStringList Row;
      Row.append ( QString::fromStdString ( Attribute->get_name() ) );
      Row.append ( QString::fromStdString ( Attribute->get_type() ) );
      Data.append ( Row );
    }
  }
}

dbse::CustomAttributeModel::~CustomAttributeModel() = default;
