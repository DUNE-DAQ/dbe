#ifndef SCHEMAMETHODIMPLEMENTATIONEDITOR_H
#define SCHEMAMETHODIMPLEMENTATIONEDITOR_H

#include <memory>
#include <QWidget>
#include "oks/method.hpp"
#include "oks/class.hpp"

namespace dbse
{

namespace Ui
{
class SchemaMethodImplementationEditor;
}  // namespace Ui

class SchemaMethodImplementationEditor: public QWidget
{
  Q_OBJECT
public:
  ~SchemaMethodImplementationEditor();

  explicit SchemaMethodImplementationEditor ( dunedaq::oks::OksClass * Class,
                                              dunedaq::oks::OksMethod * Method,
                                              dunedaq::oks::OksMethodImplementation * Implementation,
                                              QWidget * parent = nullptr );

  explicit SchemaMethodImplementationEditor ( dunedaq::oks::OksClass * Class,
                                              dunedaq::oks::OksMethod * Method,
                                              QWidget * parent = nullptr );

  void SetController();
  void InitialSettings();
  void ParseToSave();
  void ParseToCreate();
protected:
  void FillInfo();
private:
  std::unique_ptr<dbse::Ui::SchemaMethodImplementationEditor> ui;
  dunedaq::oks::OksClass * m_class;
  dunedaq::oks::OksMethod * m_method;
  dunedaq::oks::OksMethodImplementation * m_implementation;
  bool UsedNew;
private slots:
  void ProxySlot();
  void ClassUpdated( QString ClassName );
signals:
  void RebuildModel();
};

}  // namespace dbse

#endif // SCHEMAMETHODIMPLEMENTATIONEDITOR_H
