/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/SchemaCustomMethodImplementationModel.h to include/dbe/SchemaCustomMethodImplementationModel.hpp).
 */

#ifndef CUSTOMMETHODIMPLEMENTATION_H
#define CUSTOMMETHODIMPLEMENTATION_H

/// Including Schema
#include "dbe/SchemaCustomModelInterface.hpp"
/// Include oks
#include "oks/method.hpp"

namespace dbse
{

class CustomMethodImplementationModel: public CustomModelInterface
{
public:
  CustomMethodImplementationModel ( dunedaq::oks::OksMethod * Method, QStringList Headers );
  ~CustomMethodImplementationModel();
  void setupModel();
private:
  dunedaq::oks::OksMethod * SchemaMethod;
};

}  // namespace dbse
#endif // CUSTOMMETHODIMPLEMENTATION_H
