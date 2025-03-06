/// Including DBE
#include "dbe/TableNode.hpp"

dbe::TableNode::TableNode ( const QStringList & NodeData, const QVariant& tooltip )
  : Data ( NodeData ), m_tooltip(tooltip)
{
}

dbe::TableNode::~TableNode()
{
}

QStringList dbe::TableNode::GetData() const
{
  return Data;
}

dbe::TableAttributeNode::TableAttributeNode ( dunedaq::conffwk::attribute_t Attribute,
                                              const QStringList & NodeData )
  : TableNode ( NodeData,
                QVariant(QString::fromStdString(Attribute.p_description ))),
    AttributeData ( Attribute)
{
}

dbe::TableAttributeNode::~TableAttributeNode() = default;

QStringList dbe::TableAttributeNode::GetData() const
{
  return Data;
}

dunedaq::conffwk::attribute_t dbe::TableAttributeNode::GetAttribute() const
{
  return AttributeData;
}

dbe::TableRelationshipNode::TableRelationshipNode ( dunedaq::conffwk::relationship_t
                                                    Relationship,
                                                    const QStringList & NodeData )
  : TableNode ( NodeData, 
                QVariant(QString::fromStdString(Relationship.p_description ))),
    RelationshipData ( Relationship )
{
}

dbe::TableRelationshipNode::~TableRelationshipNode() = default;

QStringList dbe::TableRelationshipNode::GetData() const
{
  return Data;
}

dunedaq::conffwk::relationship_t dbe::TableRelationshipNode::GetRelationship() const
{
  return RelationshipData;
}

void dbe::TableNode::resetdata ( QStringList const & newdata )
{
  Data = newdata;
}
