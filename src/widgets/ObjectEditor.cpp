#include "dbe/ObjectEditor.hpp"
#include "dbe/StyleUtility.hpp"
#include "dbe/Sorting.hpp"
#include "dbe/config_api_set.hpp"
#include "dbe/config_api_get.hpp"
#include "dbe/config_api_graph.hpp"
#include "dbe/config_api_commands.hpp"
#include "dbe/messenger.hpp"
#include "ui_ObjectEditor.h"
#include "dbe/MainWindow.hpp"

#include <QFileInfo>
#include <QCloseEvent>
#include <QMessageBox>

namespace dbegraph = dbe::config::api::graph;

namespace {
    class NoScrollingTable : public QTableWidget {
        public:
           explicit NoScrollingTable(QWidget* parent = nullptr)
             : QTableWidget(parent) {}

      void scrollTo(const QModelIndex& /*index*/, ScrollHint /*hint*/) override {
                // NOTE: for the reason why this is an empty implementation, see ATLASDBE-202
            }
    };
} // namespace

//------------------------------------------------------------------------------------------
dbe::ObjectEditor::~ObjectEditor() = default;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
dbe::ObjectEditor::ObjectEditor(QWidget * parent)
  :
  QWidget(parent),
  ui ( new Ui::ObjectEditor ),
  IsValid ( false ),
  this_editor_values_changed ( false ),
  CurrentRow ( 0 ),
  MainLayout ( new QHBoxLayout() ),
  // WidgetTable ( new NoScrollingTable() ),
  WidgetTable ( new QTableWidget() ),
  RenameWidget ( nullptr ),
  LineEdit ( nullptr ),
  GoButton ( nullptr ),
  MoveWidget ( nullptr ),
  FileView ( nullptr ),
  IncludedFileModel ( nullptr ),
  MoveGoButton ( nullptr ),
  uuid ( QUuid::createUuid() )
{
  ui->setupUi ( this );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------

dbe::ObjectEditor::ObjectEditor ( std::string const & cname, QWidget * parent )
  : ObjectEditor ( parent ) {

  classname = cname;
  m_object_to_edit = nullptr;
  this_is_in_copy_mode = true;
  this_editor_is_owned = true;
  this_is_in_creation_mode = true;

  ui->RenameButton->setDisabled ( true ); // Cannot rename an object that does not exist
  ui->ClassLabel->setText ( QString ( "New Object" ) );

  init();
}

//------------------------------------------------------------------------------------------

dbe::ObjectEditor::ObjectEditor ( tref const & object, QWidget * parent, bool iscopy )
  : ObjectEditor ( parent ) {

  classname = object.class_name();
  m_object_to_edit.reset(new dref ( object ));
  this_is_in_copy_mode = iscopy;
  this_editor_is_owned = false;
  this_is_in_creation_mode = false;

  ui->ClassLabel->setText (
    QString ( "Full object name : %1@%2" ).arg ( Object().UID().c_str() ).arg (
      Object().class_name().c_str() ) );

  this->setWindowTitle (
    QString ( "Edit Object %1 of Class %2" ).arg ( Object().UID().c_str() ).arg (
      Object().class_name().c_str() ) );

  this->setObjectName (
    QString ( "%1@%2" ).arg ( Object().UID().c_str() ).arg ( Object().class_name().c_str() ) );

  init();
}

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::init() {

  if (m_object_to_edit and not Object().is_null()) {
    m_readonly = !confaccessor::check_file_rw (
      QString::fromStdString ( Object().contained_in() ));
  }

  dunedaq::conffwk::class_t const & Class =
    dbe::config::api::info::onclass::definition ( classname, false );
  int NumberOfRows = Class.p_attributes.size() + Class.p_relationships.size();
  int NumberOfColumns = 1;
  WidgetTable->setRowCount ( NumberOfRows );
  WidgetTable->setColumnCount ( NumberOfColumns );
  setAttribute ( Qt::WA_DeleteOnClose, true );

  SetController();
  BuildWidgets();
  UpdateActions();
  BuildFileInfo();

  ui->DetailsGroupBox->setVisible ( false );

  WidgetTable->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding );
  WidgetTable->horizontalHeader()->setVisible ( false );
  WidgetTable->horizontalHeader()->setSectionResizeMode ( QHeaderView::ResizeToContents );
  WidgetTable->horizontalHeader()->setSectionResizeMode ( 0, QHeaderView::Stretch );
  WidgetTable->setVerticalHeaderLabels ( HorizontalHeaders );
  WidgetTable->setSelectionMode ( QAbstractItemView::NoSelection );

  if ( this_is_in_creation_mode )
  {
    ui->RenameButton->setDisabled ( true );
    ui->MoveButton->setDisabled ( true );
  }
  else {
    if (m_readonly) {
      // WidgetTable->setDisabled(true);
      ui->RenameButton->setDisabled(true);
      ui->MoveButton->setDisabled(true);
      ui->ApplyButton->hide();
    }
  }
  ui->TableLayout->addWidget ( WidgetTable );

  ui->ApplyButton->setEnabled ( false );

  ui->RenameButton->setToolTip ( "Rename object" );


  this->show();
}

void dbe::ObjectEditor::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::HideDetailWidget ( bool Hide )
{
  if ( Hide )
  {
    ui->DetailWidget->hide();
    ui->DetailsGroupBox->hide();
  }
  else
  {
    ui->DetailWidget->show();
    ui->DetailsGroupBox->show();
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool dbe::ObjectEditor::IsEditorValid() const
{
  return IsValid;
}

bool dbe::ObjectEditor::WasObjectChanged() const
{
  return this_editor_values_changed;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool dbe::ObjectEditor::CanCloseWindow()
{
  if ( this_editor_values_changed and 
       ui->ApplyButton->isEnabled() )
  {
    int ret =
      QMessageBox::question (
        0,
        tr ( "DBE - ObjectEditor" ),
        QString (
          "There are unsaved changes for object\n'%1.%2'\n\nDo you want to apply changes?\n" )
        .arg ( QString::fromStdString ( Object().full_name() ) ),
        QMessageBox::Apply | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Apply );

    switch ( ret )
    {

    case QMessageBox::Discard:
      ResetObjectChanged();
      break;

    case QMessageBox::Apply:
      ParseToSave();
      break;

    case QMessageBox::Cancel:
      return false;
      break;
    }

    return true;
  }
  else
  {
    return true;
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::SetStatusBar()
{
  StatusBar->setSizeGripEnabled ( false );
  StatusBar->setAutoFillBackground ( true );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::SetController()
{
  connect ( ui->DetailButton, SIGNAL ( toggled ( bool ) ), ui->DetailsGroupBox,
            SLOT ( setVisible ( bool ) ), Qt::UniqueConnection );
  connect ( ui->CloseButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( close() ),
            Qt::UniqueConnection );
  connect ( ui->ApplyButton, SIGNAL ( clicked() ), this, SLOT ( ParseToSave() ),
            Qt::UniqueConnection );

  connect ( ui->RenameButton, SIGNAL ( clicked() ), this, SLOT ( LaunchRenameObject() ) );
  connect ( ui->MoveButton, SIGNAL ( clicked() ), this, SLOT ( LaunchMoveObject() ) );

  connect ( &confaccessor::ref(), SIGNAL ( object_changed ( QString, dref ) ), this,
            SLOT ( UpdateObjectEditor ( QString, dref ) ) );

  connect ( &confaccessor::ref(), SIGNAL ( object_deleted ( QString, dref ) ), this,
            SLOT ( ShouldCloseThisWindow ( QString, dref ) ) );

  MainWindow * mainwin = MainWindow::findthis();
  if(mainwin != nullptr) {
      connect (mainwin, SIGNAL(signal_batch_change_stopped(const QList<QPair<QString, QString>>&)),
               this, SLOT(UpdateObjectEditor(const QList<QPair<QString, QString>>&)), Qt::UniqueConnection);
      connect (mainwin, SIGNAL(signal_externalchanges_processed()),
               this, SLOT(UpdateObjectEditor()), Qt::UniqueConnection);
  }
}

void dbe::ObjectEditor::UpdateObjectEditor(const QList<QPair<QString, QString>>& objs) {
    for(const auto& p : objs) {
        if((p.first.toStdString() == Object().class_name()) && (p.second.toStdString() == Object().UID())) {
            UpdateObjectEditor("", inner::dbcontroller::get({Object().UID(), Object().class_name()}));
            break;
        }
    }
}

void dbe::ObjectEditor::UpdateObjectEditor() {
    const auto& ref = inner::dbcontroller::get({Object().UID(), Object().class_name()});
    if(ref.is_null() == false) {
        UpdateObjectEditor("", inner::dbcontroller::get({Object().UID(), Object().class_name()}));
    } else {
        // The object has been deleted
        this->close();
    }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::UpdateObjectEditor ( QString const & src, dref updated_object )
{
  // src is ignored even in the case of this being the source
  // changes must always be applied since it is developer responsibility
  // to emit signals as needed
  Q_UNUSED ( src );

  if ( not this_editor_is_owned and m_object_to_edit and Object().UID() == updated_object.UID()
       and Object().class_name() == updated_object.class_name() )
  {

    m_object_to_edit.reset ( new dref ( updated_object ) );

    dunedaq::conffwk::class_t const & classdef =
      dbe::config::api::info::onclass::definition ( classname, false );
    std::vector<dunedaq::conffwk::attribute_t> class_attributes = classdef.p_attributes;
    std::vector<dunedaq::conffwk::relationship_t> class_relations = classdef.p_relationships;

    for ( dunedaq::conffwk::attribute_t attr : class_attributes )
    {
      if ( widgets::editors::base * attreditor =
             this_widgets[QString::fromStdString ( attr.p_name )] )
      {
        set_attribute_widget ( attr, attreditor );
      }
    }

    for ( dunedaq::conffwk::relationship_t arelation : class_relations )
    {
      QStringList relvalues;
      QString relname = QString ( arelation.p_name.c_str() );

      if ( widgets::editors::relation * relwidget =
             dynamic_cast<widgets::editors::relation *> ( this_widgets[relname] ) )
      {
        if ( relwidget->ischanged() )
        {
          relvalues = relwidget->getdata();
        }
        else
        {
          std::vector<tref> connected;

          if ( config::api::info::relation::is_simple ( arelation ) )
          {
            try
            {
              connected.push_back (
                config::api::graph::linked::through::relation<tref> ( Object(), arelation ) );
            }
            catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
            {
              // nothing needs be done to handle the case that a relation has no object set
            }
          }
          else
          {
            connected =
              dbegraph::linked::through::relation<std::vector<tref>> ( Object(), arelation );
          }

          std::transform ( connected.begin(), connected.end(), std::back_inserter ( relvalues ),
                           [] ( decltype ( connected ) ::value_type const & x )
          {
            return QString::fromStdString ( x.UID() );
          }

                         );

        }

        relwidget->setdata ( relvalues );
        relwidget->SetEditor();
      }
    }
  }

  // Trick to properly refresh the window (repaint and update does not seem to work)
  const auto& s = size();
  resize(s.width() + 1, s.height() +1);
  resize(s.width(), s.height());
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::ShouldCloseThisWindow ( QString const src, dref const key )
{
  Q_UNUSED ( src );

  std::string const & fullname = key.UID() + "@" + key.class_name();

  if ( ( m_object_to_edit and not m_object_to_edit->is_valid() )
       or ( this->objectName().toStdString() == fullname ) )
  {
    this->close();
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::BuildWidgets()
{
  dunedaq::conffwk::class_t const & classdef =
    dbe::config::api::info::onclass::definition ( classname, false );
  std::vector<dunedaq::conffwk::attribute_t> attributes = classdef.p_attributes;
  std::vector<dunedaq::conffwk::relationship_t> relations = classdef.p_relationships;

  for ( dunedaq::conffwk::attribute_t const & attr : attributes ) // Build widgets for attributes

  {
    QString name = QString::fromStdString ( attr.p_name );

    if ( attr.p_is_multi_value )
    {
      widgets::editors::multiattr * widget = new widgets::editors::multiattr ( attr, this,
                                                                               true, m_readonly );
      set_attribute_widget ( attr, widget );

      set_tooltip ( attr, widget );
      register_attribute_widget ( name, widget );
      connect ( widget, SIGNAL ( signal_value_change() ), this, SLOT ( UpdateActions() ),
                Qt::UniqueConnection );
      connect ( widget, SIGNAL ( signal_value_change() ), this, SLOT ( ObjectChanged() ),
                Qt::UniqueConnection );
      emit LoadedInitials();
    }
    else
    {
      switch ( attr.p_type )
      {

      case dunedaq::conffwk::enum_type:

      case dunedaq::conffwk::bool_type:
      {
        widgets::editors::combo * Widget = new widgets::editors::combo ( attr, this, true );
        set_attribute_widget ( attr, Widget );
        set_tooltip ( attr, Widget );
        register_attribute_widget ( name, Widget );
        connect ( Widget->Combo, SIGNAL ( activated ( QString ) ), this, SLOT ( UpdateActions() ),
                  Qt::UniqueConnection );
        connect ( Widget, SIGNAL ( signal_value_change() ), this, SLOT ( ObjectChanged() ),
                  Qt::UniqueConnection );
        emit LoadedInitials();
        break;
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
        widgets::editors::numericattr * Widget =
          new widgets::editors::numericattr ( attr, this, true, m_readonly );
        set_attribute_widget ( attr, Widget );
        set_tooltip ( attr, Widget );
        register_attribute_widget ( name, Widget );
        connect ( Widget, SIGNAL ( signal_value_change() ), this, SLOT ( UpdateActions() ),
                  Qt::UniqueConnection );
        connect ( Widget, SIGNAL ( signal_value_change() ), this, SLOT ( ObjectChanged() ),
                  Qt::UniqueConnection );
        emit LoadedInitials();
        break;
      }

      case dunedaq::conffwk::string_type:

      case dunedaq::conffwk::date_type:

      case dunedaq::conffwk::time_type:
      {
        widgets::editors::stringattr * Widget =
          new widgets::editors::stringattr ( attr, this, true, m_readonly );
        set_attribute_widget ( attr, Widget );
        set_tooltip ( attr, Widget );
        register_attribute_widget ( name, Widget );
        connect ( Widget, SIGNAL ( signal_value_change() ), this, SLOT ( UpdateActions() ),
                  Qt::UniqueConnection );
        connect ( Widget, SIGNAL ( signal_value_change() ), this, SLOT ( ObjectChanged() ),
                  Qt::UniqueConnection );
        emit LoadedInitials();
        break;
      }

      case dunedaq::conffwk::class_type:
      {
        widgets::editors::combo * Widget = new widgets::editors::combo ( attr, this, true );
        set_attribute_widget ( attr, Widget );
        set_tooltip ( attr, Widget );
        register_attribute_widget ( name, Widget );
        connect ( Widget->Combo, SIGNAL ( activated ( QString ) ), this, SLOT ( UpdateActions() ),
                  Qt::UniqueConnection );
        connect ( Widget, SIGNAL ( signal_value_change() ), this, SLOT ( ObjectChanged() ),
                  Qt::UniqueConnection );
        emit LoadedInitials();
        break;
      }
      }
    }
  }

  for ( dunedaq::conffwk::relationship_t const & arelation :
        relations ) // Build widgets for relations ( relationships )
  {
    widgets::editors::relation * widget = new widgets::editors::relation ( arelation, this,
                                                                           true, m_readonly );
    QString name = QString::fromStdString ( arelation.p_name );
    QStringList Data;

    if ( m_object_to_edit and not Object().is_null() )
    {
      std::vector<tref> DataList;

      if ( config::api::info::relation::is_simple ( arelation ) )
      {
        try
        {
          DataList.push_back (
            dbegraph::linked::through::relation<tref> ( Object(), arelation ) );
        }
        catch ( daq::dbe::config_object_retrieval_result_is_null const & e )
        {
          // nothing needs be done to handle cases that a relation has not been set
        }
      }
      else
      {
        DataList = dbegraph::linked::through::relation<std::vector<tref>> ( Object(), arelation );
      }

      for ( tref const & i : DataList )
      {
        if ( not i.is_null() )
        {
          Data.push_back ( QString::fromStdString ( i.UID() ) );
        }
      }
    }

    widget->setdata ( Data );
    widget->SetEditor();

    set_tooltip ( arelation, widget );
    register_relation_widget ( name, widget );
    connect ( widget, SIGNAL ( signal_value_change() ), this, SLOT ( UpdateActions() ),
              Qt::UniqueConnection );
    connect ( widget, SIGNAL ( signal_value_change() ), this, SLOT ( ObjectChanged() ),
              Qt::UniqueConnection );
    connect ( widget, SIGNAL ( LoadedInitials() ), this, SLOT ( ResetObjectChanged() ),
              Qt::UniqueConnection );
    connect ( this, SIGNAL ( LoadedInitials() ), widget, SLOT ( slot_set_initial_loaded() ),
              Qt::UniqueConnection );
    /// Some connections are missing
    emit LoadedInitials();
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::set_tooltip ( dunedaq::conffwk::attribute_t const & Attribute,
                                      widgets::editors::base * Widget )
{
  QString ToolTip;
  ToolTip.append ( QString ( "Attribute Name:           %1 \n" ).arg (
                     Attribute.p_name.c_str() ) );
  ToolTip.append (
    QString ( "          Type:           %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::type2str ( Attribute.p_type ) ) );
  ToolTip.append ( QString ( "          Range:          %1 \n" ).arg (
                     Attribute.p_range.c_str() ) );
  ToolTip.append (
    QString ( "          Format:         %1 \n" ).arg (
      dunedaq::conffwk::attribute_t::format2str ( Attribute.p_int_format ) ) );
  ToolTip.append ( QString ( "          Not Null:       %1 \n" ).arg (
                     Attribute.p_is_not_null ) );
  ToolTip.append (
    QString ( "          Is Multi Value: %1 \n" ).arg ( Attribute.p_is_multi_value ) );
  ToolTip.append (
    QString ( "          Default Value:  %1 \n" ).arg ( Attribute.p_default_value.c_str() ) );
  ToolTip.append (
    QString ( "          Description:    %1 \n" ).arg ( Attribute.p_description.c_str() ) );
  Widget->setToolTip ( ToolTip );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::set_tooltip ( dunedaq::conffwk::relationship_t const & relation,
                                      widgets::editors::base * widget )
{
  QString ToolTip;
  ToolTip.append (
    QString ( "Relationship Name:           %1 \n" ).arg ( relation.p_name.c_str() ) );
  ToolTip.append (
    QString ( "             Type:           %1 \n" ).arg ( relation.p_type.c_str() ) );
  ToolTip.append (
    QString ( "             Cardinality     %1 \n" ).arg (
      dunedaq::conffwk::relationship_t::card2str ( relation.p_cardinality ) ) );
  ToolTip.append (
    QString ( "             Is Aggregation: %1 \n" ).arg ( relation.p_is_aggregation ) );
  ToolTip.append (
    QString ( "             Description:    %1 \n" ).arg ( relation.p_description.c_str() ) );
  widget->setToolTip ( ToolTip );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::set_attribute_widget ( dunedaq::conffwk::attribute_t const & Attribute,
                                               widgets::editors::base * Widget )
{
  {
    QStringList values;

    if ( not Widget->ischanged() and m_object_to_edit and not Object().is_null() )
    {
      values = dbe::config::api::get::attribute::list<QStringList> ( Object(), Attribute );
    }
    else
    {
      values = Widget->getdata();
    }

    Widget->setdata ( values );
  }

  {
    QStringList defaults_values
    { dbe::config::api::get::defaults::attribute::value ( Attribute ) };
    Widget->setdefaults ( defaults_values.at ( 0 ) );

    if ( this_is_in_creation_mode) {
      Widget->setdata(defaults_values);
    }
  }

  Widget->SetEditor();
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::register_attribute_widget ( QString const & name,
                                                    widgets::editors::base * widget )
{
    HorizontalHeaders.append(name);
    WidgetTable->setCellWidget(CurrentRow, 0, widget);

    if(dynamic_cast<widgets::editors::multiattr *>(widget)) {
        WidgetTable->setRowHeight(CurrentRow, 100);
    } else if(widgets::editors::stringattr * sa = dynamic_cast<widgets::editors::stringattr *>(widget)) {
        QTextEdit* le = sa->GetLineEdit();
        const auto& textSize = QFontMetrics(le->document()->defaultFont()).size(0, le->toPlainText());
        WidgetTable->setRowHeight(CurrentRow, std::min(150, textSize.height() + 15));
    } else {
        WidgetTable->verticalHeader()->setSectionResizeMode(CurrentRow, QHeaderView::Fixed);
    }

    CurrentRow++;
    this_widgets[name] = widget;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::register_relation_widget ( QString const & name,
                                                   widgets::editors::base * widget )
{
  HorizontalHeaders.append ( name );
  WidgetTable->setCellWidget ( CurrentRow, 0, widget );

  if(widgets::editors::relation * w = dynamic_cast<widgets::editors::relation *>(widget)) {
      if(w->GetIsMultiValue() == true) {
          WidgetTable->setRowHeight(CurrentRow, 100);
      } else {
          WidgetTable->verticalHeader()->setSectionResizeMode(CurrentRow, QHeaderView::Fixed);
      }
  }

  CurrentRow++;
  this_widgets[name] = widget;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::BuildFileInfo()
{
  if ( m_object_to_edit and not Object().is_null() )
  {
    QString FileName = QString ( Object().contained_in().c_str() );
    QList<QStringList> FileCache = confaccessor::ref().GetIncludedFileCache();
    QFileInfo FileInfo = QFileInfo ( FileName );

    for ( QStringList File : FileCache )
    {
      if ( FileName.contains ( File.at ( 1 ) ) )
      {
        FilePermission = File.at ( 2 );
        break;
      }
    }

    ui->FileLabel->setText ( QString ( "File: %1" ).arg ( FileInfo.fileName() ) );
    ui->DirLabel->setText ( QString ( "Dir: %1" ).arg ( FileInfo.absolutePath() ) );
    ui->WriteLabel->setText ( QString ( "Permission: %1" ).arg ( FilePermission ) );
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// void dbe::ObjectEditor::closeEvent ( QCloseEvent * e )
// {
//   if ( not this_editor_is_owned and CanCloseWindow() )
//   {
//     e->accept();
//   }
//   else if ( this_editor_is_owned )
//   {
//     e->accept();
//   }
//   else
//   {
//     e->ignore();
//   }
// }

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::UpdateActions()
{
  int NotNullCounter = 0;
  int NotValidCounter = 0;

  QStringList NotValidList;

  for ( auto & i : this_widgets )
  {
    std::shared_ptr<editor_data_state> Editor = i.second->dataeditor<editor_data_state>();

    if ( Editor != nullptr )
    {
      if ( not Editor->is_valid() )
      {
        NotValidCounter++;
        NotValidList.append ( i.first );
      }

      if ( Editor->must_not_be_null() )
      {
        NotNullCounter++;
      }
    }
  }

  QLabel * StatusLabel = new QLabel();
  StatusLabel->setWordWrap ( true );
  StatusLabel->setFrameStyle ( QFrame::NoFrame );

  if ( NotValidCounter == 0 )
  {
    StatusLabel->setText (
      QString ( "All minimal necessary attributes and relationships are set" ) );

    if ( this_is_in_copy_mode or this_is_in_creation_mode or
         confaccessor::check_file_rw ( QString::fromStdString ( Object().contained_in() ) ) )
    {
      ui->ApplyButton->setEnabled ( true );
    }

    IsValid = true;
  }
  else
  {
    QString MexHead = QString ( "From %1 NOT NULL attributes/relationships, %2 are not set: " )
                      .arg ( NotNullCounter ).arg ( NotValidCounter );
    QString Mex = MexHead + NotValidList.join ( "," );
    StatusLabel->setText ( Mex );

    ui->ApplyButton->setEnabled ( false );

    IsValid = false;
  }

  /// This signal will be caught by object editor
  emit WidgetUpdated();
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::ObjectChanged()
{
  this_editor_values_changed = true;

  if ( IsValid and (this_is_in_creation_mode
       or confaccessor::check_file_rw ( QString::fromStdString ( Object().contained_in() ) ) ))
  {
    ui->ApplyButton->setEnabled ( true );
  }
}

void dbe::ObjectEditor::ResetObjectChanged()
{
  this_editor_values_changed = false;
  ui->ApplyButton->setEnabled ( false );
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::ParseToSave()
{
  if ( FilePermission != "RO" )
  {
    if ( not IsValid )
    {
      ERROR ( "Changes cannot be applied for object", "Changes are invalid", "with UID:",
              Object().UID(), "of class:", Object().class_name() );
      return;
    }

    for ( auto const & i : this_widgets )
    {
      widgets::editors::base * ceditor = i.second;
      bool Changed = ceditor->ischanged();

      if ( Changed || this_is_in_copy_mode )
      {
        QStringList DataList = ceditor->getdata();

        if ( auto attreditor = ceditor->dataeditor<editor_data<dunedaq::conffwk::attribute_t>>() )
        {
          dunedaq::conffwk::attribute_t Attribute = attreditor->get();

          dbe::config::api::set::attribute ( Object(), Attribute, DataList );
          ceditor->setdata ( DataList );
          ceditor->setchanged ( false );
        }
        else if ( auto releditor =
                    ceditor->dataeditor<editor_data<dunedaq::conffwk::relationship_t>>() )
        {
          dunedaq::conffwk::relationship_t Relationship = releditor->get();
          dbe::config::api::set::relation ( Object(), Relationship, DataList );
          ceditor->setdata ( DataList );
          ceditor->setchanged ( false );
        }
      }
    }

    MainWindow::findthis()->build_file_model();

    ui->ApplyButton->setDisabled ( true );

    this_editor_values_changed = false;
  }
  else
  {
    ERROR ( "Changes cannot be applied", "File access permission error" );
  }
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::LaunchRenameObject()
{
  if ( RenameWidget == nullptr )
  {
    RenameWidget = new QDialog ( this );
    RenameWidget->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
    RenameWidget->setToolTip ( "Type string to edit line and press Enter." );
    RenameWidget->setWindowTitle ( "Choose a new object id" );

    QHBoxLayout * Layout = new QHBoxLayout ( RenameWidget );
    QLabel * Label = new QLabel ( QString ( "Rename Object:" ), RenameWidget );
    GoButton = new QPushButton ( "Rename !" );

    LineEdit = new QLineEdit ( RenameWidget );
    LineEdit->setToolTip ( "Type string and press Enter" );

    Layout->addWidget ( Label );
    Layout->addWidget ( LineEdit );
    Layout->addWidget ( GoButton );
    RenameWidget->setLayout ( Layout );

    connect ( LineEdit, SIGNAL ( returnPressed() ), this, SLOT ( RenameObject() ) );
    connect ( GoButton, SIGNAL ( clicked() ), this, SLOT ( RenameObject() ) );
    connect ( &confaccessor::ref(), SIGNAL ( object_renamed ( QString, dref ) ), this,
              SLOT ( slot_external_rename_object ( QString, dref ) ) );
  }

  RenameWidget->show();
}

void dbe::ObjectEditor::LaunchMoveObject()
{
  if ( MoveWidget == nullptr )
  {
    MoveWidget = new QDialog ( this );
    MoveWidget->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
    MoveWidget->setToolTip ( "Choose file and press Move." );
    MoveWidget->setWindowTitle ( "Choose new file for object" );

    QVBoxLayout * Layout = new QVBoxLayout ( MoveWidget );
    QLabel * Label = new QLabel ( QString ( "Choose new file" ), MoveWidget );
    MoveGoButton = new QPushButton ( "Move !" );

    if ( IncludedFileModel == nullptr )
    {
      QList<QStringList> List = confaccessor::ref().GetIncludedFileCache();
      IncludedFileModel = new FileModel ( List );
    }

    FileView = new CustomFileView ( MoveWidget );
    FileView->setModel ( IncludedFileModel );
    FileView->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );

    Layout->addWidget ( Label );
    Layout->addWidget ( FileView );
    Layout->addWidget ( MoveGoButton );

    MoveWidget->setLayout ( Layout );
    connect ( FileView, SIGNAL ( stateChanged ( const QString & ) ), this,
              SLOT ( ActiveFileChanged ( const QString & ) ), Qt::UniqueConnection );
    connect ( MoveGoButton, SIGNAL ( clicked() ), this, SLOT ( MoveObject() ) );
  }

  MoveWidget->show();
}

//------------------------------------------------------------------------------------------

void dbe::ObjectEditor::save_and_close() {
  if (ui->ApplyButton->isEnabled() ) {
    ParseToSave();
  }
  close();
}

//------------------------------------------------------------------------------------------
void dbe::ObjectEditor::RenameObject()
{
  std::string newname = LineEdit->text().toStdString();
  dbe::config::api::commands::renobj ( Object(), newname, uuid );
  RenameWidget->close();
}

void dbe::ObjectEditor::slot_external_rename_object ( QString const & src,
                                                      dref const & /* obj */)
{
  // Rename can only occur by undoing a command issued from an ObjectEditor therefore
  // the uuid and src have to be equal. We also check the UID

  if ( src == uuid.toString() )
  {
    ui->ClassLabel->setText (
      QString ( "Object : %1@%2" ).arg ( Object().UID().c_str() ).arg (
        Object().class_name().c_str() ) );
    setWindowTitle (
      QString ( "Edit Object %1 of Class %2" ).arg ( Object().UID().c_str() ).arg (
        Object().class_name().c_str() ) );
    setObjectName (
      QString ( "%1@%2" ).arg ( Object().UID().c_str() ).arg ( Object().class_name().c_str() ) );
  }
}

void dbe::ObjectEditor::MoveObject()
{
  if ( dbe::config::api::commands::movobj ( Object(), ActivateFile.toStdString(), uuid ) )
  {
    ui->FileLabel->setText ( QString ( "File: %1" ).arg ( Object().contained_in().c_str() ) );
  }

  MoveWidget->close();
}

void dbe::ObjectEditor::ActiveFileChanged ( const QString & File )
{
  if ( File.isEmpty() )
  {
    return;
  }

  ActivateFile = File;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
bool dbe::ObjectEditor::ParseToCreate ( std::string const & objectname,
                                        std::string const & filename )
{
  if ( not config::api::info::has_obj ( classname, objectname ) )
  {
    try
    {
      dbe::t_config_object_preimage::type_attrmap object_attributes;
      dbe::t_config_object_preimage::type_relmap object_relations;

      // Build [attribute , new values ] map and [ relation , new relations ] map
      // by looping through all widgets contained in this objects

      for ( auto const & awidget : this_widgets )
      {
        widgets::editors::base * editor = awidget.second;

        // Retrieve data held by the editor
        QStringList editordata = editor->getdata();

        if ( std::shared_ptr<editor_data<dunedaq::conffwk::attribute_t>> accessor =
               editor->dataeditor<editor_data<dunedaq::conffwk::attribute_t>>() )
        {
          dunedaq::conffwk::attribute_t attribute = accessor->get();

          /*
           * Convert to the preimage value type
           */

          for ( auto const & element : editordata )
          {
            object_attributes[attribute.p_name].push_back ( element.toStdString() );
          }
        }
        else if ( std::shared_ptr<editor_data<dunedaq::conffwk::relationship_t>> accessor =
                    editor->dataeditor<editor_data<dunedaq::conffwk::relationship_t>>() )
        {
          dunedaq::conffwk::relationship_t relation = accessor->get();

          /*
           * Convert to the preimage value type
           */

          for ( auto const & element : editordata )
          {
            object_relations[relation.p_name].push_back (
            { element.toStdString(), relation.p_type } );
          }
        }

        //        editor->SetValueList(editordata);
        editor->setchanged ( false );
      }

      dbe::config::api::commands::newobj (
        filename, classname, objectname,
        object_attributes, object_relations, uuid );

    }
    catch ( daq::dbe::ObjectChangeWasNotSuccessful const & e )
    {
      ERROR ( "Object creation did not complete successfully", dbe::config::errors::parse ( e ) );
      return false;
    }
    catch ( dunedaq::conffwk::Exception const & e )
    {
      ERROR ( "Object could not be created", dbe::config::errors::parse ( e ) );
      return false;
    }

    return true;
  }
  else
  {
    ERROR ( "Object cannot be created for", "Object already exists", "with name (UID):",
            objectname, " in class ", classname );
    return false;
  }
}

void dbe::ObjectEditor::SetUsedForCopy ( bool Used )
{
  this_is_in_copy_mode = Used;
}

//------------------------------------------------------------------------------------------
