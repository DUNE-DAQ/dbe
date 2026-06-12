/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/Validator.h to include/dbe/Validator.hpp).
 */

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QValidator>
#include <QStringList>

namespace dbe
{
class ValidatorAcceptMatch: public QValidator
{
  Q_OBJECT
public:
  ValidatorAcceptMatch ( QVariant & Storage, QObject * parent = 0 );
  QValidator::State validate ( QString & Input, int & Position ) const;
private:
  QStringList List;
};

class ValidatorAcceptNoMatch: public QValidator
{
  Q_OBJECT
public:
  ValidatorAcceptNoMatch ( QVariant & Storage, QObject * parent = 0 );
  QValidator::State validate ( QString & Input, int & Position ) const;
private:
  QStringList List;
};

} //end namespace dbe
#endif // VALIDATOR_H
