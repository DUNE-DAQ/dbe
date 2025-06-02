#ifndef SCHEMAMETHODEDITOR_H
#define SCHEMAMETHODEDITOR_H

#include <memory>
#include <QWidget>
#include "oks/class.hpp"
#include "oks/method.hpp"
#include "dbe/SchemaCustomMethodImplementationModel.hpp"



namespace dbse
{

namespace Ui
{
class SchemaMethodEditor;
}  // namespace Ui

class SchemaMethodEditor: public QWidget
{
  Q_OBJECT
public:
  ~SchemaMethodEditor();
  explicit SchemaMethodEditor ( dunedaq::oks::OksClass * ClassInfo, dunedaq::oks::OksMethod * Method, QWidget * parent =
                                  nullptr );
  explicit SchemaMethodEditor ( dunedaq::oks::OksClass * ClassInfo, QWidget * parent = nullptr );

  void SetController();
  void InitialSettings();
  void ParseToSave();
  void ParseToCreate();
  void BuildModels();
protected:
  void FillInfo();
private:
  std::unique_ptr<dbse::Ui::SchemaMethodEditor> ui;
  dunedaq::oks::OksClass * m_class;
  dunedaq::oks::OksMethod * m_method;
  CustomMethodImplementationModel * ImplementationModel;
  bool UsedNew;
  bool ShouldOpenMethodImplementationEditor ( QString Name );
  bool create();
private slots:
  void ProxySlot();
  void AddNewMethodImplementation();
  void OpenMethodImplementationEditor ( QModelIndex Index );
  void BuildModelImplementationSlot();
  void ClassUpdated( QString ClassName);
signals:
  void RebuildModel();
};

}  // namespace dbse
#endif // SCHEMAMETHODEDITOR_H
