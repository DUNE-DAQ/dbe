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
