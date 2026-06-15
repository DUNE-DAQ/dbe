// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/SchemaCustomSuperClassModel.h to include/dbe/SchemaCustomSuperClassModel.hpp).

#ifndef CUSTOMSUPERCLASSMODEL_H
#define CUSTOMSUPERCLASSMODEL_H

/// Include oks
#include "oks/class.hpp"
#include "oks/relationship.hpp"
/// Including Schema
#include "dbe/SchemaCustomModelInterface.hpp"

namespace dbse
{

class CustomSuperClassModel: public CustomModelInterface
{
public:
  CustomSuperClassModel ( dunedaq::oks::OksClass * ClassInfo, QStringList Headers, bool Derived = false );
  ~CustomSuperClassModel();
  void setupModel();
private:
  dunedaq::oks::OksClass * SchemaClass;
  bool SchemaDerived;
};

}  // namespace dbse
#endif // CUSTOMSUPERCLASSMODEL_H
