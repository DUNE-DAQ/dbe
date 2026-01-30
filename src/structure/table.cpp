/// Including QT Headers
#include "dbe/table.hpp"
#include "dbe/treenode.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/StyleUtility.hpp"
#include "dbe/Conversion.hpp"
#include "dbe/messenger.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/dbcontroller.hpp"
#include "dbe/config_api_set.hpp"

#include <QFont>
#include <QBrush>
#include <QMimeData>

#include <bitset>

dbe::models::table::table ( QObject * parent )
  : QAbstractTableModel ( parent ),
    enabled ( false )
{
  model_common_connections();
}

dbe::models::table::~table()
{}

int dbe::models::table::rowCount ( const QModelIndex & parent ) const
{
  if ( !parent.isValid() )
  {
    return this_structure.size();
  }

  return 0;
}

int dbe::models::table::columnCount ( const QModelIndex & parent ) const
{
  if( !parent.isValid() ) {
      return this_headers.size();
  }

  return 0;
}

QVariant dbe::models::table::data ( const QModelIndex & index, int role ) const
{

  if ( index.isValid() )
  {
    TableNode * TableItem = getnode ( index );

    if ( role == Qt::DisplayRole )
    {
      QString Data;

      for ( QString const & i : TableItem->GetData() )
      {
        static QString space
        { ", " };
        Data.append(i).append(space);
      }
      Data.remove(Data.length() - 2, 2);

      return QVariant ( Data );
    }

    if ( role == Qt::ToolTipRole )
    {
      return TableItem->get_tooltip();
    }
    if ( role == Qt::FontRole )
    {
      if ( dynamic_cast<TableAttributeNode *> ( TableItem ) )
        return QFont ( "Helvetica", 10, -1,
                       false );
      else if ( dynamic_cast<TableRelationshipNode *> ( TableItem ) )
        return QFont ( "Courier", 10,
                       QFont::Bold );
      else
      {
        return QFont ( "SansSerif", 10, QFont::Bold );
      }
    }

    if ( role == Qt::ForegroundRole )
    {
      if ( dynamic_cast<TableAttributeNode *> ( TableItem ) )
        return QBrush (
                 StyleUtility::TableColorAttribute );
      else if ( dynamic_cast<TableRelationshipNode *> ( TableItem ) )
        return QBrush (
                 StyleUtility::TableColorRelationship );
      else
      {
        return QVariant();
      }
    }

    if ( role == Qt::BackgroundRole )
    {
      auto attr_node = dynamic_cast<TableAttributeNode *> ( TableItem );
      if ( attr_node != nullptr ) {
        auto val = attr_node->GetData();
        if (val.size() == 1 &&
            val[0].toStdString() == attr_node->GetAttribute().p_default_value) {
          return QBrush (StyleUtility::DefaultValueBackground );
        }
        return QBrush (StyleUtility::TableAttributeBackground );
      }
      else if ( dynamic_cast<TableRelationshipNode *> ( TableItem ) ) {
        return QBrush (
                 StyleUtility::TableRelationshipBackground );
      }
    }
  }
  return QVariant();
}

bool dbe::models::table::setData ( const QModelIndex & index, const QVariant & value,
                                   int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }

  TableNode * TableItem = getnode ( index );

  QStringList OldDataList = TableItem->GetData();

  QStringList NewDataList = value.toStringList();

  if ( NewDataList == OldDataList )
  {
    return false;
  }

  dref obj_desc = this_objects[index.row()];

  tref Object = dbe::inner::dbcontroller::get (
  { obj_desc.UID(), obj_desc.class_name() } );

  if ( dynamic_cast<TableRelationshipNode *> ( TableItem ) )
  {
    TableRelationshipNode * RelationshipNode =
      dynamic_cast<TableRelationshipNode *> ( TableItem );
    dunedaq::conffwk::relationship_t RelationshipData = RelationshipNode->GetRelationship();
    dbe::config::api::set::relation ( Object, RelationshipData, NewDataList );
  }
  else if ( dynamic_cast<TableAttributeNode *> ( TableItem ) )
  {
    TableAttributeNode * AttributeNode = dynamic_cast<TableAttributeNode *> ( TableItem );
    dunedaq::conffwk::attribute_t AttributeData = AttributeNode->GetAttribute();
    dbe::config::api::set::attribute ( Object, AttributeData, NewDataList );
  }

  return true;
}

QVariant dbe::models::table::headerData ( int section, Qt::Orientation orientation,
                                          int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Horizontal )
    {
      return this_headers.at ( section );
    }
    if ( orientation == Qt::Vertical )
    {
      return section + 1;
    }
  }

  if ( role == Qt::FontRole )
  {
    return QFont ( "Helvetica [Cronyx]", 10 );
  }

  return QVariant();
}

Qt::ItemFlags dbe::models::table::flags ( const QModelIndex & index ) const
{
  if(index.isValid()) {
      dref obj_desc = this_objects[index.row()];
      tref Object = dbe::inner::dbcontroller::get ( { obj_desc.UID(), obj_desc.class_name() } );

      if ( confaccessor::check_file_rw ( QString::fromStdString ( Object.contained_in() ) ) )
      {
          return ( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable |
                   Qt::ItemIsDropEnabled );
      }
  }

  return ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
}

Qt::DropActions dbe::models::table::supportedDropActions() const
{
  return Qt::CopyAction;
}

QStringList dbe::models::table::mimeTypes() const
{
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

bool dbe::models::table::dropMimeData ( const QMimeData * data, Qt::DropAction action,
                                        int row,
                                        int column, const QModelIndex & parent )
{
  Q_UNUSED ( row )
  Q_UNUSED ( column )
  Q_UNUSED ( parent )

  bool Accept = true;

  if ( action == Qt::IgnoreAction )
  {
    return true;
  }

  if ( !data->hasFormat ( "application/vnd.text.list" ) )
  {
    return false;
  }

  /// Here if parent is valid it indicates that the drop ocurred on an item otherwise it ocurred on a top level item
  QByteArray encodedData = data->data ( "application/vnd.text.list" );

  QDataStream stream ( &encodedData, QIODevice::ReadOnly );

  QList<QStringList> newItems;

  while ( !stream.atEnd() )
  {
    QStringList text;
    stream >> text;
    newItems << text;
  }

  for ( int i = 0; i < newItems.size(); ++i )
  {
    if ( newItems.at ( 0 ).at ( 1 ) != newItems.at ( i ).at ( 1 ) )
    {
      Accept = false;
    }
  }

  if ( Accept )
  {
    BuildTableFromObject ( newItems );
    emit ResetTab();
  }

  return Accept;
}

bool dbe::models::table::BuildTableFromClass ( const QString & cname, bool include_derived )
{
  reset ( cname );

  cptr<dbe::datahandler> dbaccess_guard = confaccessor::gethandler();

  dunedaq::conffwk::class_t classinfo = dbe::config::api::info::onclass::definition (
                                     cname.toStdString(),
                                     false );

  setheader ( classinfo );

  if ( treenode * NodeClass = dbaccess_guard->getnode ( cname ) )
  {
    std::vector<treenode *> classnodes
    { NodeClass };

    if ( include_derived )
    {

      for ( std::string const & sbcname : classinfo.p_subclasses )
      {
        if ( treenode * sbcnode = dbaccess_guard->getnode ( sbcname ) )
        {
          classnodes.push_back ( sbcnode );
        }
      }
    }

    /// Looping over classes
    /// Only in the case of derived classes this will be more than one

    for ( treenode * clelement : classnodes )
    {
      /// All objects from that class

      for ( treenode * child : clelement->GetChildren() )
      {
        this_structure.append ( createrow ( child ) );
      }
    }

    enabled = true;
    return true;
  }
  else
  {
    return false;
  }
}

QList<dbe::models::table::type_datum *> dbe::models::table::createrow (
  treenode const * rownode )
{

  dref obj = rownode->GetObject();

  this_objects.append ( obj );

  dunedaq::conffwk::class_t const & cdef = dbe::config::api::info::onclass::definition (
                                        obj.class_name(),
                                        false );
  std::vector<dunedaq::conffwk::attribute_t> const & attributes = cdef.p_attributes;
  std::vector<dunedaq::conffwk::relationship_t> const & relations = cdef.p_relationships;

  assert ( attributes.size() + relations.size() < 1025 );
  std::bitset<1024> hindex; // maximum number of columns to display

  {
    int column = 0;

    for ( dunedaq::conffwk::attribute_t const & attr : attributes )
    {
      hindex.set ( column++, this_headers.contains ( QString::fromStdString ( attr.p_name ) ) );
    }

    for ( dunedaq::conffwk::relationship_t const & rel : relations )
    {
      hindex.set ( column++, this_headers.contains ( QString::fromStdString ( rel.p_name ) ) );
    }
  }

  // Create the row for this object
  QList<TableNode *> Row;
  Row.append ( new TableNode (
                 QStringList {rownode->GetData ( 0 ).toString()},
                 QVariant(QString::fromStdString(cdef.p_description))));
  {
    // Loop over object values and add them to the row
    // Values are represent as nodes ( attributes or relations ) and these contain
    // the structured data associated either with a attribute / multi-attribute or a relation
    std::size_t column = 0;

    for ( treenode * valuenode : rownode->GetChildren() )
    {
      QStringList values;
      // Every valuenode has its attributes and relations defined as its childs

      if ( hindex[column++] )
      {

        for ( treenode * nodevalues : valuenode->GetChildren() )
        {
          // Here we need to filter such that only values corresponding to the header are kept
          // Add only the first of values in a list of values
          values.append ( nodevalues->GetData ( 0 ).toString() );
        }

        if ( AttributeNode * NodeAttribute = dynamic_cast<AttributeNode *> ( valuenode ) )
        {
          Row.append ( new TableAttributeNode ( NodeAttribute->attribute_t(), values ) );
        }
        else if ( RelationshipNode * NodeRelationship =
                    dynamic_cast<RelationshipNode *> ( valuenode ) )
        {
          Row.append ( new TableRelationshipNode ( NodeRelationship->relation_t(), values ) );
        }
      }
    }
  }

  return Row;
}

void dbe::models::table::reset ( QString const & cname )
{

  for ( auto & List : this_structure )
  {
    qDeleteAll ( List );
  }

  this_objects.clear();
  this_structure.clear();
  this_headers.clear();
  this_class_name = cname;
}

void dbe::models::table::setheader ( dunedaq::conffwk::class_t const & cinfo )
{
  if ( !this_headers.contains ( "Object Name" ) )
  {
    this_headers.append ( "Object Name" );
  }

  for ( auto & i : cinfo.p_attributes )
  {
    if ( !this_headers.contains ( QString::fromStdString ( i.p_name ) ) )
    {
      this_headers.append ( QString::fromStdString ( i.p_name ) );
    }
  }

  for ( auto & i : cinfo.p_relationships )
  {
    if ( !this_headers.contains ( QString::fromStdString ( i.p_name ) ) )
    {
      this_headers.append ( QString::fromStdString ( i.p_name ) );
    }
  }

}

bool dbe::models::table::BuildTableFromObject ( QList<QStringList> BuildList )
{
  reset ( BuildList.at ( 0 ).at ( 1 ) );

  treenode * classnode = confaccessor::gethandler()->getnode ( this_class_name );

  dunedaq::conffwk::class_t classinfo = dbe::config::api::info::onclass::definition (
                                     this_class_name.toStdString(),
                                     false );

  setheader ( classinfo );

  confaccessor::gethandler()->FetchMore ( classnode );

  for ( const QStringList & i : BuildList )
  {
    QString name = i.at ( 0 );
    treenode * node = confaccessor::gethandler()->getnode ( this_class_name, name );
    this_structure.append ( createrow ( node ) );
  }

  enabled = false;
  return true;
}

dbe::tref dbe::models::table::GetTableObject ( int ObjectIndex ) const
{
  return this_objects.at ( ObjectIndex ).ref();
}

bool dbe::models::table::is_built() const
{
  return enabled;
}

QString dbe::models::table::get_class_name() const
{
  return this_class_name;
}

QAbstractItemModel * dbe::models::table::ReturnSourceModel() const
{
  return nullptr;
}

QList<dbe::dref> * dbe::models::table::GetTableObjects()
{
  return &this_objects;
}

dbe::TableNode * dbe::models::table::getnode ( const QModelIndex & Index ) const
{
  if ( Index.isValid() )
  {
    return this_structure.at ( Index.row() ).at ( Index.column() );
  }

  return nullptr;
}

void dbe::models::table::ResetModel()
{
  beginResetModel();
  endResetModel();
}

void dbe::models::table::slot_data_dropped ( QMimeData const & data, Qt::DropAction action )
{
  QModelIndex dum;
  this->dropMimeData ( &data, action, 0, 0, dum );
}

dbe::tref dbe::models::table::getobject ( QModelIndex const & index ) const
{
  if ( index.isValid() )
  {
    return this_objects[index.row()].ref();
  }

  throw daq::dbe::cannot_handle_invalid_qmodelindex ( ERS_HERE );
}

dunedaq::conffwk::class_t dbe::models::table::getclass ( QModelIndex const & index ) const
{
  if ( index.isValid() )
  {
    return class_type_info;
  }

  return dunedaq::conffwk::class_t();
}

void dbe::models::table::objectsUpdated(const std::vector<dbe::dref>& objects) {
    update_multiple_objects(objects);
}

//-----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_CREATE_THAT_OBJ_IMPL ( dbe::models::table )
{
  Q_UNUSED(index);
  if ( treenode * handlerclass = confaccessor::gethandler()->getnode ( obj.class_name() ) )
  {
    if ( obj.class_name() == this_class_name.toStdString()
         or config::api::info::onclass::derived (
           this_class_name.toStdString(), obj.class_name() ) )
    {

      dbe::treenode const * handlernode = dbe::datahandler::findchild (
                                            handlerclass, QString::fromStdString ( obj.UID() ) );

      if ( handlernode == nullptr )
      {
        handlernode = new ObjectNode ( obj, false, handlerclass );
      }

      emit layoutAboutToBeChanged();

      // We assume that the objects are sorted and we want to insert a new element

      tref const handlerobj = handlernode->GetObject();
      auto sit = this_structure.begin();
      auto it = this_objects.begin();

      for ( ;
            it != this_objects.end() and sit != this_structure.end()
            and it->UID() < handlerobj.UID(); ++it, ++sit )
      {}

      this_structure.insert ( ++sit, createrow ( handlernode ) );

      this_objects.insert ( ++it, handlerobj );

      // Normally we would have to call changePersistentIndex.
      // Because an objectnode is created which had no index
      // before this creation had occured there is no need to call it.
      emit layoutChanged();
    }
  }
}

MODEL_COMMON_INTERFACE_DELETE_THAT_OBJ_IMPL ( dbe::models::table )
{
  if ( index.isValid() )
  {
    try
    {
      this->removeRows ( index.row(), 1, index.parent() );
    }
    catch ( daq::dbe::ObjectChangeWasNotSuccessful const & err )
    {
      WARN ( "Object cannot be deleted", dbe::config::errors::parse ( err ).c_str() );
    }
  }
}

MODEL_COMMON_INTERFACE_RENAME_THAT_OBJ_IMPL ( dbe::models::table )
{
  if ( index.isValid() )
  {
    type_datum * element = getnode ( index );
    element->resetdata ( QStringList ( QString::fromStdString ( obj.ref().UID() ) ) );
    this_objects[index.row()] = obj.ref();
    emit dataChanged ( index, index );
  }
}

MODEL_COMMON_INTERFACE_UPDATE_THAT_OBJ_IMPL ( dbe::models::table )
{
  if ( treenode * handlerclass = confaccessor::gethandler()->getnode ( obj.class_name() ) )
  {
    if ( obj.class_name() == this_class_name.toStdString()
         or config::api::info::onclass::derived (
           this_class_name.toStdString(), obj.class_name() ) )
    {
      dbe::treenode const * handlernode = dbe::datahandler::findchild (
                                            handlerclass, QString::fromStdString ( obj.UID() ) );

      tref handlerobj = handlernode->GetObject();

      auto sit = this_structure.begin();
      auto it = this_objects.begin();

      // find the object to be updated by looping through both objects and nodes

      for ( ;
            it != this_objects.end() and sit != this_structure.end()
            and it->UID() != handlerobj.UID(); ++it, ++sit )

        ;

      // delete all table nodes in the list ( remove elements of the row )
      for ( TableNode * x : *sit )
      {
        delete x;
      }

      // Recreate the row
      *sit = createrow ( handlernode );

      int row = index.row() == 0 ? 0 : index.row() - 1;

      int column = index.column() == 0 ? 0 : index.column() - 1;

      emit dataChanged ( createIndex ( row, column ), createIndex ( row + 1, column + 1 ) );
    }
  }
}

//----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_SLOTS_DEF ( dbe::models::table )
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_LOOKUP_IMPL ( dbe::models::table )
{
  for ( int row = 0; row < this_objects.size(); ++row )
  {
    dref ListElement = this_objects.at ( row );

    if ( ListElement.UID() == obj.UID() )
    {
      return this->index ( row, 0 );
    }
  }

  return QModelIndex();
}

//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
MODEL_REMOVE_ROWS_DEF ( dbe::models::table )
{
  beginRemoveRows ( parent, row, row + count - 1 );

  for ( ; count != 0; --count )
  {
    this_structure.removeOne ( this_structure.at ( row + count - 1 ) );
    this_objects.removeAt ( row + count - 1 );
  }

  endRemoveRows();
  return true;
}

//-----------------------------------------------------------------------------------------------------
