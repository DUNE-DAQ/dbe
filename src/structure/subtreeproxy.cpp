/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/// Including DBE
#include "dbe/config_ui_info.hpp"
#include "dbe/subtreeproxy.hpp"
#include "dbe/tree.hpp"
#include "dbe/treenode.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/messenger.hpp"

dbe::models::subtree_proxy::subtree_proxy ( const QString & Name,
                                            const QStringList & Default,
                                            QObject * parent )
  : QSortFilterProxyModel ( parent ),
    this_gui_windowname ( Name ),
    this_default_filter ( Default ),
    this_apply_default_filter ( false )
{
  check_view_type();
  model_common_connections();
}

dbe::models::subtree_proxy::~subtree_proxy()
{
}

void dbe::models::subtree_proxy::ResetModel()
{
  beginResetModel();
  endResetModel();
}

bool dbe::models::subtree_proxy::filterAcceptsRow ( int source_row,
                                                    const QModelIndex & source_parent ) const
{
  if ( this_apply_default_filter )
  {
    return ApplyDefaultFilter ( source_row, source_parent );
  }
  else
  {
    return ApplyUserFilter ( source_row, source_parent );
  }
}

bool dbe::models::subtree_proxy::lessThan ( const QModelIndex & left,
                                            const QModelIndex & right ) const
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

void dbe::models::subtree_proxy::check_view_type()
{
  this_window_config = confaccessor::guiconfig()->window (
                         this_gui_windowname.toStdString() );

  if ( this_window_config.Title.isEmpty() )
  {
    this_apply_default_filter = true;
  }
}

void dbe::models::subtree_proxy::LoadClasses()
{
  if ( this_apply_default_filter == false )
  {
    for ( QString const & i : this_window_config.GraphicalClassesList )
    {
      GraphicalClass Class = confaccessor::guiconfig()->graphical ( i.toStdString() );
      QStringList ClassList = Class.DerivedClasses;
      ClassList.append ( Class.DatabaseClassName );

      for ( QString & ClassName : ClassList )
      {
        treenode * ClassNode = confaccessor::gethandler()->getnode ( ClassName );

        if ( ClassNode )
        {
          dbe::models::tree * UnderlyingModel =
            static_cast<dbe::models::tree *> ( sourceModel() );
          QModelIndex ParentIndex = UnderlyingModel->getindex ( ClassNode );

          if ( sourceModel()->canFetchMore ( ParentIndex ) ) sourceModel()->fetchMore (
              ParentIndex );
        }
      }
    }
  }
}

dbe::tref dbe::models::subtree_proxy::getobject ( const QModelIndex & index ) const
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

dbe::treenode * dbe::models::subtree_proxy::getnode ( const QModelIndex & index ) const
{
  QModelIndex sourceParent = mapToSource ( index );
  dbe::models::tree * my = dynamic_cast<dbe::models::tree *> ( sourceModel() );

  if ( my )
  {
    return my->getnode ( sourceParent );
  }

  return nullptr;
}

dunedaq::conffwk::class_t dbe::models::subtree_proxy::getclass ( const QModelIndex & index )
const
{
  QModelIndex sourceParent = mapToSource ( index );
  dbe::models::tree * my = dynamic_cast<dbe::models::tree *> ( sourceModel() );

  if ( my )
  {
    return my->getclass ( sourceParent );
  }

  return dunedaq::conffwk::class_t();
}

QAbstractItemModel * dbe::models::subtree_proxy::ReturnSourceModel() const
{
  return sourceModel();
}

bool dbe::models::subtree_proxy::ApplyDefaultFilter ( int source_row,
                                                      const QModelIndex & source_parent ) const
{
  Q_UNUSED ( source_row )
  Q_UNUSED ( source_parent )

  return true;
}

bool dbe::models::subtree_proxy::ApplyUserFilter ( int source_row,
                                                   const QModelIndex & source_parent ) const
{
  dbe::models::tree * srcmodel = static_cast<tree *> ( sourceModel() );

  QModelIndex const & nodeindex = srcmodel->index ( source_row, 0, source_parent );
  treenode * tofilter = srcmodel->getnode ( nodeindex );
  QString const nodename = srcmodel->data ( nodeindex ).toString();

  if ( not source_parent.isValid() )
  {
    /// Process classes

    for ( QString const & admissible : this_window_config.GraphicalClassesList )
    {
      GraphicalClass candidate = confaccessor::guiconfig()->graphical (
                                   admissible.toStdString() );

      if ( candidate.DatabaseClassName == nodename
           or candidate.DerivedClasses.contains ( nodename ) )
      {
        return true;
      }
    }
  }
  else if ( dynamic_cast<ObjectNode *> ( tofilter ) )
  {
    return true;
  }
  else if ( dynamic_cast<RelationshipNode *> ( tofilter ) )
  {
    /// Process objects
    QString const related = sourceModel()->data ( nodeindex.parent().parent() ).toString();

    for ( QString const & admissible : this_window_config.GraphicalClassesList )
    {
      GraphicalClass candidate = confaccessor::guiconfig()->graphical (
                                   admissible.toStdString() );

      if ( candidate.DatabaseClassName == related or candidate.DerivedClasses.contains (
             related ) )
      {
        return candidate.ShowAllRelationships or candidate.Relationships.contains ( nodename );
      }

    }
  }

  return false;
}
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_LOOKUP_IMPL ( dbe::models::subtree_proxy )
{
  if ( treenode * classnode = confaccessor::gethandler()->getnode (
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

MODEL_COMMON_INTERFACE_CREATE_THAT_OBJ_IMPL ( dbe::models::subtree_proxy )
{
// TODO implement create object from signal for Subtreeproxymodel
  Q_UNUSED(index);
  Q_UNUSED(obj);
}

MODEL_COMMON_INTERFACE_DELETE_THAT_OBJ_IMPL ( dbe::models::subtree_proxy )
{
  if ( index.isValid() )
  {
    this->removeRows ( index.row(), 1, index.parent() );
  }
}

MODEL_COMMON_INTERFACE_RENAME_THAT_OBJ_IMPL ( dbe::models::subtree_proxy )
{
  //TODO implement rename object
  Q_UNUSED(index);
  Q_UNUSED(obj);
}

MODEL_COMMON_INTERFACE_UPDATE_THAT_OBJ_IMPL ( dbe::models::subtree_proxy )
{
// TODO implement update object from signal for Subtreeproxymodel
  Q_UNUSED(index);
  Q_UNUSED(obj);
}

TREEMODEL_REMOVE_ROWS_DEF ( dbe::models::subtree_proxy )
MODEL_COMMON_INTERFACE_SLOTS_DEF ( dbe::models::subtree_proxy )
//----------------------------------------------------------------------------------------------------

