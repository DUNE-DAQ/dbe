#ifndef CUSTOMMETHODMODEL_H
#define CUSTOMMETHODMODEL_H

/// Include Qt
#include <QStringList>
/// Including Schema
#include "SchemaCustomModelInterface.h"
/// Include oks
#include "oks/class.hpp"
#include "oks/method.hpp"

namespace dbse
{

class CustomMethodModel: public CustomModelInterface
{
public:
  CustomMethodModel ( OksClass * ClassInfo, QStringList Headers, bool Derived = false );
  ~CustomMethodModel();
  void setupModel();
private:
  OksClass * SchemaClass;
  bool SchemaDerived;
};

}  // namespace dbse

#endif // CUSTOMMETHODMODEL_H
