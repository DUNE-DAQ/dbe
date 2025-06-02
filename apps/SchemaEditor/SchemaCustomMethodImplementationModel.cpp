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
