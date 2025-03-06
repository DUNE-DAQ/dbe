#ifndef TABLENODE_H
#define TABLENODE_H

/// Including DBE
#include <QStringList>
#include <QVariant>
/// Including config headers
#include "conffwk/Schema.hpp"

namespace dbe
{

class TableNode
{
public:
  virtual ~TableNode();
  explicit TableNode ( QStringList const & NodeData, const QVariant& tooltip );
  [[nodiscard]] virtual QStringList GetData() const;
  [[nodiscard]] const QVariant& get_tooltip() const {return m_tooltip;};
  void resetdata ( QStringList const & );
protected:
  QStringList Data;
  QVariant m_tooltip;
};

class TableAttributeNode: public TableNode
{
public:
  TableAttributeNode ( dunedaq::conffwk::attribute_t Attribute, const QStringList & NodeData );
  ~TableAttributeNode();
  [[nodiscard]] QStringList GetData() const override;
  [[nodiscard]] dunedaq::conffwk::attribute_t GetAttribute() const;
private:
  dunedaq::conffwk::attribute_t AttributeData;
};

class TableRelationshipNode: public TableNode
{
public:
  TableRelationshipNode ( dunedaq::conffwk::relationship_t Relationship,
                          const QStringList & NodeData );
  ~TableRelationshipNode();
  [[nodiscard]] QStringList GetData() const override;
  [[nodiscard]] dunedaq::conffwk::relationship_t GetRelationship() const;
private:
  dunedaq::conffwk::relationship_t RelationshipData;
};

}  // namespace dbe
#endif // TABLENODE_H
