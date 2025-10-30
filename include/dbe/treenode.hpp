#ifndef NODE_H
#define NODE_H

#include "dbe/config_reference.hpp"

#include"conffwk/ConfigObject.hpp"
#include"conffwk/Configuration.hpp"
#include"conffwk/Schema.hpp"
#include "dbe/datahandler.hpp"

namespace dbe
{

class treenode
{
public:
  treenode ( treenode * ParentNode = nullptr );
  treenode ( QString const & Datum, treenode * ParentNode = nullptr );
  treenode ( QStringList const & DataList, treenode * ParentNode = nullptr );

  virtual ~treenode();

  virtual QVariant GetData ( const int Column, int role = Qt::DisplayRole ) const;

  virtual tref GetObject() const;

  int GetRow() const;

  virtual void AddChild ( treenode * Child );

  virtual void RemoveChild ( treenode * Child );

  treenode * GetChild ( const int Row ) const;

  QList<treenode *> GetChildren() const;

  treenode * GetParent() const;

  int ChildCount() const;
  int ColumnCount() const;

  void SetHasStructure ( bool Structure );
  bool GetHasStructure() const;
  void SetWasFetched ( bool Fetched );
  bool GetWasFetched() const;

  void rename ( QString const & );

protected:
  treenode * Parent;
  QList<treenode *> Children;
  QList<QVariant> Data;
  bool HasStructure;
  bool WasFetched;
};

class ClassNode: public treenode
{
public:
  ClassNode ( const dunedaq::conffwk::class_t & Info, treenode * ParentNode );
  ~ClassNode();
  [[nodiscard]] virtual QVariant GetData (
     int Column, int role = Qt::DisplayRole ) const;
  [[nodiscard]] dunedaq::conffwk::class_t GetClassInfo() const;
  [[nodiscard]] const QVariant& get_tooltip() const {return m_tooltip;};
  void AddChild ( treenode * Child ) override;
  void RemoveChild ( treenode * Child ) override;

protected:
  void updateData(bool addition);

private:
  dunedaq::conffwk::class_t ClassInfo;
  unsigned int numObjects;
  QVariant m_tooltip;
};

class ObjectNode: public treenode
{
public:
  ObjectNode ( dref ObjectData, bool IsCopy, treenode * ParentNode );
  ~ObjectNode();
  virtual QVariant GetData ( const int Column, int role = Qt::DisplayRole ) const;
  tref GetObject() const;
private:
  dref configdata;
};

class AttributeNode: public treenode
{
public:
  AttributeNode ( const dunedaq::conffwk::attribute_t & AttributeData, treenode * ParentNode );
  ~AttributeNode();
  virtual QVariant GetData ( const int Column, int role = Qt::DisplayRole ) const;
  dunedaq::conffwk::attribute_t attribute_t() const;
private:
  dunedaq::conffwk::attribute_t attribute_t_definition;
};

class RelationshipNode: public treenode
{
public:
  RelationshipNode ( const dunedaq::conffwk::relationship_t & RelationshipData,
                     treenode * ParentNode );
  ~RelationshipNode();
  virtual QVariant GetData ( const int Column, int role = Qt::DisplayRole ) const;
  dunedaq::conffwk::relationship_t relation_t() const;
private:
  dunedaq::conffwk::relationship_t relation_t_definition;
};

}  // namespace dbe
#endif // NODE_H
