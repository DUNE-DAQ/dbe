/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/// Including QT Headers
#include "dbe/confaccessor.hpp"
#include "dbe/treenode.hpp"
#include "dbe/ui_constants.hpp"
#include "dbe/config_api_get.hpp"
#include "dbe/config_api_graph.hpp"
#include "dbe/dbcontroller.hpp"
#include "dbe/StyleUtility.hpp"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>
#include <QApplication>
#include <QMessageBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

dbe::treenode::treenode ( treenode * ParentNode )
  :
  Parent ( ParentNode ),
  HasStructure ( false ),
  WasFetched ( false )
{
  if ( Parent != nullptr )
  {
    Parent->AddChild ( this );
  }
}

dbe::treenode::treenode ( const QString & Datum, treenode * ParentNode )
  :
  treenode ( ParentNode )
{
  Data.append ( Datum );
}

dbe::treenode::treenode ( const QStringList & DataList, treenode * ParentNode )
  :
  treenode ( ParentNode )
{

  for ( auto & i : DataList )
  {
    Data.append ( i );
  }
}

dbe::treenode::~treenode()
{
  qDeleteAll ( Children );
}

QVariant dbe::treenode::GetData ( const int Column, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    return Data.value ( Column );
  }
  else
  {
    return QVariant();
  }
}

void dbe::treenode::rename ( QString const & name )
{
  Data[0] = QVariant ( name );
}

dbe::tref dbe::treenode::GetObject() const
{
  throw daq::dbe::cannot_handle_invalid_qmodelindex ( ERS_HERE );
}

int dbe::treenode::GetRow() const
{
  if ( Parent )
  {
    return Parent->Children.indexOf ( const_cast<treenode *> ( this ) );
  }

  return 0;
}

void dbe::treenode::AddChild ( treenode * Child )
{
  Children.append ( Child );
}

void dbe::treenode::RemoveChild ( treenode * Child )
{
  Children.removeOne ( Child );
}

dbe::treenode * dbe::treenode::GetChild ( const int Row ) const
{
  return Children.at ( Row );
}

QList<dbe::treenode *> dbe::treenode::GetChildren() const
{
  return Children;
}

dbe::treenode * dbe::treenode::GetParent() const
{
  return Parent;
}

int dbe::treenode::ChildCount() const
{
  return Children.size();
}

int dbe::treenode::ColumnCount() const
{
  return Data.size();
}

void dbe::treenode::SetHasStructure ( bool Structure )
{
  HasStructure = Structure;
}

bool dbe::treenode::GetHasStructure() const
{
  return HasStructure;
}

void dbe::treenode::SetWasFetched ( bool Fetched )
{
  WasFetched = Fetched;
}

bool dbe::treenode::GetWasFetched() const
{
  return WasFetched;
}

dbe::ClassNode::ClassNode ( const dunedaq::conffwk::class_t & Info, treenode * ParentNode )
  :
  treenode ( ParentNode ),
  ClassInfo ( Info ),
  numObjects( 0 )
{
    Data.append ( QVariant ( QString::fromStdString ( ClassInfo.p_name ) ) );
    Data.append ( QVariant ( numObjects ) );
    m_tooltip = QVariant ( QString::fromStdString ( ClassInfo.p_description ) );
}

dbe::ClassNode::~ClassNode()
{
}

void dbe::ClassNode::updateData(bool addition) {
    addition ? ++numObjects : --numObjects;

    Data[1] =  QVariant ( numObjects );

    confaccessor::increase_total_objects ( addition == true ? 1 : -1 );

    if ( numObjects == 0 )
    {
      SetWasFetched ( true );
    }
    else
    {
      SetHasStructure ( true );
    }
}

void dbe::ClassNode::AddChild ( treenode * Child )
{
    dbe::treenode::AddChild(Child);
    updateData(true);
}

void dbe::ClassNode::RemoveChild ( treenode * Child )
{
    dbe::treenode::RemoveChild(Child);
    updateData(false);
}

QVariant dbe::ClassNode::GetData ( const int Column, int role ) const
{
  switch ( role )
  {

  case Qt::DisplayRole:
    return Data.value ( Column );

  case Qt::ToolTipRole:

    return m_tooltip;

    break;
  case Qt::DecorationRole:

    if ( Column == 0 )
    {
      return QIcon ( ":/Images/Folder.png" );
    }
  }

  return QVariant();
}

dunedaq::conffwk::class_t dbe::ClassNode::GetClassInfo() const
{
  return ClassInfo;
}

dbe::ObjectNode::ObjectNode ( dref obj, bool acopy, treenode * ParentNode )
  :
  treenode ( ParentNode ),
  configdata ( obj )
{
  Data.append ( QVariant ( QString::fromStdString ( configdata.UID() ) ) );
  if ( not acopy )
  {
    dunedaq::conffwk::class_t ClassInfo = dbe::config::api::info::onclass::definition (
                                       configdata.class_name(),
                                       false );
    std::vector<dunedaq::conffwk::attribute_t> Attributes = ClassInfo.p_attributes;
    std::vector<dunedaq::conffwk::relationship_t> Relationships = ClassInfo.p_relationships;

    if ( ( Relationships.size() > 0 ) )
    {
      SetHasStructure ( true );
    }

    for ( dunedaq::conffwk::attribute_t & Attribute : Attributes )
    {
      new AttributeNode ( Attribute, static_cast<treenode *> ( this ) );
    }

    for ( dunedaq::conffwk::relationship_t & Relationship : Relationships )
    {
      new RelationshipNode ( Relationship, static_cast<treenode *> ( this ) );
    }
  }

  SetWasFetched ( true );
}

dbe::ObjectNode::~ObjectNode() = default;

QVariant dbe::ObjectNode::GetData ( const int Column, int role ) const
{
  switch ( role )
  {

  case Qt::DisplayRole:
    return Data.value ( Column );

  case Qt::DecorationRole:

    if ( Column == 0 )
    {
      return QIcon ( ":/Images/TextGeneric.png" );
    }
    break;
  }

  return QVariant();
}

dbe::tref dbe::ObjectNode::GetObject() const
{
  return configdata.ref();
}

dbe::AttributeNode::AttributeNode ( const dunedaq::conffwk::attribute_t & AttributeData,
                                    treenode * ParentNode )
  :
  treenode ( ParentNode ),
  attribute_t_definition ( AttributeData )
{
  Data.append ( QVariant ( QString::fromStdString ( AttributeData.p_name ) ) );

  tref ObjectParent = GetParent()->GetObject();

  QStringList DataList
  { dbe::config::api::get::attribute::list<QStringList> ( ObjectParent, AttributeData ) };

  for ( QString & ObjectData : DataList )
  {
    if ( !ObjectData.isEmpty() )
    {
      treenode * ChildNode = new treenode ( ObjectData, static_cast<treenode *> ( this ) );
      ChildNode->SetWasFetched ( true );
    }
  }

  if ( DataList.size() > 0 && !DataList.at ( 0 ).isNull() )
  {
    SetHasStructure ( true );
  }

  SetWasFetched ( true );
}

dbe::AttributeNode::~AttributeNode() = default;

QVariant dbe::AttributeNode::GetData ( const int Column, int role ) const
{
  switch ( role )
  {

  case Qt::DisplayRole:
    return Data.value ( Column );

  case Qt::DecorationRole:

    if ( Column == 0 )
    {
      return QIcon ( ":/Images/SLink.png" );
    }
  }

  return QVariant();
}

dunedaq::conffwk::attribute_t dbe::AttributeNode::attribute_t() const
{
  return attribute_t_definition;
}

dbe::RelationshipNode::RelationshipNode ( const dunedaq::conffwk::relationship_t & relation,
                                          treenode * ParentNode )
  :
  treenode ( ParentNode ),
  relation_t_definition ( relation )
{
  Data.append ( QVariant ( QString::fromStdString ( relation.p_name ) ) );

  std::vector<tref> DataList;
  tref ObjectParent = GetParent()->GetObject();

  if ( ( relation_t_definition.p_cardinality == dunedaq::conffwk::only_one )
       || ( relation_t_definition
            .p_cardinality
            == dunedaq::conffwk::zero_or_one ) )
  {
    try
    {
      tref Data =
        dbe::config::api::graph::linked::through::relation<tref> (
          ObjectParent,
          relation );
      DataList.push_back ( Data );
    }
    catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
    {
      //nothing needs be done to handle the case that a relationship is not set
    }
  }
  else
  {
    DataList =
      config::api::graph::linked::through::relation<std::vector<tref>> (
        ObjectParent,
        relation );
  }

  for ( tref const & Object : DataList )
  {
    new ObjectNode ( Object, true, static_cast<treenode *> ( this ) );
  }

  if ( DataList.size() > 0 )
  {
    SetHasStructure ( true );
  }

  SetWasFetched ( true );
}

dbe::RelationshipNode::~RelationshipNode() = default;

QVariant dbe::RelationshipNode::GetData ( const int Column, int role ) const
{
  switch ( role )
  {

  case Qt::DisplayRole:
    return Data.value ( Column );

  case Qt::DecorationRole:

    if ( Column == 0 )
    {
      return QIcon ( ":/Images/SLink.png" );
    }
  }

  return QVariant();
}

dunedaq::conffwk::relationship_t dbe::RelationshipNode::relation_t() const
{
  return relation_t_definition;
}
