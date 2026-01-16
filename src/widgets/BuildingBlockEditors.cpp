#include "dbe/config_api.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/BuildingBlockEditors.hpp"
#include "dbe/Validator.hpp"
#include "dbe/StyleUtility.hpp"
#include "dbe/ObjectEditor.hpp"
#include "dbe/CustomDelegate.hpp"

#include <QStringListModel>
#include <QMessageBox>
#include <QCompleter>
#include <QLineEdit>
#include <QListWidget>
#include <QPair>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QEvent>
#include <QWidget>

#include <boost/scope_exit.hpp>

#include <functional>
#include <memory>

namespace dbe
{
editor_data_state::~editor_data_state() = default;


template<>
editor_data<dunedaq::conffwk::relationship_t>::editor_data (
  dunedaq::conffwk::relationship_t const & virtue )
  :
  editor_data_state ( true, false, true ),
  this_virtue ( virtue )
{}

template<typename T> editor_data<T>::~editor_data() = default;

namespace widgets
{

namespace editors
{
//------------------------------------------------------------------------------------------------
base::base ( std::shared_ptr<editor_data_state> editordata,
             QWidget * parent, bool owned )
  :
  QWidget ( parent ),
  p_data_editor ( editordata ),
  this_is_owned ( owned ),
  this_value_changed ( false ),
  this_initial_load ( false )
{}


template<>
std::shared_ptr<editor_data_state> base::dataeditor<editor_data_state>()
{
  return p_data_editor;
}

QStringList base::getdata()
{
  return this_data;
}

void base::setdata ( QStringList const & ValueList )
{
  this_data = ValueList;
}

void base::setdefaults ( QString const & ValueDefault )
{
  this_defaults = ValueDefault;
}

void base::setchanged ( bool const val )
{
  this_value_changed = val;
}

bool base::ischanged() const
{
  return this_value_changed;
}

void base::slot_set_initial_loaded()
{
  this_initial_load = true;
}

void base::closeEvent ( QCloseEvent * Event )
{
  Q_UNUSED ( Event )
}

//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
relation::relation ( t_virtue const & relation, QWidget * parent,
                     bool owned, bool readonly )
  :
  base (
   std::make_shared<t_build_block_editor> ( relation ), parent, owned ),
  p_base_data_editor ( std::static_pointer_cast<t_build_block_editor> ( p_data_editor ) ),
  IsMultiValue ( false ),
  m_readonly(readonly),
  StatusBar ( nullptr ),
  ContextMenu ( nullptr ),
  RemoveAction ( nullptr ),
  MoveTop ( nullptr ),
  MoveBottom ( nullptr ),
  MoveUp ( nullptr ),
  MoveDown ( nullptr ),
  EditAction ( nullptr ),
  CurrentItem ( nullptr )
{
  setupUi ( this );

  t_virtue const & Virtue = p_base_data_editor->get();

  setWindowTitle ( QString ( "Edit Relationship: %1" ).arg ( Virtue.p_name.c_str() ) );
  ListWidget->setContextMenuPolicy ( Qt::CustomContextMenu );
  ComboBox->setHidden ( true );

  // if it is X_ and many then set multivalue

  if ( ( Virtue.p_cardinality == dunedaq::conffwk::zero_or_many ) ||
       ( Virtue.p_cardinality == dunedaq::conffwk::one_or_many ) )
  {
    IsMultiValue = true;
  }

  // Check if this relation can be left unset
  if ( ( Virtue.p_cardinality == dunedaq::conffwk::one_or_many ) ||
       ( Virtue.p_cardinality == dunedaq::conffwk::only_one ) )
  {
    p_base_data_editor->set_not_null ( true );
    p_base_data_editor->set_valid ( false );
  }

  /// Setting Controller
  SetController();

  if ( this_is_owned )
  {
    SaveButton->setHidden ( true );
    RemoveButton->setHidden ( true );
  }

  buildtooltip();
}

bool relation::eventFilter ( QObject * Target, QEvent * Event )
{
    if(dynamic_cast<QListWidget*>(Target)) {
        auto decode = [this] (const QMimeData * const data) -> QPair<bool, QStringList>
        {
            bool allGood = true;
            QStringList newItems;

            if(data->hasFormat("application/vnd.text.list") == true) {
                QByteArray encodedData = data->data("application/vnd.text.list");
                QDataStream stream(&encodedData, QIODevice::ReadOnly);

                const std::string& referenceClass = p_base_data_editor->get().p_type;

                while(!stream.atEnd()) {
                    QStringList text;
                    stream >> text;

                    const QString& obj_id = text.at(0);
                    const QString& obj_class = text.at(1);

                    if((obj_class.toStdString() == referenceClass) ||
                       (dbe::config::api::info::onclass::derived(referenceClass, obj_class.toStdString()) == true))
                    {
                        newItems.append(obj_id);
                    } else {
                        allGood = false;
                    }
                }
            } else {
                allGood = false;
            }

            return qMakePair(allGood, newItems);
        };

        if(Event->type() == QEvent::DragEnter) {
           QDragEnterEvent *tDragEvent = static_cast<QDragEnterEvent *>(Event);

           const QMimeData * const data = tDragEvent->mimeData();
           const auto& decodedData = decode(data);
           if((decodedData.first == true) && (decodedData.second.size() > 0)) {
               tDragEvent->acceptProposedAction();
           }

           return true;
        }

        if(Event->type() == QEvent::Drop) {
            QDropEvent *tDropEvent = static_cast<QDropEvent *>(Event);
            if (!m_readonly) {
                const QMimeData * const data = tDropEvent->mimeData();
                for(const QString& o : decode(data).second) {
                    // TODO: this is not very efficient for large lists
                    AddToDataList(o);
                }
            }

            tDropEvent->acceptProposedAction();

            return true;
        }
    } else if(QComboBox * Box = dynamic_cast<QComboBox *>(Target)) {
        if(Event->type() == QEvent::Wheel) {
            return true;
        }

        if(Event->type() == QEvent::KeyPress) {
            QKeyEvent * KeyEvent = dynamic_cast<QKeyEvent *>(Event);

            if(KeyEvent->key() == Qt::Key_Up || KeyEvent->key() == Qt::Key_Down) {
                return true;
            } else {
                return false;
            }
        }

        if(Event->type() == QEvent::FocusOut) {
            QWidget * popup = Box->findChild<QFrame *>();
            QWidget * popup1 = Box->lineEdit()->findChild<QFrame *>();

            if(popup != popup1 and not popup->isHidden()) {
                return true;
            }

            if(FirstItem != nullptr &&
               FirstItem->listWidget() == ListWidget &&
               ListWidget->itemWidget(FirstItem) != 0) {
                ListWidget->takeItem(ListWidget->row(FirstItem));
                ListWidget->setItemWidget(FirstItem, nullptr);
                delete FirstItem;
                SetFirstItem();
            }
            else {
              std::cout << "Do we ever get here? FirstItem is not null but other conditions not met!\n";
            }
            return false;
        }
    } else if(QLineEdit* le = dynamic_cast<QLineEdit *>(Target)) {
        if(Event->type() == QEvent::MouseButtonDblClick) {
            const QString& value = le->text();

            // This string checking is very very bad...
            if((value.isEmpty() == false) && (value != c_input_placeholder) && (value != c_no_object_placeholder)) {
                CreateObjectEditor(le->text().toStdString());
                return false;
            }
        }
    }

    return false;
}

void relation::SetEditor()
{
  ListWidget->clear();

  if ( IsMultiValue )
  {
    ListWidget->addItems ( this_data );

    // Enable drops only for multi-value
    // This allows to drop several objects in a relationship (e.g., several computers in a rack)
    // Not really needed for single-value relationship (the item can be selected from the list)
    if (!m_readonly) {
      ListWidget->setAcceptDrops ( true );
      ListWidget->installEventFilter ( this );
    }
  }

  SetFirstItem();

  ListWidget->setSelectionMode ( QAbstractItemView::ExtendedSelection );
  ListWidget->setEditTriggers ( QAbstractItemView::DoubleClicked );
  ListWidget->setDragDropMode ( QAbstractItemView::InternalMove );

  CurrentDataList = this_data;

  /// Cleaning and Fetching Data
  ComboBox->clear();

  FetchData();
}

void relation::FetchData()
{
  QStringList result;
  t_virtue const & Virtue = p_base_data_editor->get();

  std::vector<tref> const & related
  {
    dbe::config::api::info::onclass::objects<true> ( Virtue.p_type )
  };

  for ( tref const & o : related )
  {
    result.append ( QString::fromStdString ( o.UID() ) );
  }

  emit FetchDataDone ( result );
}

bool relation::GetIsMultiValue() const
{
  return IsMultiValue;
}

void relation::DataWasFetched ( QStringList ListOfObjects )
{
  ComboBox->clear();
  if (!m_readonly) {
    ComboBox->addItems ( ListOfObjects );
  }
  emit signal_internal_value_change();

  if ( this_initial_load )
  {
    emit LoadedInitials();
  }
}

void relation::SetController()
{
  connect ( SaveButton, SIGNAL ( clicked() ), this, SLOT ( EndSignal() ) );
  connect ( RemoveButton, SIGNAL ( clicked() ), this, SLOT ( RemoveFromDataList() ) );
  connect ( this, SIGNAL ( signal_internal_value_change() ), this, SLOT ( UpdateActions() ) );
  connect (
    this, SIGNAL ( FetchDataDone ( QStringList ) ), this,
    SLOT ( DataWasFetched ( QStringList ) ) );
  /// Connect an action to edit objects from the list view
  connect ( ListWidget, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
            SLOT ( CustomContextMenuRequested ( QPoint ) ) );
  connect ( ListWidget, SIGNAL ( itemDoubleClicked ( QListWidgetItem * ) ), this,
            SLOT ( CreateObjectEditor ( QListWidgetItem * ) ), Qt::UniqueConnection );
  connect ( ListWidget->model(), SIGNAL ( layoutChanged() ), this, SLOT ( DummyMovement() ) );
  connect ( ListWidget, SIGNAL ( itemPressed ( QListWidgetItem * ) ), this,
            SLOT ( EditItemEntered ( QListWidgetItem * ) ) );
  connect ( &confaccessor::ref(), SIGNAL ( object_created ( QString, dref ) ), this,
              SLOT ( FetchData () ) );
  connect ( &confaccessor::ref(), SIGNAL ( object_deleted ( QString, dref ) ), this,
                SLOT ( FetchData () ) );
}

void relation::SetFirstItem()
{
  if ( IsMultiValue )
  {
    if (m_readonly) {
      if ( this_data.isEmpty() ) {
        FirstItem = new QListWidgetItem ( c_no_object_placeholder );
        ListWidget->addItem ( FirstItem );
      }
      else {
        FirstItem = new QListWidgetItem ( this_data.at ( 0 ) );
      }
    }
    else {
      FirstItem = new QListWidgetItem ( c_input_placeholder );
      FirstItem->setBackground ( Qt::GlobalColor::lightGray );
      FirstItem->setForeground ( QColor ( 0, 0, 0, 127 ) );
      ListWidget->addItem ( FirstItem );
    }
  }
  else
  {
    if ( this_data.isEmpty() )
    {
      FirstItem = new QListWidgetItem ( c_no_object_placeholder );
    }
    else
    {
      FirstItem = new QListWidgetItem ( this_data.at ( 0 ) );
    }
    ListWidget->addItem ( FirstItem );
  }


  FirstItem->setSizeHint ( FirstItem->sizeHint() + QSize ( 0, 25 ) );
}

void relation::buildtooltip()
{
  t_virtue const & Virtue = p_base_data_editor->get();
  setToolTip (
    QString ( "Relationship Name:            %1 \n" ).arg ( Virtue.p_name.c_str() ) + QString (
      "             Type:            %1 \n" ).arg ( Virtue.p_type.c_str() )
    + QString ( "             Cardinality:     %1 \n" ).arg (
      dunedaq::conffwk::relationship_t::card2str ( Virtue.p_cardinality ) )
    + QString ( "             Is Aggregation:  %1 \n" ).arg ( Virtue.p_is_aggregation )
    + QString ( "             Description:     %1 \n" ).arg ( Virtue.p_description.c_str() ) );
}
void relation::CreateObjectEditor ( const std::string& objectID )
{
    t_virtue const & virt = p_base_data_editor->get();
    tref const & obj = inner::dbcontroller::get( {objectID, virt.p_type} );

    QString oname = QString::fromStdString ( obj.full_name() );

    for ( QWidget * editor : QApplication::allWidgets() )
    {
      if ( ObjectEditor * w = dynamic_cast<ObjectEditor *> ( editor ) )
      {
        if ( ( w->objectName() ).compare ( oname ) == 0 )
        {
          w->raise();
          w->setVisible ( true );
          return;
        }
      }
    }

    ( new ObjectEditor ( obj ) )->show();
}

void relation::CreateObjectEditor ( QListWidgetItem * Item )
{
  if ( Item == FirstItem )
  {
    EditItemEntered ( FirstItem );
    return;
  }

  std::string const & ouid = Item->data ( Qt::DisplayRole ).toString().toStdString();
  CreateObjectEditor(ouid);
}

void relation::CustomContextMenuRequested ( const QPoint & pos )
{
  if ( ContextMenu == nullptr )
  {
    ContextMenu = new QMenu ( ListWidget );

    MoveTop = new QAction ( tr ( "Move Top" ), this );
    MoveTop->setShortcutContext ( Qt::WidgetShortcut );
    connect ( MoveTop, SIGNAL ( triggered() ), this, SLOT ( MoveTopSlot() ) );
    ContextMenu->addAction ( MoveTop );

    MoveBottom = new QAction ( tr ( "Move Bottom" ), this );
    MoveBottom->setShortcutContext ( Qt::WidgetShortcut );
    connect ( MoveBottom, SIGNAL ( triggered() ), this, SLOT ( MoveBottomSlot() ) );
    ContextMenu->addAction ( MoveBottom );

    MoveUp = new QAction ( tr ( "Move Up" ), this );
    MoveUp->setShortcutContext ( Qt::WidgetShortcut );
    connect ( MoveUp, SIGNAL ( triggered() ), this, SLOT ( MoveUpSlot() ) );
    ContextMenu->addAction ( MoveUp );

    MoveDown = new QAction ( tr ( "Move Down" ), this );
    MoveDown->setShortcutContext ( Qt::WidgetShortcut );
    connect ( MoveDown, SIGNAL ( triggered() ), this, SLOT ( MoveDownSlot() ) );
    ContextMenu->addAction ( MoveDown );

    ContextMenu->addSeparator();

    EditAction = new QAction ( tr ( "Edit" ), this );
    EditAction->setShortcutContext ( Qt::WidgetShortcut );
    connect ( EditAction, SIGNAL ( triggered() ), this, SLOT ( EditSlot() ) );
    ContextMenu->addAction ( EditAction );

    RemoveAction = new QAction ( tr ( "Remove" ), this );
    RemoveAction->setShortcutContext ( Qt::WidgetShortcut );
    connect ( RemoveAction, SIGNAL ( triggered() ), this, SLOT ( RemoveSlot() ) );
    ContextMenu->addAction ( RemoveAction );
  }

  if ( !m_readonly && ( CurrentItem = ListWidget->itemAt ( pos ) ) )
  {
    if ( CurrentItem != FirstItem )
    {
      ( ContextMenu->actions() ).at ( 0 )->setVisible ( true );
      ( ContextMenu->actions() ).at ( 1 )->setVisible ( true );
      ( ContextMenu->actions() ).at ( 2 )->setVisible ( true );
      ( ContextMenu->actions() ).at ( 3 )->setVisible ( true );
      ( ContextMenu->actions() ).at ( 4 )->setVisible ( true );
      ( ContextMenu->actions() ).at ( 5 )->setVisible ( false );
      ( ContextMenu->actions() ).at ( 6 )->setVisible ( true );

      ContextMenu->exec ( ListWidget->mapToGlobal ( pos ) );
    }

    else if ( CurrentItem == FirstItem and not IsMultiValue )
    {
      ( ContextMenu->actions() ).at ( 0 )->setVisible ( false );
      ( ContextMenu->actions() ).at ( 1 )->setVisible ( false );
      ( ContextMenu->actions() ).at ( 2 )->setVisible ( false );
      ( ContextMenu->actions() ).at ( 3 )->setVisible ( false );
      ( ContextMenu->actions() ).at ( 4 )->setVisible ( true );
      ( ContextMenu->actions() ).at ( 5 )->setVisible ( true );
      ( ContextMenu->actions() ).at ( 6 )->setVisible ( true );

      ContextMenu->exec ( ListWidget->mapToGlobal ( pos ) );
    }
  }
}

void relation::RemoveSlot()
{
  RemoveFromDataList();
}

void relation::MoveTopSlot()
{
  int ItemPosition = ListWidget->row ( CurrentItem );
  QString Item = this_data.at ( ItemPosition );

  this_data.removeAt ( ItemPosition );
  this_data.push_front ( Item );

  FirstItem = ListWidget->takeItem ( this_data.size() );
  ListWidget->clear();
  ListWidget->addItems ( this_data );
  ListWidget->addItem ( FirstItem );

  int index = this_data.indexOf ( Item );
  ListWidget->scrollTo ( ListWidget->model()->index ( index, 0 ) );
  ListWidget->setCurrentRow ( index );

  emit signal_internal_value_change();
}

void relation::MoveBottomSlot()
{
  int ItemPosition = ListWidget->row ( CurrentItem );
  QString Item = this_data.at ( ItemPosition );

  this_data.removeAt ( ItemPosition );
  this_data.push_back ( Item );

  FirstItem = ListWidget->takeItem ( this_data.size() );
  ListWidget->clear();
  ListWidget->addItems ( this_data );
  ListWidget->addItem ( FirstItem );

  int index = this_data.indexOf ( Item );
  ListWidget->scrollTo ( ListWidget->model()->index ( index, 0 ) );
  ListWidget->setCurrentRow ( index );

  emit signal_internal_value_change();
}

void relation::MoveUpSlot()
{
  int ItemPosition = ListWidget->row ( CurrentItem );
  QString Item = this_data.at ( ItemPosition );

  if ( ItemPosition != 0 )
  {
    this_data.swapItemsAt ( ItemPosition, ItemPosition - 1 );

    FirstItem = ListWidget->takeItem ( this_data.size() );
    ListWidget->clear();
    ListWidget->addItems ( this_data );
    ListWidget->addItem ( FirstItem );

    int index = this_data.indexOf ( Item );
    ListWidget->scrollTo ( ListWidget->model()->index ( index, 0 ) );
    ListWidget->setCurrentRow ( index );

    emit signal_internal_value_change();
  }
}

void relation::MoveDownSlot()
{
  int ItemPosition = ListWidget->row ( CurrentItem );
  QString Item = this_data.at ( ItemPosition );

  if ( ItemPosition != ( this_data.size() - 1 ) )
  {
    this_data.swapItemsAt ( ItemPosition, ItemPosition + 1 );

    FirstItem = ListWidget->takeItem ( this_data.size() );
    ListWidget->clear();
    ListWidget->addItems ( this_data );
    ListWidget->addItem ( FirstItem );

    int index = this_data.indexOf ( Item );
    ListWidget->scrollTo ( ListWidget->model()->index ( index, 0 ) );
    ListWidget->setCurrentRow ( index );

    emit signal_internal_value_change();
  }
}

void relation::EditSlot()
{
  if ( QComboBox * ComboBox = dynamic_cast<QComboBox *> ( ListWidget->itemWidget (
                                                            FirstItem ) )
     )
  {
    ComboBox->lineEdit()->setFocus();
  }

  t_virtue const & virt = p_base_data_editor->get();
  tref const & obj = inner::dbcontroller::get (
  {
    FirstItem->text().toStdString(), virt.p_type
  }

  );
  QString const & editorname = QString::fromStdString ( obj.full_name() );

  for ( QWidget * editor :
        QApplication::allWidgets()
      )
  {
    if ( ObjectEditor * w = dynamic_cast<ObjectEditor *> ( editor ) )
    {
      if ( ( w->objectName() ).compare ( editorname ) == 0 )
      {
        w->raise();
        w->setVisible ( true );
        return;
      }
    }
  }

  ( new ObjectEditor ( obj ) )->show();
}

void relation::DummyMovement()
{
  if ( this_is_owned )
  {
    emit signal_internal_value_change();
  }
}

void relation::EditItemEntered ( QListWidgetItem * Item )
{
  if ( Item != FirstItem )
  {
    return;
  }

  QStringList ListOfObjects;

  for ( int i = 0; i < ComboBox->count(); ++i )
  {
    ListOfObjects.append ( ComboBox->itemText ( i ) );
  }

  QComboBox * NewComboBox = new QComboBox ( this );

  NewComboBox->setLineEdit ( new QLineEdit() );

  NewComboBox->addItems ( ListOfObjects );

  QCompleter * Completer = new QCompleter ( ListOfObjects );
  Completer->setCaseSensitivity ( Qt::CaseInsensitive );
  Completer->setFilterMode(Qt::MatchContains);

  NewComboBox->lineEdit()->setCompleter ( Completer );

  NewComboBox->setEditable ( true );

  NewComboBox->setEditText ( Item->text() );

  if ( ListOfObjects.size() > 0 )
  {
    NewComboBox->lineEdit()->selectAll();
  }

  else
  {
    NewComboBox->setEnabled ( false );
  }

  QVariant VariantFromList ( ListOfObjects );
  ValidatorAcceptMatch * MyCustomValidator = new ValidatorAcceptMatch ( VariantFromList );
  NewComboBox->setValidator ( MyCustomValidator );
  NewComboBox->installEventFilter ( this );

  connect ( NewComboBox, SIGNAL ( activated ( const QString & ) ), this,
            SLOT ( AddToDataList ( const QString & ) ) );

  NewComboBox->lineEdit()->installEventFilter(this);

  ListWidget->setItemWidget ( FirstItem, NewComboBox );
}

void relation::EndSignal()
{
  if ( IsMultiValue and ListWidget->item ( 0 )->text() == c_input_placeholder
       and not this_value_changed )
  {
    return;
  }

  if ( not this_data.isEmpty() )
  {
    this_data.clear();
  }

  for ( int i = 0; i != ListWidget->count(); ++i )
  {
    QListWidgetItem * Item = ListWidget->item ( i );
    QString const & val = Item->text();

    if ( val != c_input_placeholder and val != c_no_object_placeholder )
    {
      this_data.append ( val );
    }
  }

  if ( not this_is_owned )
  {
    p_base_data_editor->set_valid ( true );
    emit signal_edit_end();
  }
  else
  {
    emit signal_value_change(); /// This signal is caught by Object editor
  }
}

void relation::AddToDataList ( const QString & DataValue )
{
  if ( IsMultiValue )
  {
    this_data.append ( DataValue );
  }
  else
  {
    this_data = QStringList ( DataValue );
  }

  ListWidget->clear();

  if ( IsMultiValue )
  {
    ListWidget->addItems ( this_data );
  }

  int index = this_data.indexOf ( DataValue );
  ListWidget->scrollTo ( ListWidget->model()->index ( index, 0 ) );
  ListWidget->setCurrentRow ( index );

  if ( IsMultiValue )
  {
    FirstItem = new QListWidgetItem ( c_input_placeholder );
  }
  else
  {
    FirstItem = new QListWidgetItem ( DataValue.toStdString().c_str() );
  }

  ListWidget->addItem ( FirstItem );

  if ( IsMultiValue )
  {
    FirstItem->setBackground ( Qt::GlobalColor::lightGray );
    FirstItem->setForeground ( QColor ( 0, 0, 0, 127 ) );
  }

  FirstItem->setSizeHint ( FirstItem->sizeHint() + QSize ( 0, 25 ) );

  if ( this_is_owned )
  {
    emit signal_internal_value_change();
  }
}

void relation::RemoveFromDataList()
{
  QList<QListWidgetItem *> items;

  if ( IsMultiValue )
  {
    items = ListWidget->selectedItems();
  }

  else
  {
    items.append ( FirstItem );
  }

  if ( items.size() > 0 )
  {

    for ( QListWidgetItem * item : items )
    {
      QString text = item->text();

      if ( text != c_input_placeholder )
      {
        if ( IsMultiValue )
        {
          if ( QListWidgetItem * toremove = ListWidget->takeItem ( ListWidget->row ( item ) ) )
          {
            delete toremove;
          }
        }
        else
        {

          if ( QComboBox * editbox = dynamic_cast<QComboBox *> ( ListWidget->itemWidget (
                                                                   FirstItem ) )
             )
          {
            editbox->lineEdit()->clear();
            editbox->lineEdit()->clearFocus();
            FirstItem->setText ( c_no_object_placeholder );
          }
        }

        this_data.removeOne ( text );
      }
    }

    emit signal_internal_value_change();
  }
}

void relation::UpdateActions()
{
  QStringList values;

  for ( int i = 0; i < ListWidget->count(); ++i )
  {
    QString const & value = ListWidget->item ( i )->text();

    if ( value != c_no_object_placeholder and value != c_input_placeholder )
    {
      values.append ( value );
    }
  }

  if ( p_base_data_editor->must_not_be_null() )
  {
    if ( values.size() > 0 )
    {
      p_base_data_editor->set_valid ( true );
      ListWidget->setPalette ( QApplication::palette ( this ) );
    }
    else
    {
      p_base_data_editor->set_valid ( false );
      ListWidget->setPalette ( StyleUtility::WarningStatusBarPallete );
    }
  }
  else
  {
    p_base_data_editor->set_valid ( true );
  }

  if ( values != CurrentDataList )
  {
    this_value_changed = true;
  }

  if ( this_is_owned )
  {
    EndSignal();
  }
}

void relation::closeEvent ( QCloseEvent * Event )
{
  Q_UNUSED ( Event )
  emit signal_force_close();
}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
stringattr::stringattr ( t_virtue const & attr, QWidget * parent,
                         bool owned, bool readonly )
  :
  base ( std::make_shared<t_build_block_editor> ( attr ), parent, owned ),
  m_base_data_editor ( std::static_pointer_cast<t_build_block_editor> ( p_data_editor ) ),
  m_readonly(readonly),
  DefaultValue ( "" ),
  PopUpButton ( nullptr ),
  Dialog ( nullptr ),
  OkButtonDialog ( nullptr ),
  TextEditDialog ( nullptr )
{
  setupUi ( this );
  t_virtue const & virtue = m_base_data_editor->get();

  if (m_readonly) {
      StringLineEdit->setReadOnly(true);
  }
  else {
      StringLineEdit->installEventFilter(this);
  }
  StringLineEdit->setTabChangesFocus(true);

  setWindowTitle ( QString ( "Edit Attr : %1" ).arg ( virtue.p_name.c_str() ) );
  AttributeNameLabel->setText ( QString ( virtue.p_name.c_str() ) + " : " );
  /// LAYOUT CHANGE
  AttributeNameLabel->setHidden ( true );

  UpdateActions (  );

  if ( m_base_data_editor->must_not_be_null() )
  {
    m_base_data_editor->set_valid ( true );
    StringLineEdit->setPalette ( StyleUtility::WarningStatusBarPallete );
    OkButton->setDisabled ( true );
    SetNullCheck ( true );
  }
  else
  {
    SetNullCheck ( false );
  }

  if ( virtue.p_is_multi_value )
  {
    StringLineEdit->setPalette ( QApplication::palette ( this ) );
  }

  OkButton->setHidden ( true );

  if ( this_is_owned )
  {
    connect ( StringLineEdit, SIGNAL ( StringValidated() ), this, SLOT ( AddToDataList() ),
              Qt::UniqueConnection );
    OkButton->setHidden ( true );
  }

  SetController();

  buildtooltip();
  UpdateActions (  );

  AttributeNameLabel->setFocus();
}

stringattr::~stringattr()
{
  delete Dialog;
  delete PopUpButton;
}

void stringattr::SetEditor()
{
  if ( this_data.isEmpty() )
  {
    return;
  }

  StringLineEdit->setText ( this_data.value ( 0 ) );

  slot_set_initial_loaded();
}

QTextEdit * stringattr::GetLineEdit() const
{
  return StringLineEdit;
}

void stringattr::SetNullCheck ( bool Check )
{
  StringLineEdit->SetNullCheck ( Check );
}

void stringattr::SetMultiCheck ( bool Multi )
{
  StringLineEdit->SetMultiCheck ( Multi );
}

void stringattr::SetCheckDefaults ( bool Default )
{
  StringLineEdit->SetCheckDefault ( Default );
}

void stringattr::SetFocusOnLine()
{
  StringLineEdit->selectAll();
}

bool stringattr::eventFilter(QObject * target, QEvent * evt)
{
	if (target == StringLineEdit)
	{
		if (evt->type() == QEvent::FocusIn)
		{
			ShowPopupButton();
		}
		else if (evt->type() == QEvent::KeyPress)
		{
			QKeyEvent *kevt = static_cast<QKeyEvent*>(evt);
			switch (kevt->key())
			{
				case Qt::Key_Enter:
				case Qt::Key_Return:

			if ( kevt->modifiers() == Qt::AltModifier)
			{
				kevt->setModifiers(Qt::NoModifier);
				return false;
			}else
			{
				StringLineEdit->parentWidget()->setFocus(Qt::FocusReason::TabFocusReason);
				emit signal_data_input_complete();
				return true;
			}

					break;
				default:
					break;
			}
		}
	}
  return false;
}

void stringattr::ClearText()
{
  GetLineEdit()->clear();
}

void stringattr::buildtooltip()
{
  t_virtue const & Virtue = m_base_data_editor->get();

  setToolTip (
    QString ( "Attribute Name:           %1 \n" ).arg ( Virtue.p_name.c_str() ) + QString (
      "          Type:           %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::type2str ( Virtue.p_type ) )
    + QString ( "          Range:          %1 \n" ).arg ( Virtue.p_range.c_str() )
    + QString ( "          Format:         %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::format2str ( Virtue.p_int_format ) )
    + QString ( "          Not Null:       %1 \n" ).arg ( Virtue.p_is_not_null )
    + QString ( "          Is Multi Value: %1 \n" ).arg ( Virtue.p_is_multi_value )
    + QString ( "          Default Value:  %1 \n" ).arg ( Virtue.p_default_value.c_str() )
    + QString ( "          Description:    %1 \n" ).arg ( Virtue.p_description.c_str() ) );
}

void stringattr::closeEvent ( QCloseEvent * Event )
{
  Q_UNUSED ( Event )
  emit signal_force_close();
}

void stringattr::SetController()
{
  if (!m_readonly) {
      connect ( StringLineEdit, SIGNAL ( textChanged (  ) ), this, SLOT ( UpdateActions (  ) ), Qt::UniqueConnection );
      connect ( this, SIGNAL ( signal_data_input_complete() ), this, SLOT ( AddToDataList() ) );
      connect ( OkButton, SIGNAL ( clicked() ), this, SLOT ( AddToDataList() ), Qt::UniqueConnection );
  }
}

void stringattr::setdefaults ( const QString & ValueDefault )
{
  this_defaults = ValueDefault;
  StringLineEdit->SetDefaultValue ( ValueDefault );
}

void stringattr::ShowPopupButton()
{
  if ( PopUpButton == nullptr )
  {
    PopUpButton = new QPushButton ( "..." );
    PopUpButton->setMaximumWidth (
      PopUpButton->fontMetrics().boundingRect ( "..." ).width() + 15 );
    connect ( PopUpButton, SIGNAL ( clicked() ), this, SLOT ( ShowDialog() ),
              Qt::UniqueConnection );
    horizontalLayout->addWidget ( PopUpButton );
  }

  PopUpButton->show();
}

void stringattr::HidePopupButton()
{
  if ( PopUpButton != nullptr )
  {
    PopUpButton->hide();
  }
}

void stringattr::UpdateActions ( )
{
  t_virtue const & Virtue = m_base_data_editor->get();

  if ( Virtue.p_is_not_null && ( this_data.size() == 0 ) )
  {
    OkButton->setEnabled ( true );
    m_base_data_editor->set_valid ( false );
  }

  else
  {
    OkButton->setEnabled ( true );
    m_base_data_editor->set_valid ( true );
  }
}

void stringattr::AddToDataList()
{
  QString NewValue = StringLineEdit->toPlainText();

  if ( !this_data.contains ( NewValue ) )
  {
    StringLineEdit->setToolTip ( QString ( "The set value is : %1" ).arg ( NewValue ) );
    this_data.clear();
    this_data.append ( NewValue );

    UpdateActions ( );
    this_value_changed = true;

    if ( !this_is_owned )
    {
      emit signal_value_change();
      emit signal_edit_end();
    }
    else
    {
      emit signal_value_change();
    }
  }
}

void stringattr::ShowDialog()
{

  if ( Dialog == nullptr )
  {
    QVBoxLayout * Layout = new QVBoxLayout();
    Dialog = new QDialog();
    t_virtue const & Virtue = m_base_data_editor->get();
    Dialog->setWindowTitle ( QString ( "Edit Attr : %1" ).arg ( Virtue.p_name.c_str() ) );
    TextEditDialog = new QPlainTextEdit ( Dialog );
    OkButtonDialog = new QPushButton ( "OK", Dialog );
    Layout->addWidget ( TextEditDialog );
    Layout->addWidget ( OkButtonDialog );
    Dialog->setLayout ( Layout );
    Dialog->setModal ( true );

    connect ( TextEditDialog, SIGNAL ( textChanged() ), this, SLOT ( ToogleTextEditOkButton() ),
              Qt::UniqueConnection );
    connect ( OkButtonDialog, SIGNAL ( clicked() ), this, SLOT ( UpdateFromTextEdit() ),
              Qt::UniqueConnection );
  }

  TextEditDialog->clear();
  TextEditDialog->insertPlainText ( StringLineEdit->toPlainText() );
  Dialog->show();
}

void stringattr::UpdateFromTextEdit()
{
  StringLineEdit->clear();
  StringLineEdit->setText ( TextEditDialog->toPlainText() );
  AddToDataList();
  Dialog->close();
}

void stringattr::ToogleTextEditOkButton()
{
  t_virtue const & Virtue = m_base_data_editor->get();

  if ( TextEditDialog->toPlainText().isEmpty() && Virtue.p_is_not_null )
    OkButtonDialog
    ->setEnabled (
      false );
  else
  {
    OkButtonDialog->setEnabled ( true );
  }
}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
numericattr::numericattr ( t_virtue const & attr, QWidget * parent,
                           bool owned, bool readonly )
  :
  base ( std::make_shared<t_build_block_editor> ( attr ), parent, owned ),
  this_base_data_editor ( std::static_pointer_cast<t_build_block_editor> ( p_data_editor ) ),
  m_readonly(readonly),
  this_native_base ( -1 )
{
  setupUi ( this );
  t_virtue const & Virtue = this_base_data_editor->get();

  setWindowTitle ( QString ( "Edit Attribute: %1" ).arg ( Virtue.p_name.c_str() ) );
  if (m_readonly) {
      LineEdit->setReadOnly(true);
  }
  else {
      LineEdit->SetPopupMenu();
  }
  if ( Virtue.p_is_not_null )
  {
    this_base_data_editor->set_valid ( false );
    LineEdit->setPalette ( StyleUtility::WarningStatusBarPallete );
  }
  else
  {
    LineEdit->SetNullCheck ( false );
  }

  if ( ! ( Virtue.p_type == dunedaq::conffwk::float_type
           || Virtue.p_type == dunedaq::conffwk::double_type ) )
  {
    if ( Virtue.p_int_format == dunedaq::conffwk::oct_int_format )
    {
      FormatBox->setCurrentIndex ( 2 );
      this_base = 8;
      this_native_base = 8;
    }
    else if ( Virtue.p_int_format == dunedaq::conffwk::dec_int_format )
    {
      FormatBox->setCurrentIndex ( 0 );
      this_base = 10;
      this_native_base = 10;
    }
    else if ( Virtue.p_int_format == dunedaq::conffwk::hex_int_format )
    {
      FormatBox->setCurrentIndex ( 1 );
      this_base = 16;
      this_native_base = 16;
    }

    FormatBox->hide();
  }
  else
  {
    FormatBox->hide();
  }

  SetController();

  AttributeName->setText ( QString ( Virtue.p_name.c_str() ) + " : " );
  AttributeName->setHidden ( true );
  buildtooltip();

  QString Dummy ( "Dummy" );
  UpdateActions ( Dummy );
}

void numericattr::setdefaults ( const QString & ValueDefault )
{
  this_defaults = ValueDefault;
  LineEdit->SetDefaultValue ( this_defaults );
}

QLineEdit * numericattr::GetLineEdit() const
{
  return LineEdit;
}

void numericattr::SetEditor()
{
  if ( this_data.isEmpty() )
  {
    return;
  }

  LineEdit->clear();

  /// change here
  if ( this_base == 16 )
  {
    if ( not this_data.value ( 0 ).startsWith ( "0x" ) )
    {
      LineEdit->setText ( "0x" + this_data.value ( 0 ) );
    }
    else
    {
      LineEdit->setText ( this_data.value ( 0 ) );
    }
  }
  else if ( this_base == 8 )
  {
    if ( not this_data.value ( 0 ).startsWith ( "0" ) and this_data.value ( 0 ) != 0 )
    {
      LineEdit->setText ( "0" + this_data.value ( 0 ) );
    }
    else
    {
      LineEdit->setText ( this_data.value ( 0 ) );
    }
  }
  else
  {
    LineEdit->setText ( this_data.value ( 0 ) );
  }
}

void numericattr::buildtooltip()
{
  t_virtue const & Virtue = this_base_data_editor->get();

  setToolTip (
    QString ( "Attribute Name:           %1 \n" ).arg ( Virtue.p_name.c_str() ) + QString (
      "          Type:           %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::type2str ( Virtue.p_type ) )
    + QString ( "          Range:          %1 \n" ).arg ( Virtue.p_range.c_str() )
    + QString ( "          Format:         %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::format2str ( Virtue.p_int_format ) )
    + QString ( "          Not Null:       %1 \n" ).arg ( Virtue.p_is_not_null )
    + QString ( "          Is Multi Value: %1 \n" ).arg ( Virtue.p_is_multi_value )
    + QString ( "          Default Value:  %1 \n" ).arg ( Virtue.p_default_value.c_str() )
    + QString ( "          Description:    %1 \n" ).arg ( Virtue.p_description.c_str() ) );
}

void numericattr::closeEvent ( QCloseEvent * Event )
{
  Q_UNUSED ( Event )
  emit signal_force_close();
}

void numericattr::SetController()
{
  connect ( LineEdit, SIGNAL ( DecChange() ), this, SLOT ( ChangeFormatDec() ) );
  connect ( LineEdit, SIGNAL ( OctChange() ), this, SLOT ( ChangeFormatOct() ) );
  connect ( LineEdit, SIGNAL ( HexChange() ), this, SLOT ( ChangeFormatHex() ) );

  connect ( FormatBox, SIGNAL ( currentIndexChanged ( int ) ),
            this, SLOT ( ChangeFormat ( int ) ), Qt::UniqueConnection );

  connect ( LineEdit, SIGNAL ( editingFinished() ),
            this, SLOT ( AddToList() ), Qt::UniqueConnection );
  connect ( LineEdit, SIGNAL ( textChanged ( QString ) ),
            this, SLOT ( UpdateActions ( QString ) ), Qt::UniqueConnection );
  connect ( LineEdit, SIGNAL ( returnPressed () ),
            this, SLOT ( checkIfDuplicated () ), Qt::UniqueConnection );

}

void numericattr::ShowWarning ( QString Format, QString Range )
{
  LineEdit->setPalette ( StyleUtility::AlertStatusBarPallete );

  t_virtue const & Virtue = this_base_data_editor->get();

  QString Message = QString ( "The value for attribute %1 is in the wrong format" ).arg (
                      Virtue.p_name.c_str() );

  if ( Range != "" )
  {
    QString RangeMex = QString ( " - The value should be in the range : %1" ).arg ( Range );
    Message.append ( "\n\n" + RangeMex );
  }

  if ( Format != "" )
  {
    Message.append ( QString ( "\n\n - The value should be formatted as a %1." ).arg (
                       Format ) );
  }

  QMessageBox::warning ( this, tr ( "Wrong value" ), Message );
  return;
}

bool numericattr::ValidateIntegerValue ( QString const & text )
{
  auto checkInteger = [this, text]()
  {
      t_virtue const & input = this_base_data_editor->get();

      bool convert_to_defined_base;

      qlonglong NewLongLong{0};
      qulonglong NewULongLong{0};

      auto check_convert = [this, &convert_to_defined_base]()
      {
        // If input cannot be converted to any base then show a warning and invalidate

        if ( not convert_to_defined_base )
        {
          if ( this_native_base == 10 )
          {
            ShowWarning ( "DECIMAL" );
          }
          else if ( this_native_base == 16 )
          {
            ShowWarning ( "HEX-DECIMAL" );
          }
          else if ( this_native_base == 8 )
          {
            ShowWarning ( "OCT-DECIMAL" );
          }

          return false;
        }

        return true;
      };

      if ( input.p_type == dunedaq::conffwk::u8_type
           or input.p_type == dunedaq::conffwk::u16_type
           or input.p_type == dunedaq::conffwk::u32_type
           or input.p_type == dunedaq::conffwk::u64_type )
      {
        NewULongLong = text.toULongLong ( &convert_to_defined_base, this_base );

        if ( not check_convert() )
        {
          return false;
        }

        qulonglong min_u;
        qulonglong max_u;

        if ( input.p_type == dunedaq::conffwk::u8_type )
        {
          min_u = 0;
          max_u = UCHAR_MAX;
        }
        else if ( input.p_type == dunedaq::conffwk::u16_type )
        {
          min_u = 0;
          max_u = USHRT_MAX;
        }
        else if ( input.p_type == dunedaq::conffwk::u32_type )
        {
          min_u = 0;
          max_u = ULONG_MAX;
        }
        else if ( input.p_type == dunedaq::conffwk::u64_type )
        {
          min_u = 0;
          max_u = ULONG_LONG_MAX;
        }
        else
        {
          return true;
        }

        if ( min_u <= NewULongLong and NewULongLong <= max_u )
        {
          return true;
        }

        ShowWarning ( "", QString ( "[%1 - %2]" ).arg ( min_u ).arg ( max_u ) );
      }
      else
      {
        NewLongLong = text.toLongLong ( &convert_to_defined_base, this_base );

        if ( not check_convert() )
        {
          return false;
        }

        qlonglong min_s;
        qlonglong max_s;

        if ( input.p_type == dunedaq::conffwk::s8_type )
        {
          min_s = SCHAR_MIN;
          max_s = SCHAR_MAX;
        }
        else if ( input.p_type == dunedaq::conffwk::s16_type )
        {
          min_s = SHRT_MIN;
          max_s = SHRT_MAX;
        }
        else if ( input.p_type == dunedaq::conffwk::s32_type )
        {
          min_s = LONG_MIN;
          max_s = LONG_MAX;
        }
        else if ( input.p_type == dunedaq::conffwk::s64_type )
        {
          min_s = - ( LONG_LONG_MAX ) - 1;
          max_s = LONG_LONG_MAX;
        }
        else
        {
          return true;
        }

        if ( min_s <= NewLongLong and NewLongLong <= max_s )
        {
          return true;
        }

        ShowWarning ( "", QString ( "[%1 - %2]" ).arg ( min_s ).arg ( max_s ) );
      }

      return false;
  };

  return (checkInteger() && checkRange(text));
}

bool numericattr::checkRange (QString const & Value)
{
    t_virtue const & Virtue = this_base_data_editor->get();

    QString Range = QString::fromStdString ( Virtue.p_range );

    if ( Range.isEmpty() )
    {
      return true;
    }

    QStringList ValueList = Range.split ( "," );

    QList<QPair<QString, QString>> RangeList;

    for ( QString & RangeValue : ValueList )
    {
      if ( RangeValue.contains ( ".." ) )
      {
        QStringList Ranges = RangeValue.split ( ".." );
        RangeList.append ( QPair<QString, QString> ( Ranges.at ( 0 ), Ranges.at ( 1 ) ) );
        ValueList.removeOne ( RangeValue );
      }
    }

    if ( ValueList.contains ( Value ) )
    {
      return true;
    }

    for ( QPair<QString, QString> & ValuePair : RangeList )
    {
      if ( ValuePair.first == "*" )
      {
        if ( Value.toDouble() <= ValuePair.second.toDouble() )
        {
          return true;
        }

        ShowWarning("", Range);

        return false;
      }

      else if ( ValuePair.second == "*" )
      {
        if ( Value.toDouble() >= ValuePair.first.toDouble() )
        {
          return true;
        }

        ShowWarning("", Range);

        return false;
      }

      else
      {
        if ( ( Value.toDouble() >= ValuePair.first.toDouble() ) &&
             ( Value.toDouble() <= ValuePair.second.toDouble() ) )
        {
          return true;
        }

        ShowWarning("", Range);

        return false;
      }
    }

    ShowWarning("", Range);

    return false;
}

bool numericattr::ValidateFloatValue ( QString const & Value )
{
  bool validValue = true;

  t_virtue const & Virtue = this_base_data_editor->get();

  if(Virtue.p_type == dunedaq::conffwk::float_type) {
      bool ok;
      Value.toFloat(&ok);

      if(ok == false) {
          ShowWarning("FLOAT");
          validValue = false;
      }
  } else {
      bool ok;
      Value.toDouble(&ok);

      if(ok == false) {
          ShowWarning("DOUBLE");
          validValue = false;
      }
  }

  return (validValue && checkRange(Value));
}

void numericattr::checkIfDuplicated() {
    // Emit a special signal when the Enter/Return key is pressed
    // and the data have not been changed
    // That is useful, for instance, for the multi-attribute
    // widget in order to add multiple times the same attribute
    // (you do not want to add the same value multiple times just
    // when the focus is lost)
    if ( this_data.contains(LineEdit->text()) ) {
        emit signal_value_duplicated();
    }
}

void numericattr::AddToList()
{
  // No change in data, do nothing
  // It avoids some unwanted loops when focus is acquired/lost
  if ( this_data.contains(LineEdit->text()) ) {
      return;
  }

  auto convert_to_proper_base = [this] (const QString& value) -> QString {
      if((value.isEmpty() == false) && (this_native_base != -1) && (this_native_base != this_base)) {
          QString cnvrtd;

          t_virtue const & virtue = this_base_data_editor->get();
          bool Unsigned = ( virtue.p_type == dunedaq::conffwk::u8_type
                              || virtue.p_type == dunedaq::conffwk::u16_type
                              || virtue.p_type == dunedaq::conffwk::u32_type
                              || virtue.p_type == dunedaq::conffwk::u64_type );

          if(Unsigned == true) {
              cnvrtd.setNum(LineEdit->text().toULongLong(0, this_base), this_native_base);
          } else {
              cnvrtd.setNum(LineEdit->text().toLongLong(0, this_base), this_native_base);
          }

          // Be careful to set the contextaul menu properly
          if(this_native_base == 8) {
              if(cnvrtd.startsWith("0") == false) {
                  cnvrtd.insert(0, "0");
              }
              LineEdit->EmitOctSlot();
          } else if(this_native_base == 10) {
              LineEdit->EmitDecSlot();
          } else if(this_native_base == 16) {
              if(cnvrtd.startsWith("0x", Qt::CaseInsensitive) == false) {
                  cnvrtd.insert(0, "0x");
              }
              LineEdit->EmitHexSlot();
          }

          // The current and native bases are now the same
          this_base = this_native_base;

          return cnvrtd;
      }

      return value;
  };

  LineEdit->blockSignals ( true );

  BOOST_SCOPE_EXIT(this_)
  {
      this_->LineEdit->blockSignals ( false );
  }
  BOOST_SCOPE_EXIT_END

  bool isValid = true;

  QString input = LineEdit->text();

  t_virtue const & virtue = this_base_data_editor->get();
  if ( ( virtue.p_type == dunedaq::conffwk::float_type
           or virtue.p_type == dunedaq::conffwk::double_type ))
  {
      if (not ValidateFloatValue ( input )) {
          isValid = false;
      }
  } else if ( not ValidateIntegerValue ( input ) )
  {
    isValid = false;
  }

  //    ShowWarning("FLOAT", QString::fromStdString(virtue.p_name));

  this_data.clear();

  // We better save data in the proper format, in order to avoid issues
  // with conversions (and having data saved in a bad format)
  this_data.append ( convert_to_proper_base(input) );

  this_value_changed = true;

  if(isValid == true) {
      QString Dummy ( "Dummy" );
      UpdateActions ( Dummy );
  } else {
      this_base_data_editor->set_valid ( false );
  }

  if ( not this_is_owned )
  {
    // Not owned: multiattr (connected to signal_value_change) and
    //            CustomDelegate (connected to signal_edit_end and signal_force_close)
    if(isValid == true) {
        // Do not emit the signal in this case
        // The table will try t write data and get back an exception
        emit signal_edit_end();
    }

    emit signal_value_change();
  }
  else
  {
    // Owned: ObjectEditor (connected to signal_value_change)
    emit signal_value_change();
  }
}

void numericattr::ChangeFormat ( int i )
{
  QString CurrentString = LineEdit->text();

  if ( CurrentString.isEmpty() )
  {
    //if(DecButton->isChecked())

    if ( i == 0 )
    {
      this_base = 10;
    }

    //else if(OctButton->isChecked())
    else if ( i == 2 )
    {
      this_base = 8;
    }

    //else if(HexButton->isChecked())
    else if ( i == 1 )
    {
      this_base = 16;
    }

    return;
  }

  QString ConvertedString;
  t_virtue const & Virtue = this_base_data_editor->get();

  bool Unsigned = ( Virtue.p_type == dunedaq::conffwk::u8_type
                    || Virtue.p_type == dunedaq::conffwk::u16_type || Virtue.p_type == dunedaq::conffwk::u32_type
                    || Virtue.p_type == dunedaq::conffwk::u64_type );
  bool OkConversion = false;

  //if(DecButton->isChecked())

  if ( i == 0 )
  {
    if ( Unsigned )
    {
      ConvertedString.setNum ( CurrentString.toULongLong ( &OkConversion, 0 ), 10 );
    }
    else
    {
      ConvertedString.setNum ( CurrentString.toLongLong ( &OkConversion, 0 ), 10 );
    }

    this_base = 10;
  }

  //else if(OctButton->isChecked())
  else if ( i == 2 )
  {
    if ( Unsigned )
    {
      ConvertedString.setNum ( CurrentString.toULongLong ( &OkConversion, 0 ), 8 );
    }
    else
    {
      ConvertedString.setNum ( CurrentString.toLongLong ( &OkConversion, 0 ), 8 );
    }

    ConvertedString.insert(0, "0");
    this_base = 8;
  }

  //else if(HexButton->isChecked())
  else if ( i == 1 )
  {
    if ( Unsigned )
    {
      ConvertedString.setNum ( CurrentString.toULongLong ( &OkConversion, 0 ), 16 );
    }
    else
    {
      ConvertedString.setNum ( CurrentString.toLongLong ( &OkConversion, 0 ), 16 );
    }

    ConvertedString.insert(0, "0x");
    this_base = 16;
  }

  if ( !OkConversion )
  {
    QString Format;

    if ( this_native_base == 10 )
    {
      Format = "DECIMAL";
    }
    else if ( this_native_base == 16 )
    {
      Format = "HEX-DECIMAL";
    }
    else if ( this_native_base == 8 )
    {
      Format = "OCT-DECIMAL";
    }

    ShowWarning ( Format );

    return;
  }

  if ( !ConvertedString.isEmpty() )
  {
    LineEdit->clear();
    LineEdit->setText ( ConvertedString );
  }
}

void numericattr::UpdateActions ( QString Dummy )
{
  Q_UNUSED ( Dummy )

  if ( this_base_data_editor->must_not_be_null() and ( LineEdit->text().isEmpty() ) )
  {
    this_base_data_editor->set_valid ( false );
  }
  else if ( this_base_data_editor->must_not_be_null() and ( this_data.size() == 0 ) )
  {
    this_base_data_editor->set_valid ( false );
  }
  else
  {
    this_base_data_editor->set_valid ( true );
  }
}

void numericattr::ChangeFormatDec()
{
  ChangeFormat ( 0 );
}

void numericattr::ChangeFormatHex()
{
  ChangeFormat ( 1 );
}

void numericattr::ChangeFormatOct()
{
  ChangeFormat ( 2 );
}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
combo::combo ( t_virtue const & attr, QWidget * parent,
               bool owned, bool readonly )
  :
  base ( std::make_shared<t_build_block_editor> ( attr ), parent, owned ),
  m_base_data_editor ( std::static_pointer_cast<t_build_block_editor> ( p_data_editor ) ),
  m_readonly(readonly)
{
  setupUi ( this );
  SetController();
  /// Configuration of combo box
  Combo->setFocusPolicy ( Qt::ClickFocus );
  if (m_readonly) {
      Combo->setEditable (false);
      if ( Combo->lineEdit() != nullptr ) {
          Combo->lineEdit()->setReadOnly(true);
          Combo->lineEdit()->hide();
      }
  }
  else {
      Combo->installEventFilter ( this );
      if ( Combo->lineEdit() != nullptr ) {
          Combo->lineEdit()->installEventFilter ( this );
      }

      t_virtue const & Virtue = m_base_data_editor->get();
      if ( Virtue.p_type == dunedaq::conffwk::class_type ) {
          Combo->setEditable ( true );
      }
  }
}

void combo::SetData ( QStringList const & Data )
{
  if ( Data.isEmpty() )
  {
    return;
  }

  int Index = Combo->findText ( Data.value ( 0 ) );

  if ( Index != -1 )
  {
    Combo->setCurrentIndex ( Index );
    Combo->setEditText ( Data.value ( 0 ) );
  }
}

void combo::SetValidatorData ( QStringList const & Data, bool AcceptNoMatch )
{
  Combo->clear();
  Combo->addItems ( Data );

  if (Combo->isEditable()) {
    QCompleter * Completer = new QCompleter ( Combo->model(), Combo );
    Completer->setCaseSensitivity ( Qt::CaseInsensitive );
    Completer->setCompletionMode ( QCompleter::PopupCompletion );
    Completer->setFilterMode(Qt::MatchContains);
    Combo->setCompleter ( Completer );
  }
  m_base_data_editor->set_obligatory ( false );

  QVariant VarFromList ( Data );

  if ( AcceptNoMatch )
  {
    ValidatorAcceptNoMatch * TmpValidator = new ValidatorAcceptNoMatch ( VarFromList, this );
    Combo->setValidator ( TmpValidator );
  }
  else
  {
    ValidatorAcceptMatch * TmpValidator = new ValidatorAcceptMatch ( VarFromList, this );
    Combo->setValidator ( TmpValidator );
  }
}

QStringList combo::getdata()
{
  this_data.clear();
  this_data << Combo->currentText();
  return this_data;
}

void combo::SetEditor()
{
  t_virtue const & virt = m_base_data_editor->get();

  if ( virt.p_type == dunedaq::conffwk::bool_type )
  {
    QStringList tmp
    { "true", "false" };
    SetValidatorData ( tmp );
  }
  else if ( virt.p_type == dunedaq::conffwk::enum_type )
  {
    QString rangename = virt.p_range.c_str();
    QStringList range = rangename.split ( "," );
    range.sort();
    SetValidatorData ( range );
  }
  else if ( virt.p_type == dunedaq::conffwk::class_type )
  {
    QStringList classes ( dbe::config::api::info::onclass::allnames<QStringList>() );
    classes.sort();
    SetValidatorData ( classes );
    Combo->lineEdit()->setText(c_input_placeholder);
  }

  SetData ( this_data );
}

void combo::setdata ( QStringList const & v )
{
  this_data = v;
}

bool combo::eventFilter ( QObject * Target, QEvent * Event )
{
  if (dynamic_cast<QLineEdit*>(Target) != nullptr) {
    std::cout << "cast to QLineEdit\n";
  }
  if ( Target == Combo && Combo->lineEdit() != nullptr) {
    if (Event->type() < 100) {
    }
    if (Event->type() == QEvent::FocusOut) {
      if (Combo->lineEdit()->text().isEmpty()) {
        Combo->lineEdit()->setText(c_input_placeholder);
      }
      return true;
    }
    if (Event->type() == QEvent::FocusIn) {
      if (!Combo->lineEdit()->hasSelectedText() ) {
        Combo->lineEdit()->selectAll();
      }
      return true;
    }
    if(Event->type() == QEvent::KeyPress) {
      QKeyEvent * KeyEvent = dynamic_cast<QKeyEvent *>(Event);
      if(KeyEvent->key() == Qt::Key_Up || KeyEvent->key() == Qt::Key_Down) {
        return true;
      } else {
        return false;
      }
    }
  }

  if ( Event->type() == QEvent::Wheel )
  {
    return true;
  }

  return false;
}

void combo::wheelEvent ( QWheelEvent * Event )
{
  Q_UNUSED ( Event )
  return;
}

void combo::SetController()
{
  connect ( Combo, SIGNAL ( editTextChanged ( const QString & ) ), this,
            SLOT ( TryValidate ( const QString & ) ), Qt::UniqueConnection );
  connect ( Combo, SIGNAL ( activated ( const QString & ) ), this,
            SLOT ( ChangeDetected ( const QString & ) ), Qt::UniqueConnection );
}

void combo::TryValidate ( QString tmp )
{
  if (tmp == c_input_placeholder) {
    Combo->lineEdit()->setPalette ( QApplication::palette ( this ) );
    return;
  }
  int index = 0;
  if (Combo->validator() != nullptr )
  {
    if ( Combo->validator()->validate ( tmp, index ) == QValidator::Acceptable )
    {
      m_base_data_editor->set_valid ( true );

      if ( CompareDefaults() )
      {
        Combo->lineEdit()->setPalette ( StyleUtility::LoadedDefault );
      }

      else
      {
        Combo->lineEdit()->setPalette ( QApplication::palette ( this ) );
      }
    }

    else if ( Combo->validator()->validate ( tmp, index ) == QValidator::Intermediate )
    {
      m_base_data_editor->set_not_null ( false );
      Combo->lineEdit()->setPalette ( StyleUtility::WarningStatusBarPallete );
    }
  }

  else
  {
    if ( CompareDefaults() )
    {
      Combo->setPalette ( StyleUtility::LoadedDefault );
    }
    else
    {
      Combo->setPalette ( QApplication::palette ( this ) );
    }
  }
}

void combo::ChangeDetected ( const QString & StringChange )
{
  Q_UNUSED ( StringChange )
  this_value_changed = true;
  emit signal_value_change();
}

void combo::CheckDefaults ( int DefaultIndex )
{
  Q_UNUSED ( DefaultIndex )

  if ( !this_defaults.isEmpty() )
  {
    TryValidate ( Combo->currentText() );
  }
}

bool combo::CompareDefaults()
{
  if ( this_defaults.isEmpty() )
  {
    return false;
  }

  if ( this_defaults == Combo->currentText() )
  {
    return true;
  }

  return false;
}

void combo::buildtooltip()
{}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
multiattr::multiattr ( t_virtue const & attr, QWidget * parent,
                       bool owned, bool readonly )
  :
  base ( std::make_shared<t_build_block_editor> ( attr ), parent, owned ),
  m_base_data_editor ( std::static_pointer_cast<t_build_block_editor> ( p_data_editor ) ),
  m_readonly(readonly),
  StatusBar ( nullptr ),
  OkButton ( nullptr ),
  RemoveButton ( nullptr ),
  ListWidget ( nullptr ),
  ContextMenu ( nullptr ),
  RemoveAction ( nullptr )
{
  t_virtue const & Virtue = m_base_data_editor->get();

  setWindowTitle ( QString ( "Edit Attribute : %1" ).arg ( Virtue.p_name.c_str() ) );
  QVBoxLayout * MainLayout = new QVBoxLayout ( this );
  MainLayout->setSpacing ( 0 );
  MainLayout->setMargin ( 0 );
  MainLayout->setContentsMargins ( 0, 0, 0, 0 );
  QHBoxLayout * ButtonLayout = new QHBoxLayout();
  ListWidget = new QListWidget ( this );
  ListWidget->setContextMenuPolicy ( Qt::CustomContextMenu );
  MainLayout->addWidget ( ListWidget );

  switch ( Virtue.p_type )
  {
  // Bool and enums have the same widget

  case dunedaq::conffwk::bool_type:
  case dunedaq::conffwk::class_type:
  case dunedaq::conffwk::enum_type:
  {
    combo * Combo = new combo ( attr, this );
    MainLayout->addWidget ( Combo );
    connect ( Combo->Combo, SIGNAL ( activated ( const QString & ) ), this,
              SLOT ( AddToDataList ( const QString & ) ), Qt::UniqueConnection );
    BaseWidget = Combo;
    Combo->setStyleSheet ( "QLineEdit { background: #c0c0c0;} "
                           "QLineEdit:focus {background: white;}" );
    break;
  }

  // All numeric types are treated as uint64

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
    numericattr * Numeric = new numericattr ( Virtue, this );
    MainLayout->addWidget ( Numeric );
    connect ( Numeric, SIGNAL ( signal_value_change() ), this, SLOT ( LineValueChanged() ),
              Qt::UniqueConnection );
    connect ( Numeric, SIGNAL ( signal_value_duplicated() ), this, SLOT ( LineValueChanged() ),
                  Qt::UniqueConnection );
    BaseWidget = Numeric;
    /// Frame
    Numeric->GetLineEdit()->setFrame ( false );
    Numeric->GetLineEdit()->setReadOnly(m_readonly);
    Numeric->GetLineEdit()->setPlaceholderText (c_input_placeholder);
    Numeric->setStyleSheet (
      "QLineEdit { background: #c0c0c0;} QLineEdit:focus {background: white;}" );
    Numeric->GetLineEdit()->setFixedHeight ( 24 );
    ListWidget->setFrameStyle ( QFrame::NoFrame );
    break;
  }

  // Types below are all treated as string types
  case dunedaq::conffwk::date_type:
  case dunedaq::conffwk::time_type:
  case dunedaq::conffwk::string_type:
  {
    stringattr * String = new stringattr ( Virtue, this );
    String->SetMultiCheck ( true );
    MainLayout->addWidget ( String );
    connect ( String, SIGNAL ( signal_value_change() ), this, SLOT ( LineValueChanged() ),
              Qt::UniqueConnection );
    BaseWidget = String;
    /// Frame
    String->GetLineEdit()->setReadOnly(m_readonly);
    String->GetLineEdit()->setFrameStyle(QFrame::NoFrame);
    String->GetLineEdit()->setPlaceholderText ( c_input_placeholder );
    String->setStyleSheet ( "QLineEdit { background: #c0c0c0;} "
                            "QLineEdit:focus {background: white;}" );
//    String->GetLineEdit()->setFixedHeight ( 24 );

    ListWidget->setFrameStyle ( QFrame::NoFrame );
    break;
  }

  default:
    break;
  }

  OkButton = new QPushButton ( tr ( "Apply" ) );
  //RemoveButton = new QPushButton(tr("Remove"));
  ButtonLayout->addWidget ( OkButton );
  //ButtonLayout->addWidget(RemoveButton);
  connect ( OkButton, SIGNAL ( clicked() ), this, SLOT ( EndSignal() ) );
  //connect(RemoveButton,SIGNAL(clicked()),this,SLOT(RemoveFromDataList()));
  connect ( this, SIGNAL ( signal_internal_value_change() ), this, SLOT ( UpdateActions() ) );
  connect ( ListWidget, SIGNAL ( customContextMenuRequested ( QPoint ) ), this,
            SLOT ( CustomContextMenuRequested ( QPoint ) ) );

  MainLayout->addLayout ( ButtonLayout );
  SetStatusBar();
  MainLayout->addWidget ( StatusBar );

  setLayout ( MainLayout );

  if ( Virtue.p_is_not_null )
  {
    m_base_data_editor->set_valid ( false );
    OkButton->setDisabled ( true );
  }

  if ( this_is_owned )
  {
    OkButton->setHidden ( true );
  }

  else
  {
    QFont Font;
    QFontMetrics FontMetrics ( Font );
    setMinimumWidth (
      2 * FontMetrics.horizontalAdvance ( QString ( "Edit Relationship: %1" ).arg ( Virtue.p_name.c_str() ) )
      - 15 );
    setMinimumHeight ( 100 );
  }

  buildtooltip();
  UpdateActions();
}

void multiattr::SetEditor()
{
  BaseWidget->SetEditor();

  ListWidget->clear();
  ListWidget->addItems ( this_data );
  ListWidget->setSelectionMode ( QAbstractItemView::ExtendedSelection );
  ListWidget->setDragEnabled ( true );
  ListWidget->setAcceptDrops ( true );
  ListWidget->setEditTriggers ( QAbstractItemView::DoubleClicked );
  ListWidget->setDragDropMode ( QAbstractItemView::InternalMove );

  for ( int i = 0; i < ListWidget->count(); ++i )
  {
    QListWidgetItem * widget = ListWidget->item ( i );
    widget->setFlags (
      Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
      | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled );
  }

  connect ( ListWidget, SIGNAL ( itemChanged ( QListWidgetItem * ) ), this,
            SLOT ( LineValueChanged ( QListWidgetItem * ) ) );
  emit signal_internal_value_change();
}

void multiattr::buildtooltip()
{
  t_virtue const & Virtue = m_base_data_editor->get();

  setToolTip (
    QString ( "Attribute Name:           %1 \n" ).arg ( Virtue.p_name.c_str() ) + QString (
      "          Type:           %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::type2str ( Virtue.p_type ) )
    + QString ( "          Range:          %1 \n" ).arg ( Virtue.p_range.c_str() )
    + QString ( "          Format:         %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::format2str ( Virtue.p_int_format ) )
    + QString ( "          Not Null:       %1 \n" ).arg ( Virtue.p_is_not_null )
    + QString ( "          Is Multi Value: %1 \n" ).arg ( Virtue.p_is_multi_value )
    + QString ( "          Default Value:  %1 \n" ).arg ( Virtue.p_default_value.c_str() )
    + QString ( "          Description:    %1 \n" ).arg ( Virtue.p_description.c_str() ) );
}

void multiattr::closeEvent ( QCloseEvent * Event )
{
  Q_UNUSED ( Event )
  emit signal_force_close();
}

void multiattr::SetStatusBar()
{
  StatusBar = new QStatusBar ( this );
  StatusBar->setSizeGripEnabled ( false );
  /// Color Management needs to be done
  StatusBar->setAutoFillBackground ( true );
  /// Uncomment if you want a status bar
  StatusBar->setHidden ( true );
}

bool multiattr::eventFilter ( QObject * Target, QEvent * Event )
{
  ( void ) Target;
  ( void ) Event;
  return false;
}

void multiattr::AddToDataList ( const QString & Data )
{
  if ( Data.isEmpty() )
  {
    return;
  }

  this_data.append ( Data );

  ListWidget->clear();
  ListWidget->addItems ( this_data );
  StatusBar->showMessage ( QString ( "Item %1 was added." ).arg ( Data ) );

  this_value_changed = true;
  emit signal_internal_value_change();
}

void multiattr::RemoveFromDataList()
{
  QList<QListWidgetItem *> widgets = ListWidget->selectedItems();
  this_value_changed = true;

  for ( QListWidgetItem * widget : widgets )
  {
    int index = this_data.indexOf ( widget->text() );

    if ( index != -1 )
    {
      this_data.takeAt ( index );
    }

    int r = ListWidget->row ( widget );

    if ( QListWidgetItem * w = ListWidget->takeItem ( r ) )
    {
      delete w;
    }

  }

  if ( this_is_owned )
  {
    StatusBar->showMessage ( "The selected items were removed." );
  }

  if ( dynamic_cast<stringattr *> ( BaseWidget ) )
  {
    /// Color management needs to be done
    stringattr * StringWidget = dynamic_cast<stringattr *> ( BaseWidget );
    StringWidget->GetLineEdit()->selectAll();
    StringWidget->GetLineEdit()->clear();
    StringWidget->SetCheckDefaults ( false );
    StringWidget->SetNullCheck ( true );
  }

  emit signal_internal_value_change();
  emit signal_value_change();
}

void multiattr::UpdateActions()
{
  t_virtue const & Virtue = m_base_data_editor->get();

  if ( Virtue.p_is_not_null and this_data.size() == 0 )
  {
    OkButton->setDisabled ( true );
    StatusBar->showMessage ( "Attribute can not be null." );
    m_base_data_editor->set_valid ( false );
    ListWidget->setPalette ( StyleUtility::WarningStatusBarPallete );
    /// EditString validator string
  }

  else if ( Virtue.p_is_not_null and this_data.size() > 0 )
  {
    /// Null Check validator string

    StatusBar->showMessage ( "Attribute is set." );
    ListWidget->setPalette ( QApplication::palette ( this ) );
    OkButton->setEnabled ( true );
    m_base_data_editor->set_valid ( true );

    if ( this_is_owned )
    {
      EndSignal();
    }
  }
  else if ( not Virtue.p_is_not_null and this_is_owned )
  {
    m_base_data_editor->set_valid ( true );
    EndSignal();
  }
}

void multiattr::LineValueChanged()
{
  QStringList Data = BaseWidget->getdata();

  if ( (!Data.isEmpty()) && BaseWidget->dataeditor()->is_valid())
  {
    AddToDataList ( Data.at ( 0 ) );
  }
}

void multiattr::LineValueChanged ( QListWidgetItem * changed )
{
  int r = ListWidget->row ( changed );
  QString newtext = changed->text();
  QString oldtext = this_data.at ( r );
  this_data.replace ( r, newtext );
  this_value_changed = true;

  StatusBar->showMessage ( QString ( "Item %1 was modified." ).arg ( oldtext ) );

  emit signal_internal_value_change();
}

void multiattr::ListOrderChange ( const QModelIndexList & IndexList )
{
  Q_UNUSED ( IndexList )

  this_data.clear();

  for ( int i = 0; i < ListWidget->count(); ++i )
  {
    this_data.append ( ListWidget->item ( i )->text() );
  }
}

void multiattr::EndSignal()
{
  this_data.clear();

  for ( int i = 0; i < ListWidget->count(); ++i )
  {
    this_data.append ( ListWidget->item ( i )->text() );
  }

  if ( !this_is_owned )
  {
    m_base_data_editor->set_valid ( true );
    emit signal_edit_end();
  }

  else
  {
    emit signal_value_change();
  }
}

void multiattr::CustomContextMenuRequested ( const QPoint & pos )
{
  if ( ContextMenu == nullptr )
  {
    ContextMenu = new QMenu ( ListWidget );

    RemoveAction = new QAction ( tr ( "Remove" ), this );
    RemoveAction->setShortcutContext ( Qt::WidgetShortcut );
    connect ( RemoveAction, SIGNAL ( triggered() ), this, SLOT ( RemoveSlot() ),
              Qt::UniqueConnection );
    ContextMenu->addAction ( RemoveAction );
  }

  ContextMenu->exec ( ListWidget->mapToGlobal ( pos ) );
}

void multiattr::RemoveSlot()
{
  RemoveFromDataList();
}

//------------------------------------------------------------------------------------------------

} // end namespace editors
} // end namespace widgets
} // end namespace dbe

//------------------------------------------------------------------------------------------------
