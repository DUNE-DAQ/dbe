/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

#include "dbe/tableselection.hpp"

#include "dbe/messenger.hpp"

dbe::models::tableselection::tableselection ( QObject * parent )
  : QSortFilterProxyModel ( parent ),
    Type ( ExactFilter )
{
  model_common_connections();
}

void dbe::models::tableselection::SetFilterType (
  dbe::models::tableselection::FilterType Filter )
{
  Type = Filter;
}

dbe::TableNode * dbe::models::tableselection::getnode ( const QModelIndex & index ) const
{
  if ( index.isValid() )
  {
    QModelIndex sourceParent = mapToSource ( index );
    dbe::models::table * my = dynamic_cast<dbe::models::table *> ( sourceModel() );

    if ( my != 0 )
    {
      return my->getnode ( sourceParent );
    }
    else
    {
      return nullptr;
    }
  }
  else
  {
    return nullptr;
  }
}

bool dbe::models::tableselection::setData ( const QModelIndex & index,
                                            const QVariant & value,
                                            int role )
{
  if ( index.isValid() )
  {
    QModelIndex sourceParent;

    int idx_row = index.row();
    int idx_col = index.column();

    try
    {
      QModelIndex idx_index = this->index ( idx_row, idx_col );
      sourceParent = mapToSource ( idx_index );
    }
    catch ( std::exception const & stderr )
    {
      WARN ( "Error setting data", stderr.what() );
      return true;
    }

    bool success = sourceModel()->setData ( sourceParent, value, role );
    QModelIndex idx_index = this->index ( idx_row, idx_col );
    emit dataChanged ( idx_index, idx_index );

    return success;
  }
  else
  {
    return true;
  }
}

void dbe::models::tableselection::ResetModel()
{
  beginResetModel();
  endResetModel();
}

bool dbe::models::tableselection::filterAcceptsRow ( int sourceRow,
                                                     const QModelIndex & sourceParent ) const
{
  QModelIndex index0 = sourceModel()->index ( sourceRow, 0, sourceParent );

  if ( filterRegExp().isEmpty() )
  {
    return true;
  }

  switch ( Type )
  {
  case dbe::models::tableselection::RegExpFilter:
    if ( filterRegExp().indexIn ( sourceModel()->data ( index0 ).toString() ) != -1 )
    {
      return true;
    }
    else
    {
      return false;
    }

  case dbe::models::tableselection::ExactFilter:
    return ( filterRegExp().exactMatch ( sourceModel()->data ( index0 ).toString() ) );

  default:
    return true;
  }
}

bool dbe::models::tableselection::lessThan ( const QModelIndex & left,
                                             const QModelIndex & right ) const
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
  {
    const QString& l = LeftData.toString();
    const QString& r = RightData.toString();

    bool ok;
    const double ld = l.toDouble(&ok);
    if(ok == true) {
        return ( ld < r.toDouble());
    }

    const qint64 lll = l.toLongLong(&ok);
    if(ok == true) {
        return (lll < r.toLongLong());
    }

    const quint64 lull = l.toULongLong(&ok);
    if(ok == true) {
        return (lull < r.toULongLong());
    }

    return ((l).compare(r) > 0);
  }

  default:
    return false;
  }

  return true;
}

dbe::tref dbe::models::tableselection::getobject ( QModelIndex const & index ) const
{
  if ( index.isValid() )
  {
    if ( models::table * src_model = dynamic_cast<dbe::models::table *>
                                     ( this->sourceModel() ) )
    {
      QModelIndex const src_index = this->mapToSource ( index );

      if ( src_index.isValid() )
      {
        return src_model->GetTableObject ( src_index.row() );
      }
    }
  }

  throw daq::dbe::cannot_handle_invalid_qmodelindex ( ERS_HERE );
}

dunedaq::conffwk::class_t dbe::models::tableselection::getclass ( QModelIndex const & index )
const
{
  if ( index.isValid() )
  {
    if ( dbe::models::table * src_model =
           dynamic_cast<dbe::models::table *> ( this->sourceModel() ) )
    {
      QModelIndex const src_index = this->mapToSource ( index );

      if ( src_index.isValid() )
      {
        return src_model->getclass ( src_index );
      }
    }
  }

  return dunedaq::conffwk::class_t();
}

QAbstractItemModel * dbe::models::tableselection::ReturnSourceModel() const
{
  return nullptr;
}

//-----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_CREATE_THAT_OBJ_IMPL ( dbe::models::tableselection )
{
  Q_UNUSED(index);
  Q_UNUSED(obj);
}

MODEL_COMMON_INTERFACE_DELETE_THAT_OBJ_IMPL ( dbe::models::tableselection )
{
  Q_UNUSED(index);
}

MODEL_COMMON_INTERFACE_RENAME_THAT_OBJ_IMPL ( dbe::models::tableselection )
{
  Q_UNUSED(index);
  Q_UNUSED(obj);
}

MODEL_COMMON_INTERFACE_UPDATE_THAT_OBJ_IMPL ( dbe::models::tableselection )
{
  Q_UNUSED(index);
  Q_UNUSED(obj);
}

//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
MODEL_COMMON_INTERFACE_LOOKUP_IMPL ( dbe::models::tableselection )
{
  Q_UNUSED(obj);
  return QModelIndex();
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
MODEL_REMOVE_ROWS_DEF ( dbe::models::tableselection )
{
  Q_UNUSED(row);
  Q_UNUSED(count);
  Q_UNUSED(parent);
  return true;
}

MODEL_COMMON_INTERFACE_SLOTS_DEF ( dbe::models::tableselection )
//------------------------------------------------------------------------------------------
