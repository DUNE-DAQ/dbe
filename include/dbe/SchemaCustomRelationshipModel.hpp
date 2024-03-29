#ifndef CUSTOMRELATIONSHIPMODEL_H
#define CUSTOMRELATIONSHIPMODEL_H

/// Include oks
#include "oks/class.hpp"
#include "oks/relationship.hpp"
/// Including Schema
#include "dbe/SchemaCustomModelInterface.hpp"

namespace dbse
{

class CustomRelationshipModel: public CustomModelInterface
{
public:
  CustomRelationshipModel ( dunedaq::oks::OksClass * ClassInfo, QStringList Headers, bool Derived = false );
  ~CustomRelationshipModel();
  void setupModel();
private:
  dunedaq::oks::OksClass * SchemaClass;
  bool SchemaDerived;
};

}  // namespace dbse
#endif // CUSTOMRELATIONSHIPMODEL_H
