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
