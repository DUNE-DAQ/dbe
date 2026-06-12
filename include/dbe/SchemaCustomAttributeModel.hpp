/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/SchemaCustomAttributeModel.h to include/dbe/SchemaCustomAttributeModel.hpp).
 */

#ifndef CUSTOMATTRIBUTEMODEL_H
#define CUSTOMATTRIBUTEMODEL_H

/// Including Schema
#include "dbe/SchemaCustomModelInterface.hpp"
/// Include oks
#include "oks/class.hpp"
#include "oks/attribute.hpp"

namespace dbse
{
class CustomAttributeModel: public CustomModelInterface
{
public:
  CustomAttributeModel ( dunedaq::oks::OksClass * ClassInfo, QStringList Headers, bool Derived = false );
  ~CustomAttributeModel();
  void setupModel();
private:
  dunedaq::oks::OksClass * SchemaClass;
  bool SchemaDerived;
};
}  // namespace dbse

#endif // CUSTOMATTRIBUTEMODEL_H
