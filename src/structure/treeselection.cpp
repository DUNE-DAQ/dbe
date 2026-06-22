// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/// Including DBE
#include "dbe/tree.hpp"
#include "dbe/treeselection.hpp"
#include "dbe/messenger.hpp"

dbe::models::treeselection::treeselection ( QObject * parent )
  : QSortFilterProxyModel ( parent ),
    Type ( RegExpFilterType ),
    LevelRestriction ( 1000 ),
    Hide ( false )
{
  model_common_connections();
}

dbe::models::treeselection::~treeselection()
{
}

bool dbe::models::treeselection::hasChildren ( type_index const & index ) const
{
  if ( index.isValid() )
  {
    QModelIndex sourceParent = mapToSource ( index );
    return sourceModel()->hasChildren ( sourceParent );
  }
  else
  {
    return true;
  }
}

bool dbe::models::treeselection::canFetchMore ( type_index const & index ) const
{
  if ( index.isValid() )
  {
    QModelIndex sourceParent = mapToSource ( index );
    return sourceModel()->canFetchMore ( sourceParent );
  }
  else
  {
    return false;
  }
}

void dbe::models::treeselection::fetchMore ( type_index const & index )
{
  QModelIndex sourceParent = mapToSource ( index );
  sourceModel()->fetchMore ( sourceParent );
}

void dbe::models::treeselection::SetFilterRestrictionLevel ( int Levels )
{
  LevelRestriction = Levels;
}

void dbe::models::treeselection::SetFilterType (
  dbe::models::treeselection::FilterType Filter )
{
  Type = Filter;
}

void dbe::models::treeselection::SetQueryObjects ( std::vector<tref> Objects )
{
  QueryObjects = Objects;
}

std::vector<dbe::tref> dbe::models::treeselection::GetQueryObjects()
{
  return QueryObjects;
}

dbe::treenode * dbe::models::treeselection::getnode ( type_index const & index ) const
{
  if ( index.isValid() )
  {
    dbe::models::tree * UnderlyingModel = dynamic_cast<dbe::models::tree *> ( sourceModel() );

    if ( UnderlyingModel )
    {
      return UnderlyingModel->getnode ( index );
    }
  }

  return nullptr;
}

dbe::tref dbe::models::treeselection::getobject ( type_index const & index ) const
{

  if ( index.isValid() )
  {
    QModelIndex sourceParent = mapToSource ( index );
    dbe::models::tree * my = dynamic_cast<dbe::models::tree *> ( sourceModel() );

    if ( my )
    {
      return my->getobject ( sourceParent );
    }
  }

  throw daq::dbe::cannot_handle_invalid_qmodelindex ( ERS_HERE );
}

dunedaq::conffwk::class_t dbe::models::treeselection::getclass ( type_index const & index ) const
{
  QModelIndex sourceParent = mapToSource ( index );

  if ( dbe::models::tree * my = dynamic_cast<dbe::models::tree *> ( sourceModel() ) )
  {
    return my->getclass ( sourceParent );
  }

  return dunedaq::conffwk::class_t();
}

QAbstractItemModel * dbe::models::treeselection::ReturnSourceModel() const
{
  return sourceModel();
}

void dbe::models::treeselection::ResetQueryObjects()
{
  QueryObjects.clear();
}

void dbe::models::treeselection::ResetModel()
{
  beginResetModel();
  endResetModel();
}

bool dbe::models::treeselection::filterAcceptsRow ( int source_row,
                                                    type_index const & source_parent ) const
{
  treenode * NodeObject = getnode ( sourceModel()->index ( source_row, 0, source_parent ) );

  if ( dynamic_cast<AttributeNode *> ( NodeObject ) )
  {
    return false;
  }

  if ( Hide )
  {
    if ( !source_parent.isValid() )
    {
      QModelIndex Index_1 = sourceModel()->index ( source_row, 1, source_parent );

      if ( sourceModel()->data ( Index_1 ).toUInt() == 0 )
      {
        return false;
      }
    }
  }

  switch ( Type )
  {
  case RegExpFilterType:
    return RegexpFilter ( source_row, source_parent );

  case ObjectFilterType:
    return ObjectFilter ( source_row, source_parent );

  default:
    return true;
  }
}

bool dbe::models::treeselection::lessThan ( type_index const & left,
                                            type_index const & right ) const
{
  if ( left.parent() == QModelIndex() || left.parent().parent() == QModelIndex() )
  {
    QVariant LeftData = sourceModel()->data ( left );
    QVariant RightData = sourceModel()->data ( right );

    switch ( LeftData.type() )
    {
    case QVariant::Bool:
    case QVariant::UInt:
      return ( LeftData.toUInt() < RightData.toUInt() );

    case QVariant::Int:
      return ( LeftData.toInt() < RightData.toInt() );

    case QVariant::String:
      return ( ( LeftData.toString() ).compare ( RightData.toString() ) > 0 );

    default:
      return false;
    }

    return true;
  }

  return false;
}

bool dbe::models::treeselection::AcceptItem ( type_index const & SourceIndex,
                                              int LevelRestriction ) const
{
  if ( sourceModel()->canFetchMore ( SourceIndex ) )
  {
    sourceModel()->fetchMore ( SourceIndex );
  }

  if ( sourceModel()->data ( SourceIndex ).toString().contains ( filterRegExp() ) )
  {
    return true;
  }

  if ( LevelRestriction <= 1 )
  {
    return false;
  }

  for ( int i = 0; i < sourceModel()->rowCount ( SourceIndex ); ++i )
  {
    if ( AcceptItem ( sourceModel()->index ( i, 0 ), --LevelRestriction ) )
    {
      return true;
    }
  }

  return false;
}

bool dbe::models::treeselection::RegexpFilter ( int sourceRow,
                                                type_index const & sourceParent ) const
{
  if ( AtDepth ( sourceParent ) <= LevelRestriction )
  {
    QModelIndex index0 = sourceModel()->index ( sourceRow, 0, sourceParent );
    return AcceptItem ( index0, LevelRestriction );
  }

  return true;
}

bool dbe::models::treeselection::ObjectFilter ( int sourceRow,
                                                type_index const & sourceParent ) const
{
  QModelIndex index0 = sourceModel()->index ( sourceRow, 0, sourceParent );
  QString id = sourceModel()->data ( index0 ).toString();

  if ( ( filterRegExp() ).isEmpty() )
  {
    return true;
  }

  if ( AtDepth ( sourceParent ) > 2 )
  {
    return true;
  }

  if ( !sourceParent.isValid() )
  {
    for ( size_t i = 0; i < QueryObjects.size(); ++i )
      if ( QString ( QueryObjects.at ( i ).class_name().c_str() ).compare ( id ) == 0 )
      {
        return true;
      }
  }
  else
  {
    for ( size_t i = 0; i < QueryObjects.size(); ++i )
      if ( id.compare ( QString ( QueryObjects.at ( i ).UID().c_str() ) ) == 0 )
      {
        return true;
      }
  }

  return false;
}

int dbe::models::treeselection::AtDepth ( type_index const & SourceParent ) const
{
  int Depth = 1;
  QModelIndex CurrentIndex = SourceParent;

  while ( CurrentIndex.isValid() )
  {
    CurrentIndex = CurrentIndex.parent();
    Depth++;
  }

  return Depth;
}

void dbe::models::treeselection::ToggleEmptyClasses ( bool HideLocal )
{
  Hide = HideLocal;
  ResetModel();
}
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_LOOKUP_IMPL ( dbe::models::treeselection )
{
  if ( dbe::treenode * classnode = confaccessor::gethandler()->getnode (
                                     QString::fromStdString ( obj.class_name() ) ) )
  {

    auto found = [&obj, classnode] ( int i )
    {
      return obj.UID() == classnode->GetChild ( i )->GetData ( 0 ).toString().toStdString();
    };

    int i = 0;
    int const childs = classnode->ChildCount();

    for ( ; i < childs and not found ( i ); ++i )
      ;

    return i == childs ? QModelIndex() : index ( i, 0, QModelIndex() );
  }

  return QModelIndex();
}
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_CREATE_THAT_OBJ_IMPL ( dbe::models::treeselection )
{
// This is a stub and nothing needs to be done since
// the real job happens in the slot defined in dbe::models::tree object from signal for treeselectionmodel
  Q_UNUSED(index);
  Q_UNUSED(obj);
}

MODEL_COMMON_INTERFACE_DELETE_THAT_OBJ_IMPL ( dbe::models::treeselection )
{
// This is a stub and nothing needs to be done since
// the real job happens in the slot defined in dbe::models::tree
  Q_UNUSED(index);
}

MODEL_COMMON_INTERFACE_UPDATE_THAT_OBJ_IMPL ( dbe::models::treeselection )
{
  Q_UNUSED(obj);
  type_index const mapped_index = this->mapFromSource ( index );
  emit dataChanged ( mapped_index, mapped_index );
}

MODEL_COMMON_INTERFACE_RENAME_THAT_OBJ_IMPL ( dbe::models::treeselection )
{
  Q_UNUSED(obj);
  type_index const mapped_index = this->mapFromSource ( index );
  emit dataChanged ( mapped_index, mapped_index );
}
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
TREEMODEL_REMOVE_ROWS_DEF ( dbe::models::treeselection )
MODEL_COMMON_INTERFACE_SLOTS_DEF ( dbe::models::treeselection )
//----------------------------------------------------------------------------------------------------

