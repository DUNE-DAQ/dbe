// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

#include "dbe/confaccessor.hpp"
#include "dbe/tableselection.hpp"
#include "dbe/CustomDelegate.hpp"
#include "dbe/BuildingBlockEditors.hpp"
#include "dbe/TableNode.hpp"
#include "dbe/Validator.hpp"
//#include "dbe/MainWindow.hpp"

#include <QKeyEvent>
#include <QCompleter>

dbe::CustomDelegate::CustomDelegate ( QObject * parent )
  : QItemDelegate ( parent )
{
  setClipping ( true );
}

QWidget * dbe::CustomDelegate::createEditor ( QWidget * parent,
                                              const QStyleOptionViewItem & option,
                                              const QModelIndex & index ) const
{
  Q_UNUSED ( option );
  const dbe::models::tableselection * Model =
    dynamic_cast<const dbe::models::tableselection *> ( index.model() );

  const dbe::models::table * SourceModel = dynamic_cast<const dbe::models::table *> ( Model
                                                                                      ->sourceModel() );

  TableNode * Item = Model->getnode ( index );

  tref Object = SourceModel->GetTableObject ( Model->mapToSource ( index ).row() );

  if ( Model == nullptr )
  {
    return nullptr;
  }

  if ( index.column() == 0 )
  {
    emit CreateObjectEditorSignal ( Object );
    return nullptr;
  }

  if ( dynamic_cast<TableRelationshipNode *> ( Item ) )
  {
    TableRelationshipNode * RelationshipItem = dynamic_cast<TableRelationshipNode *> ( Item );
    dunedaq::conffwk::relationship_t RelationshipData = RelationshipItem->GetRelationship();
    widgets::editors::relation * Editor = new widgets::editors::relation ( RelationshipData );
    connect ( Editor, SIGNAL ( signal_edit_end() ), this, SLOT ( CommitAndClose() ) );
    connect ( Editor, SIGNAL ( signal_force_close() ), this, SLOT ( Close() ) );
    return Editor;
  }
  else if ( dynamic_cast<TableAttributeNode *> ( Item ) )
  {
    TableAttributeNode * AttributeItem = dynamic_cast<TableAttributeNode *> ( Item );
    dunedaq::conffwk::attribute_t AttributeData = AttributeItem->GetAttribute();

    if ( AttributeData.p_is_multi_value )
    {
        widgets::editors::multiattr * widget = new widgets::editors::multiattr(AttributeData);
        connect ( widget, SIGNAL ( signal_edit_end() ), this, SLOT ( CommitAndClose() ) );
        connect ( widget, SIGNAL ( signal_force_close() ), this, SLOT ( Close() ) );
        return widget;
    }
    else
    {
      switch ( AttributeData.p_type )
      {

      case dunedaq::conffwk::date_type:

      case dunedaq::conffwk::time_type:

      case dunedaq::conffwk::string_type:
      {
        widgets::editors::stringattr * Editor = new widgets::editors::stringattr ( AttributeData,
                                                                                   parent );
        connect ( Editor, SIGNAL ( signal_edit_end() ), this, SLOT ( CommitAndClose() ) );
        connect ( Editor, SIGNAL ( signal_force_close() ), this, SLOT ( Close() ) );
        return Editor;
      }

      case dunedaq::conffwk::double_type:

      case dunedaq::conffwk::float_type:

      case dunedaq::conffwk::s8_type:

      case dunedaq::conffwk::s16_type:

      case dunedaq::conffwk::s32_type:

      case dunedaq::conffwk::s64_type:

      case dunedaq::conffwk::u8_type:

      case dunedaq::conffwk::u16_type:

      case dunedaq::conffwk::u32_type:

      case dunedaq::conffwk::u64_type:
      {
        widgets::editors::numericattr * Editor = new widgets::editors::numericattr ( AttributeData,
                                                                                     parent );
        connect ( Editor, SIGNAL ( signal_edit_end() ), this, SLOT ( CommitAndClose() ) );
        connect ( Editor, SIGNAL ( signal_force_close() ), this, SLOT ( Close() ) );
        return Editor;
      }

      case dunedaq::conffwk::bool_type:

      case dunedaq::conffwk::enum_type:
      {
        widgets::editors::combo * Editor = new widgets::editors::combo ( AttributeData, parent );
        connect ( Editor, SIGNAL ( signal_value_change() ), this, SLOT ( CommitAndClose() ) );
        connect ( Editor, SIGNAL ( signal_force_close() ), this, SLOT ( Close() ) );
        return Editor;
      }

      case dunedaq::conffwk::class_type:
      {
        widgets::editors::combo * Editor = new widgets::editors::combo ( AttributeData, parent );
        connect ( Editor, SIGNAL ( signal_edit_end() ), this, SLOT ( CommitAndClose() ) );
        connect ( Editor, SIGNAL ( signal_force_close() ), this, SLOT ( Close() ) );
        return Editor;
      }

      default:
        break;
      }
    }
  }

  return nullptr;
}

void dbe::CustomDelegate::setEditorData ( QWidget * editor,
                                          const QModelIndex & index ) const
{
  const dbe::models::tableselection * mdl =
    dynamic_cast<const dbe::models::tableselection *> ( index.model() );

  if ( mdl != nullptr || editor != nullptr )
  {
    if ( widgets::editors::base * w = dynamic_cast<widgets::editors::base *> ( editor ) )
    {
      QStringList const & data = mdl->getnode ( index )->GetData();
      w->setdata ( data );
      w->SetEditor();
    }
  }
}

void dbe::CustomDelegate::setModelData ( QWidget * editor, QAbstractItemModel * model,
                                         const QModelIndex & index ) const
{
  const dbe::models::tableselection * Model =
    dynamic_cast<const dbe::models::tableselection *> ( index.model() );

  if ( Model == nullptr || editor == nullptr )
  {
    return;
  }

  widgets::editors::base * WidgetInterface = dynamic_cast<widgets::editors::base *>
                                             ( editor );

  QStringList ValueList = WidgetInterface->getdata();

  model->setData ( index, ValueList, Qt::EditRole );
}

void dbe::CustomDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option,
                                  const QModelIndex & index ) const
{
    QItemDelegate::paint(painter, option, index);

    // TODO: now commented-out because of small issues with text rendering
    //       indeed the text is too close to the cell's borders and some times artifacts are present
    //       the alternate colors have been anyway enabled in the table view
    //       so basically we miss the back-ground for the first column

    //  painter->save();
    //
    //  QFont fontmodif = option.font;
    //
    //  if ( option.state & QStyle::State_Selected )
    //  {
    //    painter->setBackground ( option.palette.mid() );
    //    painter->setRenderHint ( QPainter::Antialiasing, true );
    //  }
    //  else if ( index.column() == 0 )
    //  {
    //    painter->setBackground ( QColor::fromRgb ( 75, 222, 148 ) );
    //    fontmodif.setBold ( true );
    //  }
    //  else if ( index.row() % 2 == 1 )
    //  {
    //    painter->setBackground ( Qt::lightGray );
    //  }
    //  else
    //  {
    //    painter->setBackground ( Qt::white );
    //  }
    //
    //  painter->setFont ( fontmodif );
    //  painter->fillRect ( option.rect, painter->background() );
    //  painter->drawText ( option.rect, option.displayAlignment, index.data().toString() );
    //
    //  painter->restore();
}

void dbe::CustomDelegate::updateEditorGeometry ( QWidget * editor,
                                                 const QStyleOptionViewItem & option,
                                                 const QModelIndex & index ) const
{
  Q_UNUSED ( index );
  editor->setGeometry ( option.rect );
}

bool dbe::CustomDelegate::eventFilter ( QObject * editor, QEvent * event )
{
  if ( event->type() == QEvent::KeyPress )
  {
    QKeyEvent * KeyEvent = static_cast<QKeyEvent *> ( event );

    if ( KeyEvent->key() == Qt::Key_Enter || KeyEvent->key() == Qt::Key_Return )
    {
      if ( dynamic_cast<widgets::editors::combo *> ( editor ) != 0 )
      {
        return QItemDelegate::eventFilter (
                 editor, event );
      }
      else
      {
        return true;
      }
    }
  }
  else if ( event->type() == QEvent::FocusOut )
  {
    return true;
  }

  return QItemDelegate::eventFilter ( editor, event );
}

bool dbe::CustomDelegate::editorEvent ( QEvent * event, QAbstractItemModel * model,
                                        const QStyleOptionViewItem & option,
                                        const QModelIndex & index )
{
  if ( ! ( event->type() == QEvent::MouseButtonPress
           || event->type() == QEvent::MouseButtonRelease ) )
  {
    if ( dynamic_cast<QKeyEvent *> ( event ) )
    {
      QKeyEvent * KeyEvent = static_cast<QKeyEvent *> ( event );

      if ( KeyEvent->text() != "e" )
      {
        return true;
      }
    }
  }

  return QItemDelegate::editorEvent ( event, model, option, index );
}

void dbe::CustomDelegate::CommitAndClose()
{
  widgets::editors::base * Editor = qobject_cast<widgets::editors::base *> ( sender() );

  if ( Editor )
  {
    emit commitData ( Editor ); /// This trigger : this->setModelData
    emit closeEditor ( Editor );
    return;
  }
}

void dbe::CustomDelegate::Close()
{
  widgets::editors::base * Editor = qobject_cast<widgets::editors::base *> ( sender() );

  if ( Editor )
  {
    emit closeEditor ( Editor );
    return;
  }
}
