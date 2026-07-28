// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from src/SchemaEditor/SchemaCustomSubClassModel.cpp to apps/SchemaEditor/SchemaCustomSubClassModel.cpp).

#include "dbe/SchemaCustomSubClassModel.hpp"

using namespace dunedaq::oks;

dbse::CustomSubClassModel::CustomSubClassModel ( OksClass * ClassInfo,
                                                 QStringList Headers )
  : CustomModelInterface ( Headers ),
    SchemaClass ( ClassInfo )
{
  setupModel();
}

void dbse::CustomSubClassModel::setupModel()
{
  Data.clear();

  const OksClass::FList* allClasses = SchemaClass->all_sub_classes();
  if(allClasses != nullptr) {
      for(const OksClass* cl : *allClasses) {
          QStringList Row;
          Row.append ( QString::fromStdString ( cl->get_name() ) );
          Data.append ( Row );
      }
  }
}

dbse::CustomSubClassModel::~CustomSubClassModel() = default;
