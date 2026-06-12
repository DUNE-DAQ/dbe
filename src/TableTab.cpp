/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

/// Including Qt
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMimeData>
#include <qevent.h>

/// Including dbe
#include "dbe/TableTab.hpp"
#include "dbe/MainWindow.hpp"

//------------------------------------------------------------------------------------------------------
/**
 * Construct a Tabletab and let everything unitialized until first use
 *
 * Set the geometry and operational properties of the object
 *
 * @param parent a pointer to the parent widget
 */
dbe::TableTab::TableTab ( QWidget * parent )
  : QWidget ( parent ),
    this_model ( nullptr ),
    this_filter ( nullptr ),
    this_delegate ( nullptr ),
    this_view ( nullptr )
{
  this_view = new CustomTableView();
  /// Adjusting layout
  QVBoxLayout * TabLayout = new QVBoxLayout ( this );
  TabLayout->addWidget ( this_view );
  this_view->hide();
  setAcceptDrops ( true );
  this_holder = dynamic_cast<QTabWidget *> ( parent );
}

dbe::TableTab::~TableTab()
{
  delete this_model;
  delete this_filter;
  delete this_delegate;
  delete this_view;
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
void dbe::TableTab::create_models()
{

  if ( this_model == nullptr )
  {
    this_model = new dbe::models::table();
    connect ( this, SIGNAL ( sig_data_dropped ( QMimeData const &, Qt::DropAction ) ),
              this_model,
              SLOT ( slot_data_dropped ( QMimeData const &, Qt::DropAction ) ) );
  }

  this_delegate = this_delegate == nullptr ? new CustomDelegate() : this_delegate;
  this_filter = this_filter == nullptr ? new dbe::models::tableselection() : this_filter;
}

void dbe::TableTab::reset_models()
{
  this_model->ResetModel();
  this_filter->ResetModel();
}

void dbe::TableTab::CreateModels()
{
  if ( this_model == nullptr )
  {
    create_models();
  }
  else
  {
    reset_models();
  }

  connect ( this_model, SIGNAL ( ResetTab() ), this, SLOT ( ResetTabView() ) );
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
void dbe::TableTab::DisconnectView()
{
  this_view->setModel ( NULL );
}

void dbe::TableTab::ResetTableView()
{
  this_filter->setSourceModel ( this_model );

  this_view->setItemDelegate ( this_delegate );
  this_view->setModel ( this_filter );
  this_view->show();

  ResizeHeaders();
}

void dbe::TableTab::ResizeHeaders()
{
    // TODO: at the moment the resize of columns creates too large columns when several data are in a cell
    //       the resize of rows is a little bit expensive with large tables
    //       to be re-evaluated
    //
    //  if ( this_view->model() != nullptr )
    //  {
    //      this_view->resizeRowsToContents();
    //      this_view->resizeColumnsToContents();
    //  }
}

void dbe::TableTab::ResetTabView()
{
  reset_models();
  ResetTableView();
  this_holder->setTabText ( this_holder->indexOf ( this ), this_model->get_class_name() );
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
/**
 * Capture event of the pointer entering the tabletab while dragging something
 *
 * @param event is a pointer to the associated qt event object
 */
void dbe::TableTab::dragEnterEvent ( QDragEnterEvent * event )
{

  // If there is nothing created then create as needed
  CreateModels();

  // Check that all mimetypes are supported
  for ( auto const & mimetype : this_model->mimeTypes() )
  {
    if ( not event->mimeData()->hasFormat( mimetype.toStdString().c_str() ) )
    {
      event->ignore();
      return;
    }
  }

  // Support only copy actions
  if ( event->proposedAction() != Qt::DropAction::CopyAction )
  {
    return;
  }

  event->acceptProposedAction();

  // connect signals
  MainWindow * mainwin = MainWindow::findthis();
  connect ( GetTableView(), SIGNAL ( OpenEditor ( tref ) ), mainwin,
            SLOT ( slot_launch_object_editor ( tref ) ), Qt::UniqueConnection );
  connect ( GetTableDelegate(), SIGNAL ( CreateObjectEditorSignal ( tref ) ), mainwin,
            SLOT ( slot_launch_object_editor ( tref ) ), Qt::UniqueConnection );
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
/**
 * Capture a mouse release event of a dragged object
 *
 * Properly emits a signal to the associated tablemodel, i.e. this is a thread safe operation
 *
 * @param event is a pointer to the associated qt event object
 */
void dbe::TableTab::dropEvent ( QDropEvent * event )
{
  emit sig_data_dropped ( *event->mimeData(), event->proposedAction() );
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
dbe::models::table * dbe::TableTab::GetTableModel() const
{
  return this_model;
}

dbe::models::tableselection * dbe::TableTab::GetTableFilter() const
{
  return this_filter;
}

dbe::CustomDelegate * dbe::TableTab::GetTableDelegate() const
{
  return this_delegate;
}

dbe::CustomTableView * dbe::TableTab::GetTableView() const
{
  return this_view;
}
//------------------------------------------------------------------------------------------------------
