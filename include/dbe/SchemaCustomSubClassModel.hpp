// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/SchemaCustomSubClassModel.h to include/dbe/SchemaCustomSubClassModel.hpp).

#ifndef CUSTOMSUBCLASSMODEL_H
#define CUSTOMSUBCLASSMODEL_H

/// Include oks
#include "oks/class.hpp"
#include "oks/relationship.hpp"
/// Including Schema
#include "dbe/SchemaCustomModelInterface.hpp"

namespace dbse
{

class CustomSubClassModel: public CustomModelInterface
{
public:
  CustomSubClassModel ( dunedaq::oks::OksClass * ClassInfo, QStringList Headers);
  ~CustomSubClassModel();
  void setupModel();
private:
  dunedaq::oks::OksClass * SchemaClass;
};

}  // namespace dbse
#endif // CUSTOMSUBCLASSMODEL_H
