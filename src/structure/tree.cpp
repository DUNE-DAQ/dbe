// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

#include "dbe/config_api.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/config_reference.hpp"
#include "dbe/tree.hpp"
#include "dbe/treenode.hpp"
#include "dbe/ui_constants.hpp"
#include "dbe/version.hpp"

#include <QMimeData>

char const * const dbe_lib_structure_version = dbe_compiled_version;

dbe::models::tree::tree ( const QStringList & Headers, QObject * parent )
  : QAbstractItemModel ( parent ),
    abstract_classes_selectable ( false )
{
  confaccessor::gethandler()->root = new dbe::treenode ( Headers );

  for ( std::string const & aclass : dbe::config::api::info::onclass::allnames <
        std::vector<std::string >> () )
  {
    new ClassNode ( dbe::config::api::info::onclass::definition ( aclass, false ),
                    confaccessor::gethandler()->root );
  }

  model_common_connections();
}

dbe::models::tree::~tree()
{}

dbe::models::tree::type_index dbe::models::tree::index ( int row, int column,
                                                         type_index const & parent ) const
{
  // This is standard qt implementation for a tree mode;
  dbe::treenode * up{parent.isValid() ? getnode ( parent ) : confaccessor::gethandler()->root};

  if ( dbe::treenode * down = up->GetChild ( row ) )
  {
    return createIndex ( row, column, down );
  }
  else
  {
    return QModelIndex();
  }
}

dbe::models::tree::type_index dbe::models::tree::parent ( const type_index & child ) const
{
  if ( child.isValid() )
  {
    dbe::treenode * up = getnode ( child )->GetParent();

    if ( up != confaccessor::gethandler()->root )
    {
      return createIndex ( up->GetRow(), 0, up );
    }
  }

  return QModelIndex();
}

int dbe::models::tree::rowCount ( const QModelIndex & parent ) const
{
  dbe::treenode * ParentNode;

  if ( !parent.isValid() )
  {
    ParentNode = confaccessor::gethandler()->getnode();
  }
  else
  {
    ParentNode = getnode ( parent );
  }

  return ParentNode->ChildCount();
}

int dbe::models::tree::columnCount ( const type_index & parent ) const
{
  if ( parent.isValid() )
  {
    dbe::treenode * DataNode = getnode ( parent );

    return DataNode->ColumnCount();
  }
  else
  {
    return confaccessor::gethandler()->root->ColumnCount();
  }
}

Qt::ItemFlags dbe::models::tree::flags ( type_index const & index ) const
{
  treenode * node = getnode ( index );

  if ( ClassNode * classnode = dynamic_cast<ClassNode *> ( node ) )
  {
    dunedaq::conffwk::class_t classinfo = classnode->GetClassInfo();

    if ( classinfo.p_abstract )
    {
      return
        abstract_classes_selectable ?
        ( Qt::ItemIsEnabled | Qt::ItemIsSelectable ) : ( Qt::ItemIsSelectable );
    }
    else
    {
      return ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    }
  }

  return ( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
}

QVariant dbe::models::tree::data ( type_index const & index, int role ) const
{
  if ( index.isValid() )
  {
    switch ( role )
    {
    case Qt::ForegroundRole:
      if ( ObjectNode * onode = dynamic_cast<ObjectNode *> ( getnode ( index ) ) )
      {
        try
        {
          tref const & obj = onode->GetObject();

          if ( confaccessor::check_file_rw ( QString::fromStdString ( obj.contained_in() ) ) )
          {
            return QVariant ( QColor ( Qt::blue ) );
          }
          else
          {
            return QVariant ( QColor ( Qt::darkGray ) );
          }
        }
        catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
        {
          // nothing to do the onode refers to a removed object and has not yet been removed
          return QVariant();
        }
      }

      break;

    case Qt::ToolTipRole:
      if ( auto cnode = dynamic_cast<ClassNode *> ( getnode ( index ) ) )
      {
        return cnode->GetData ( index.column(), role );
      }
      break;

    case Qt::DisplayRole:
    case Qt::DecorationRole:
      return getnode ( index )->GetData ( index.column(), role );
    }
  }

  return QVariant();
}

QVariant dbe::models::tree::headerData ( int section, Qt::Orientation orientation,
                                         int role ) const
{
  if ( ( orientation == Qt::Horizontal ) && ( role == Qt::DisplayRole ) )
  {
    return confaccessor::gethandler()->root->GetData ( section );
  }

  return QVariant();
}

bool dbe::models::tree::insertRows ( int position, int rows, const QModelIndex & parent )
{
  if ( ClassNode * onode = dynamic_cast<ClassNode *> ( getnode ( parent ) ) )
  {
    rows = onode->ChildCount();

    beginInsertRows ( parent, position, position + rows - 1 );

    std::string const & classname = onode->GetData (
                                      static_cast<int> ( tablepositions::classname ) ).toString().toStdString();

    std::vector<tref> const & objects = config::api::info::onclass::objects ( classname,
                                                                              false );

    for ( auto const & i : objects )
    {
      new ObjectNode ( i, false, onode );
      emit ObjectFile(QString::fromStdString(i.contained_in()));
    }

    endInsertRows();

    return true;
  }
  else
  {
    return false;
  }
}

bool dbe::models::tree::canFetchMore ( type_index const & parent ) const
{
  if ( parent.isValid() )
  {
    treenode * ParentNode = getnode ( parent );
    return ( !ParentNode->GetWasFetched() );
  }

  return false;
}

void dbe::models::tree::fetchMore ( type_index const & parent )
{
  if ( parent.isValid() )
  {
    treenode * ParentNode = getnode ( parent );

    if ( !ParentNode->GetWasFetched() )
    {
      insertRows ( 0, 0, parent );
      ParentNode->SetWasFetched ( true );
    }
  }
}

bool dbe::models::tree::hasChildren ( const QModelIndex & parent ) const
{
  if ( parent.isValid() )
  {
    treenode * ParentNode = getnode ( parent );
    return ( ParentNode->GetHasStructure() );
  }

  return true;
}

bool dbe::models::tree::setData ( type_index const & index, const QVariant & value,
                                  int role )
{
  Q_UNUSED ( index )
  Q_UNUSED ( value )
  Q_UNUSED ( role )

  return true;
}

QStringList dbe::models::tree::mimeTypes() const
{
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

QMimeData * dbe::models::tree::mimeData ( const QModelIndexList & indexes ) const
{
  QMimeData * mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream ( &encodedData, QIODevice::WriteOnly );

  foreach ( QModelIndex index, indexes )
  {
    if ( index.isValid() && index.column() == 0 )
    {
      QStringList Text;
      treenode * NodeObject = getnode ( index );

      tref Object = NodeObject->GetObject();
      QString ObjectUid = QString::fromStdString ( Object.UID() );
      QString ObjectClassName = QString::fromStdString ( Object.class_name() );

      Text.append ( ObjectUid );
      Text.append ( ObjectClassName );

      stream << Text;
    }
  }

  mimeData->setData ( "application/vnd.text.list", encodedData );
  return mimeData;
}

QModelIndex dbe::models::tree::getindex ( treenode * NodeItem,
                                          type_index const & RootIndex ) const
{
  for ( int i = 0; i < rowCount ( RootIndex ); ++i )
  {
    QModelIndex ChildIndex = index ( i, 0, RootIndex );
    treenode * ChildNode = getnode ( ChildIndex );

    if ( NodeItem == ChildNode )
    {
      return ChildIndex;
    }

    QModelIndex ChildChildIndex = getindex ( NodeItem, ChildIndex );

    if ( ChildChildIndex.isValid() )
    {
      return ChildChildIndex;
    }
  }

  return QModelIndex();
}

void dbe::models::tree::ResetModel()
{
  beginResetModel();
  endResetModel();
}

dbe::models::tree::type_datum * dbe::models::tree::getnode ( type_index const & index )
const
{
  if ( index.isValid() )
  {
    return static_cast<treenode *> ( index.internalPointer() );
  }

  return confaccessor::gethandler()->root;
}

dbe::tref dbe::models::tree::getobject ( const QModelIndex & index ) const
{
  if ( index.isValid() )
  {
    if ( ObjectNode * ObjectItem = dynamic_cast<ObjectNode *> ( getnode ( index ) ) )
    {
      return ObjectItem->GetObject();
    }
  }

  throw daq::dbe::cannot_handle_invalid_qmodelindex ( ERS_HERE );

}

dunedaq::conffwk::class_t dbe::models::tree::getclass ( type_index const & index ) const
{
  treenode * Item = getnode ( index );

  if ( Item != 0 )
  {
    ClassNode * ClassItem = dynamic_cast<ClassNode *> ( Item );

    if ( ClassItem != nullptr )
    {
      return ClassItem->GetClassInfo();
    }

    ObjectNode * ObjectItem = dynamic_cast<ObjectNode *> ( Item );

    if ( ObjectItem != nullptr )
    {
      tref Object = ObjectItem->GetObject();
      return dbe::config::api::info::onclass::definition ( Object.class_name(), false );
    }

    RelationshipNode * RelationshipItem = dynamic_cast<RelationshipNode *> ( Item );

    if ( RelationshipItem != nullptr )
    {
      return dbe::config::api::info::onclass::definition (
               RelationshipItem->relation_t().p_type,
               false );
    }
  }

  return dunedaq::conffwk::class_t();
}

QAbstractItemModel * dbe::models::tree::ReturnSourceModel() const
{
  return nullptr;
}

void dbe::models::tree::ToggleAbstractClassesSelectable ( bool const val )
{
  abstract_classes_selectable = val;
  ResetModel();
}

void dbe::models::tree::objectsUpdated(const std::vector<dbe::dref>& objects) {
    update_multiple_objects(objects);
}

//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_LOOKUP_IMPL ( dbe::models::tree )
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

    QModelIndex parent_index = getindex ( classnode );

    return childs == i ? QModelIndex() : index ( i, parent_index.column(), parent_index );
  }

  return QModelIndex();
}

MODEL_COMMON_INTERFACE_CREATE_THAT_OBJ_IMPL ( dbe::models::tree )
{
  Q_UNUSED(index);
  if ( treenode * classnode = confaccessor::gethandler()->getnode ( obj.class_name() ) )
  {
    if ( not dbe::datahandler::findchild ( classnode, QString::fromStdString ( obj.UID() ) ) )
    {
      layoutAboutToBeChanged();
      new ObjectNode ( obj, false, classnode );
      // Normally we would have to call changePersistentIndex.
      // Because an objectnode is created which had no index
      // before this creation had occured there is no need to call it.
      emit layoutChanged();
    }
  }
}

MODEL_COMMON_INTERFACE_DELETE_THAT_OBJ_IMPL ( dbe::models::tree )
{
  if ( index.isValid() )
  {
    this->removeRows ( index.row(), 1, index.parent() );
  }
}

MODEL_COMMON_INTERFACE_RENAME_THAT_OBJ_IMPL ( dbe::models::tree )
{
  if ( index.isValid() )
  {
    treenode * child = getnode ( index );
    child->rename ( QString::fromStdString ( obj.ref().UID() ) );
    emit dataChanged ( index, index );
  }
}

MODEL_COMMON_INTERFACE_UPDATE_THAT_OBJ_IMPL ( dbe::models::tree )
{
  // Some form of change has occured , we remove and insert a new object node
  // for the specified object
  if ( index.isValid() )
  {
    // This in the worst case return a pointer to root in the tree
    treenode * child = getnode ( index );

    if ( treenode * parent = child->GetParent() )
    {
      emit layoutAboutToBeChanged();
      parent->RemoveChild ( child );
      new ObjectNode ( obj, false, parent );
      emit layoutChanged();
    }
  }
}

//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
TREEMODEL_REMOVE_ROWS_DEF ( dbe::models::tree )
MODEL_COMMON_INTERFACE_SLOTS_DEF ( dbe::models::tree )
//----------------------------------------------------------------------------------------------------
