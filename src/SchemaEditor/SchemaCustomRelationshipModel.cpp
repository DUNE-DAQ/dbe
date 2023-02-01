#include "SchemaCustomRelationshipModel.h"

dbse::CustomRelationshipModel::CustomRelationshipModel ( OksClass * ClassInfo,
                                                         QStringList Headers, bool Derived )
  : CustomModelInterface ( Headers ),
    SchemaClass ( ClassInfo ),
    SchemaDerived ( Derived )
{
  setupModel();
}

void dbse::CustomRelationshipModel::setupModel()
{
  Data.clear();
  const std::list<OksRelationship *> * RelationshipList;

  if ( SchemaDerived )
  {
    RelationshipList = SchemaClass->all_relationships();
  }
  else
  {
    RelationshipList = SchemaClass->direct_relationships();
  }

  if ( RelationshipList )
  {
    for ( OksRelationship * Relationship : *RelationshipList )
    {
      QStringList Row;
      Row.append ( QString::fromStdString ( Relationship->get_name() ) );
      Row.append ( QString::fromStdString ( Relationship->get_type() ) );
      Row.append (
        QString ( Relationship->card2str ( Relationship->get_low_cardinality_constraint() ) ) );
      Row.append (
        QString ( Relationship->card2str ( Relationship->get_high_cardinality_constraint() ) ) );
      Data.append ( Row );
    }
  }
}

dbse::CustomRelationshipModel::~CustomRelationshipModel() = default;
