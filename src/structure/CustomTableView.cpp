/// Including QT#include <QHeaderView>
#include "dbe/tableselection.hpp"

#include "dbe/confaccessor.hpp"
#include <QAbstractItemView>
#include <QScrollBar>
#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>
#include <QContextMenuEvent>
#include <QDialog>
#include <QLineEdit>
#include <QVariant>
#include <QHeaderView>

/// Including DBE
#include "dbe/CustomTableView.hpp"
#include "dbe/ObjectEditor.hpp"
#include "dbe/ObjectCreator.hpp"
#include "dbe/messenger.hpp"

#include "dbe/MainWindow.hpp"

//-----------------------------------------------------------------------------------------------------
dbe::CustomTableView::CustomTableView ( QWidget * parent )
  : QTableView ( parent ),
    m_context_menu ( nullptr ),
    FindObject ( nullptr ),
    editObject ( nullptr ),
    deleteObjectAc ( nullptr ),
    refByAc ( nullptr ),
    refByAcOnlyComp ( nullptr ),
    copyObjectAc ( nullptr ),
    m_find_object_dialog ( nullptr ),
    LineEdit ( nullptr ),
    NextButton ( nullptr ),
    GoButton ( nullptr ),
    ListIndex ( 0 )
{
  verticalHeader()->setVisible(true);
  verticalHeader()->setSectionResizeMode ( QHeaderView::Interactive );

  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  horizontalHeader()->setMaximumSectionSize(750);
  setSortingEnabled ( true );
  setAlternatingRowColors ( true );
  setSelectionMode ( SelectionMode::SingleSelection );
  setSelectionBehavior ( QAbstractItemView::SelectionBehavior::SelectRows );
  setHorizontalScrollMode(ScrollMode::ScrollPerPixel);
  setVerticalScrollMode(ScrollMode::ScrollPerPixel);
  setWordWrap(true);
  setTextElideMode(Qt::ElideRight);

  connect ( this, SIGNAL ( activated(const QModelIndex&) ), this, SLOT ( slot_edit_object() ) );

}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::contextMenuEvent ( QContextMenuEvent * Event )
{
  if ( m_context_menu == nullptr )
  {
    m_context_menu = new QMenu ( this );
    CreateActions();
  }

  QModelIndex Index = indexAt ( Event->pos() );

  if ( Index.isValid() ) {
    for (int item=0; item<m_last_object_item; ++item) {
      m_context_menu->actions().at ( item )->setVisible ( true );
    }
  }
  else {
    for (int item=0; item<m_last_object_item; ++item) {
      m_context_menu->actions().at ( item )->setVisible ( false );
    }
  }
  m_context_menu->exec ( Event->globalPos() );
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::FindObjectSlot()
{
  if ( m_find_object_dialog != nullptr )
  {
    delete m_find_object_dialog;
    m_find_object_dialog = nullptr;
  }

  m_find_object_dialog = new QDialog ( this );
  m_find_object_dialog->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
  m_find_object_dialog->setToolTip ( "Type string to edit line and press Enter." );
  m_find_object_dialog->setWindowTitle ( "Search for an object in the table" );

  QHBoxLayout * Layout = new QHBoxLayout ( m_find_object_dialog );
  QLabel * Label = new QLabel ( QString ( "Find Object:" ), m_find_object_dialog );

  NextButton = new QPushButton ( "Next" );
  GoButton = new QPushButton ( "Go !" );

  LineEdit = new QLineEdit ( m_find_object_dialog );
  LineEdit->setToolTip ( "Type string and press Enter" );

  Layout->addWidget ( Label );
  Layout->addWidget ( LineEdit );
  Layout->addWidget ( GoButton );
  Layout->addWidget ( NextButton );

  m_find_object_dialog->setLayout ( Layout );
  m_find_object_dialog->show();
  NextButton->setDisabled ( true );

  connect ( LineEdit, SIGNAL ( textEdited ( QString ) ), this,
            SLOT ( EditedSearchString ( QString ) ) );
  connect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( slot_go_to_object() ) );
  connect ( GoButton, SIGNAL ( clicked() ), this, SLOT ( slot_go_to_object() ) );
  connect ( NextButton, SIGNAL ( clicked() ), this, SLOT ( GoToNext() ) );
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::slot_go_to_object()
{
  ListIndex = 0;
  ListOfMatch.clear();

  QString UserType = LineEdit->text();

  if ( UserType.isEmpty() )
  {
    return;
  }

  QAbstractItemModel * Model = model();

  if ( Model != nullptr )
  {
    QVariant StringCriterium = QVariant ( UserType );
    QModelIndex WhereToStartSearch = Model->index ( 0, 0 );
    ListOfMatch = Model->match ( WhereToStartSearch, Qt::DisplayRole, StringCriterium, 1000,
                                 Qt::MatchContains | Qt::MatchWrap );

    if ( ListOfMatch.size() > 0 )
    {
      ListIndex = 0;
      auto val=ListOfMatch.value ( ListIndex );
      selectRow ( val.row() );
//      resizeColumnToContents ( val.row() );
      scrollTo (val , QAbstractItemView::PositionAtCenter );

      GoButton->setDisabled ( true );
      NextButton->setEnabled ( true );

      disconnect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( slot_go_to_object() ) );
      connect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( GoToNext() ) );
    }
  }
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::GoToNext()
{
  if ( ( LineEdit->text() ).isEmpty() )
  {
    ListIndex = 0;
    ListOfMatch.clear();
    return;
  }

  if ( ListOfMatch.size() > 0 )
  {
    if ( ( ++ListIndex ) < ListOfMatch.size() )
    {
      scrollTo ( ListOfMatch.value ( ListIndex ), QAbstractItemView::EnsureVisible );
      selectRow ( ListOfMatch.value ( ListIndex ).row() );
      resizeColumnToContents ( ListIndex );
    }
    else
    {
      ListIndex = 0;
      scrollTo ( ListOfMatch.value ( ListIndex ), QAbstractItemView::EnsureVisible );
      selectRow ( ListOfMatch.value ( ListIndex ).row() );
      resizeColumnToContents ( ListIndex );
    }
  }
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::EditedSearchString ( QString Text )
{
  Q_UNUSED ( Text )

  connect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( slot_go_to_object() ) );
  disconnect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( GoToNext() ) );

  GoButton->setEnabled ( true );
  NextButton->setDisabled ( true );
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::slot_edit_object()
{
  QModelIndex const & index = this->currentIndex();

  if ( index.isValid() )
  {
    if ( models::tableselection const * selectionmodel =
           dynamic_cast<models::tableselection const *> ( index.model() ) )
    {
      if ( models::table const * srcmodel =
             dynamic_cast<const dbe::models::table *> ( selectionmodel->sourceModel() ) )
      {
        tref obj = srcmodel->GetTableObject ( selectionmodel->mapToSource ( index ).row() );

        if ( not obj.is_null() )
        {
          emit OpenEditor ( obj );
        }
      }
    }
  }
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::referencedBy_OnlyComposite()
{
  QModelIndex const & Index = this->currentIndex();

  const dbe::models::tableselection * Model =
    dynamic_cast<const dbe::models::tableselection *> ( Index.model() );

  const dbe::models::table * SourceModel = dynamic_cast<const dbe::models::table *> ( Model
                                                                                      ->sourceModel() );

  if ( !Index.isValid() || !Model )
  {
    return;
  }

  tref obj = SourceModel->GetTableObject ( Model->mapToSource ( Index ).row() );
  referencedBy ( obj, true );
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::referencedBy_All()
{
  QModelIndex const & Index = this->currentIndex();
  const dbe::models::tableselection * Model =
    dynamic_cast<const dbe::models::tableselection *> ( Index.model() );

  dbe::models::table const * SourceModel = dynamic_cast<const dbe::models::table *> ( Model
                                                                                      ->sourceModel() );

  if ( !Index.isValid() || !Model )
  {
    return;
  }

  tref obj = SourceModel->GetTableObject ( Model->mapToSource ( Index ).row() );
  referencedBy ( obj, false );
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::slot_copy_object()
{
  QModelIndex const & Index = this->currentIndex();

  dbe::models::tableselection const * Model =
    dynamic_cast<const dbe::models::tableselection *> ( Index.model() );

  dbe::models::table const * SourceModel = dynamic_cast<const dbe::models::table *> ( Model
                                                                                      ->sourceModel() );

  if ( Index.isValid() and Model )
  {
    tref obj = SourceModel->GetTableObject ( Model->mapToSource ( Index ).row() );

    if ( not obj.is_null() )
    {
      ( new ObjectCreator ( obj ) )->show();
    }
  }
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::slot_create_object()
{
  // TODO implement dbe::CustomTableView::slot_create_object
  throw std::string ( " dbe::CustomTableView::slot_create_object is not yet implemented " );
}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::slot_delete_objects()
{
  if ( this->model() != nullptr )
  {
    QModelIndexList qindices = this->selectedIndexes();
    std::vector<QModelIndex> indices;

    for ( QModelIndex q : qindices )
    {
      if ( q.isValid() )
      {
        indices.push_back ( q );
      }
    }

    if ( dbe::models::tableselection * casted =
           dynamic_cast<dbe::models::tableselection *> ( this->model() ) )
    {
      casted->delete_objects ( indices.begin(), indices.end() );
    }
    else if ( dbe::models::table * casted = dynamic_cast<dbe::models::table *>
                                            ( this->model() ) )
    {
      casted->delete_objects ( indices.begin(), indices.end() );
    }
    else
    {
      WARN ( "Object Deleting",
             "Object deletion failed due to the internal model being in invalid state",
             "You are advised to restart the application before proceeding any further" );
    }
  }

}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::CreateActions()
{
  editObject = new QAction ( tr ( "&Edit Object" ), this );
  connect ( editObject, SIGNAL ( triggered() ), this, SLOT ( slot_edit_object() ) );
  m_context_menu->addAction ( editObject );

  deleteObjectAc = new QAction ( tr ( "&Delete Object" ), this );
  connect ( deleteObjectAc, SIGNAL ( triggered() ), this, SLOT ( slot_delete_objects() ) );
  m_context_menu->addAction ( deleteObjectAc );

  refByAc = new QAction ( tr ( "Referenced B&y (All objects)" ), this );
  refByAc->setToolTip ( "Find all objects which reference the selected object" );
  refByAc->setStatusTip ( refByAc->toolTip() );
  connect ( refByAc, SIGNAL ( triggered() ), this, SLOT ( referencedBy_All() ) );
  m_context_menu->addAction ( refByAc );

  refByAcOnlyComp = new QAction ( tr ( "Referenced B&y (Only Composite)" ), this );
  refByAcOnlyComp->setToolTip (
    "Find objects (ONLY Composite ones) which reference the selected object" );
  refByAcOnlyComp->setStatusTip ( refByAcOnlyComp->toolTip() );
  connect ( refByAcOnlyComp, SIGNAL ( triggered() ), this,
            SLOT ( referencedBy_OnlyComposite() ) );
  m_context_menu->addAction ( refByAcOnlyComp );

  copyObjectAc = new QAction ( tr ( "Copy This Object Into A &New One" ), this );
  connect ( copyObjectAc, SIGNAL ( triggered() ), this, SLOT ( slot_copy_object() ) );
  m_context_menu->addAction ( copyObjectAc );

  m_context_menu->addSeparator();
  m_last_object_item = m_context_menu->actions().size();

  FindObject = new QAction ( tr ( "Find &Object" ), this );

  connect ( FindObject, SIGNAL ( triggered() ), this, SLOT ( FindObjectSlot() ) );
  m_context_menu->addAction ( FindObject );

}
//-----------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
void dbe::CustomTableView::referencedBy ( tref obj, bool onlyComposite )
{
  if ( not obj.is_null() )
  {
    MainWindow::findthis()->get_view()->referencedBy ( onlyComposite, obj );
  }
}
//-----------------------------------------------------------------------------------------------------

