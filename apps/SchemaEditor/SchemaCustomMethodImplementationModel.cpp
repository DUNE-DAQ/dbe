// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from src/SchemaEditor/SchemaCustomMethodImplementationModel.cpp to apps/SchemaEditor/SchemaCustomMethodImplementationModel.cpp).

#include "dbe/SchemaCustomMethodImplementationModel.hpp"

using namespace dunedaq::oks;

dbse::CustomMethodImplementationModel::CustomMethodImplementationModel ( OksMethod * Method,
                                                                         QStringList Headers )
  : CustomModelInterface ( Headers ),
    SchemaMethod ( Method )
{
  setupModel();
}

void dbse::CustomMethodImplementationModel::setupModel()
{
  Data.clear();
  const std::list<OksMethodImplementation *> * ImplementationList =
    SchemaMethod->implementations();

  if ( ImplementationList )
  {
    for ( OksMethodImplementation * Implementation : *ImplementationList )
    {
      QStringList Row;
      Row.append ( QString::fromStdString ( Implementation->get_language() ) );
      Row.append ( QString::fromStdString ( Implementation->get_prototype() ) );
      Data.append ( Row );
    }
  }
}

dbse::CustomMethodImplementationModel::~CustomMethodImplementationModel() = default;
