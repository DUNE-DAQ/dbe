// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

#include "dbe/BatchChangeWidget.hpp"

#include "ui_BatchChangeWidget.h"

#include "dbe/MainWindow.hpp"
#include "dbe/config_api.hpp"
#include "dbe/config_api_set.hpp"
#include "dbe/config_api_get.hpp"
#include "dbe/config_api_graph.hpp"
#include "dbe/messenger.hpp"

#include <QApplication>
#include <boost/scope_exit.hpp>

namespace dbegraph = dbe::config::api::graph;

dbe::BatchChangeWidget::~BatchChangeWidget() = default;

dbe::BatchChangeWidget::BatchChangeWidget ( QWidget * parent )
  :
  QWidget ( parent ),
  ui ( new Ui::BatchChangeWidget ),
  UseTable ( false ),
  ClassChanged ( true ),
  TableClass ( "" ),
  m_filter_table ( nullptr )
{
  ui->setupUi ( this );
  ui->TitleLabel->setStyleSheet (
    "QLabel { background-color:green; color:beige; font:bold14px; border-style:outset; border-width:2px;"
    "border-radius:10px; border-color:beige; font:bold14px; min-width:10em; padding:6px}" );
  ui->FirstStepLabel->setStyleSheet ( "QLabel { color:#DB5816; font: bold 14px;}" );
  ui->SecondStepLabel->setStyleSheet ( "QLabel { color:#DE8A14; font: bold 14px;}" );
  ui->ThirdStepLabel->setStyleSheet ( "QLabel { color:#8DAA2E; font: bold 14px;}" );
  setWindowTitle ( "Batch Change" );

  QStringList ListOfClasses ( dbe::config::api::info::onclass::allnames<QStringList>() );
  ui->ClassCombo->addItems ( ListOfClasses );
  ui->ClassCombo->setEditable ( true );
  ui->ClassCombo->setEditText ( "Type/Select Classes" );

  ui->AttributeBox->setEnabled ( false );
  ui->RelationshipBox->setEnabled ( false );
  ui->NewAttributeGroupBox->setEnabled ( false );
  ui->NewRelationshipGroupBox->setEnabled ( false );
  ui->MatchButton->setEnabled ( false );
  ui->FilterUid->setText ( "*" );
  ui->AttributeValueFilter->setText ( "*" );

  SetController();
}

dbe::BatchChangeWidget::BatchChangeWidget ( bool ObjectsFromTable, QString ClassName,
                                            std::vector<dref> & Objects, QWidget * parent )
  :
  BatchChangeWidget ( parent )
{
  ClassChanged = false;

  TableObjects = Objects;
  UseTable = ObjectsFromTable;
  TableClass = ClassName;

  ui->ClassCombo->setCurrentIndex ( ui->ClassCombo->findText ( TableClass ) );
  ui->SubclassesCheckBox->setEnabled ( false );
  FillInfo ( TableClass );

  if ( UseTable )
  {
    setWindowTitle ( "Batch change - Table" );
    ui->FilterGroupBox->hide();
    ui->Label1->hide();
    ui->Label2->hide();
    ui->FirstStepLabel->hide();
    ui->SecondStepLabel->hide();
    ui->ThirdStepLabel->hide();
    ui->Line1->hide();
    ui->Line2->hide();
    ui->TitleLabel->setText ( "Batch Change of multiple objects - Table" );
    resize ( 540, 280 );
  }
}

void dbe::BatchChangeWidget::SetController()
{
  connect ( ui->ClassCombo, SIGNAL ( activated ( QString ) ), this,
            SLOT ( FillInfo ( const QString & ) ) );

  connect ( ui->ChangeButton, SIGNAL ( clicked() ), this, SLOT ( MakeChanges() ) );

  connect ( ui->MatchButton, SIGNAL ( clicked() ), this, SLOT ( FindMatching() ) );

  connect ( ui->NewRelationshipComboBox, SIGNAL ( activated ( QString ) ), this,
            SLOT ( EnableCheckBox ( QString ) ) );
  connect ( ui->CancelButton, SIGNAL ( clicked() ), this, SLOT ( close() ) );

  connect ( ui->RelationshipComboBox, SIGNAL ( activated ( int ) ), this,
            SLOT ( UpdateRelationshipFilter ( int ) ) );

  connect ( ui->NewRelationshipComboBox, SIGNAL ( activated ( int ) ), this,
            SLOT ( UpdateRelationshipNewValues ( int ) ) );

  MainWindow * mainwin = MainWindow::findthis();

  if ( mainwin != nullptr )
  {
    connect ( this, SIGNAL ( sig_batch_change_start() ), mainwin,
              SLOT ( slot_batch_change_start() ), Qt::UniqueConnection );
    connect ( this, SIGNAL ( sig_batch_change_stop(const QList<QPair<QString, QString>>&) ), mainwin,
              SLOT ( slot_batch_change_stop(const QList<QPair<QString, QString>>&) ),
              Qt::UniqueConnection );
  }
}

void dbe::BatchChangeWidget::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}

void dbe::BatchChangeWidget::filter ( std::vector<dref> & objs, const QString & cname )
{

  dunedaq::conffwk::class_t const & cinfo = dbe::config::api::info::onclass::definition (
                                         cname.toStdString(), false );
  std::vector<dunedaq::conffwk::attribute_t> const & attributes = cinfo.p_attributes;
  std::vector<dunedaq::conffwk::relationship_t> const & relations = cinfo.p_relationships;

  // Filtering UID
  std::vector<dref> filtered;

  if ( !ui->FilterUid->text().isEmpty() )
  {
    QRegExp uidfilter ( ui->FilterUid->text() );
    uidfilter.setPatternSyntax ( QRegExp::Wildcard );

    for ( dbe::dref const & it : objs )
    {

      if ( uidfilter.exactMatch ( QString::fromStdString ( it.ref().UID() ) ) )
      {
        filtered.push_back ( it );
      }
    }

    objs = filtered;
    filtered.clear();
  }

/// Filtering Attributes
  QRegExp attribute_filter ( ui->AttributeValueFilter->text() );
  attribute_filter.setPatternSyntax ( QRegExp::Wildcard );

  std::string selected_attribute_name = ui->AttributeComboBox->currentText().toStdString();

  std::vector<dunedaq::conffwk::attribute_t>::const_iterator attrdef = std::find_if (
                                                                    std::begin ( attributes ), std::end ( attributes ),
                                                                    [&selected_attribute_name] ( dunedaq::conffwk::attribute_t const & attr )
  {
    return attr.p_name == selected_attribute_name;
  } );

  if ( attrdef != std::end ( attributes ) )
  {
    for ( dbe::dref const & it : objs )
    {
      QStringList values
      { dbe::config::api::get::attribute::list<QStringList> ( it.ref(), *attrdef ) };

      for ( QString & v : values )
        if ( attribute_filter.exactMatch ( v ) )
        {
          filtered.push_back ( it );
          break;
        }
    }

  }

  objs = filtered;
  filtered.clear();

  // Filter on Relationship

  std::string selected_relation_name =
    ui->RelationshipComboBox->currentText().toStdString();

  std::vector<dunedaq::conffwk::relationship_t>::const_iterator relationdef = std::find_if (
                                                                           std::begin ( relations ), std::end ( relations ),
                                                                           [&selected_relation_name] ( dunedaq::conffwk::relationship_t const & rel )
  {
    return rel.p_name == selected_relation_name;
  } );

  if ( relationdef != std::end ( relations ) )
  {
    QRegExp relation_filter ( ui->RelationshipValueFilter->currentText() );
    relation_filter.setPatternSyntax ( QRegExp::Wildcard );

    for ( dbe::dref const & it : objs )
    {
      QStringList ValueList
      {
        dbegraph::linked::through::relation<QStringList> (
          it.ref(), *relationdef ) };

      if ( ValueList.isEmpty() and ( ui->RelationshipValueFilter->currentText() == "*"
                                     or ui->RelationshipValueFilter->currentText() == "" ) )
      {
        filtered.push_back ( it );
      }
      else
      {
        for ( QString const & v : ValueList )
          if ( relation_filter.exactMatch ( v ) )
          {
            filtered.push_back ( it );
            break;
          }
      }
    }

  }

  objs = filtered;
  filtered.clear();
}

void dbe::BatchChangeWidget::FillInfo ( const QString & Name )
{
  ui->AttributeBox->setEnabled ( false );
  ui->RelationshipBox->setEnabled ( false );
  ui->NewAttributeGroupBox->setEnabled ( false );
  ui->NewRelationshipGroupBox->setEnabled ( false );

  const dunedaq::conffwk::class_t & ClassInfo = dbe::config::api::info::onclass::definition (
                                             Name.toStdString(),
                                             false );
  const std::vector<dunedaq::conffwk::attribute_t> AttributeList = ClassInfo.p_attributes;
  const std::vector<dunedaq::conffwk::relationship_t> RelationshipList = ClassInfo
                                                                    .p_relationships;

  QStringList ValueList;

  for ( dunedaq::conffwk::attribute_t Attribute : AttributeList )
  {
    ValueList.append ( QString ( Attribute.p_name.c_str() ) );
  }

  ui->AttributeComboBox->clear();
  ui->NewAttributeComboBox->clear();
  ui->AttributeComboBox->addItems ( ValueList );
  ui->NewAttributeComboBox->addItems ( ValueList );

  if ( !AttributeList.empty() && !UseTable )
  {
    ui->AttributeBox->setEnabled ( true );
    ui->NewAttributeGroupBox->setEnabled ( true );
    ui->MatchButton->setEnabled ( true );
  }

  if ( !AttributeList.empty() && UseTable )
  {
    ui->NewAttributeGroupBox->setEnabled ( true );
  }

  ValueList.clear();

  for ( dunedaq::conffwk::relationship_t Relationship : RelationshipList )
  {
    ValueList.append ( QString ( Relationship.p_name.c_str() ) );
  }

  ui->RelationshipComboBox->clear();
  ui->NewRelationshipComboBox->clear();
  ui->RelationshipComboBox->addItems ( ValueList );
  ui->NewRelationshipComboBox->addItems ( ValueList );

  if ( !RelationshipList.empty() && !UseTable )
  {
    ui->RelationshipBox->setEnabled ( true );
    ui->NewRelationshipGroupBox->setEnabled ( true );
    ui->MatchButton->setEnabled ( true );
  }

  if ( !RelationshipList.empty() && UseTable )
  {
    ui->NewRelationshipGroupBox->setEnabled ( true );
  }

  if ( !UseTable )
  {
    UpdateRelationshipFilter ( 0 );
    UpdateRelationshipNewValues ( 0 );
    ui->RelationshipValueFilter->setCurrentIndex ( 0 );
    ui->RelationshipEdit->setCurrentIndex ( -1 );
  }
  else
  {
    UpdateRelationshipNewValues ( 0 );
    ui->RelationshipEdit->setCurrentIndex ( -1 );
  }

  if ( ClassChanged )
  {
    UseTable = false;
  }

  ClassChanged = true;
}

void dbe::BatchChangeWidget::MakeChanges()
{
  std::vector<dref> Objects;

  BOOST_SCOPE_EXIT(&Objects, this_)
  {
      QList<QPair<QString, QString>> objs;
      for(const auto& o : Objects) {
          objs.append(QPair<QString, QString>(QString::fromStdString(o.class_name()),
                                              QString::fromStdString(o.UID())));
      }

      this_->emit sig_batch_change_stop(objs);


      QApplication::restoreOverrideCursor();
      this_->ui->ChangeButton->setEnabled(true);
      confaccessor::ref().blockSignals(false);
  }
  BOOST_SCOPE_EXIT_END

  ui->ChangeButton->setEnabled(false);
  QApplication::setOverrideCursor(Qt::WaitCursor);
  confaccessor::ref().blockSignals(true);

  bool SubClasses = ui->SubclassesCheckBox->isChecked();
  QString ClassName = ui->ClassCombo->currentText();

  std::unique_ptr<dbe::interface::messenger::batch_guard> batchmode (
    interface::messenger::qt::batchmode() );

  emit sig_batch_change_start();

  if ( not UseTable )
  {
    std::vector<tref> const & tmprefs = dbe::config::api::info::onclass::objects (
                                          ClassName.toStdString(), SubClasses );

    std::copy ( tmprefs.begin(), tmprefs.end(), std::back_inserter ( Objects ) );

    if ( not Objects.empty() )
    {
      filter ( Objects, ClassName );
    }
  }
  else
  {
    Objects = TableObjects;
  }

  if ( Objects.empty() ) QMessageBox::information ( this, tr ( "Message" ),
                                                      tr ( "No objects match filter!" ) );
  else
  {
    const dunedaq::conffwk::class_t & ClassInfo = dbe::config::api::info::onclass::definition (
                                               ClassName.toStdString(), false );
    const std::vector<dunedaq::conffwk::attribute_t> AttributeList = ClassInfo.p_attributes;
    const std::vector<dunedaq::conffwk::relationship_t> RelationshipList = ClassInfo
                                                                      .p_relationships;
    dunedaq::conffwk::attribute_t AttributeChange;
    dunedaq::conffwk::relationship_t RelationshipChange;

    for ( dunedaq::conffwk::attribute_t i : AttributeList )
      if ( i.p_name == ui->NewAttributeComboBox->currentText().toStdString() )
      {
        AttributeChange = i;
        break;
      }

    for ( dunedaq::conffwk::relationship_t i : RelationshipList )
      if ( i.p_name == ui->NewRelationshipComboBox->currentText().toStdString() )
      {
        RelationshipChange = i;
        break;
      }

    QStringList AttributeChangeList;

    if ( !ui->AttributeEdit->text().isEmpty() ) AttributeChangeList
          << ui->AttributeEdit->text();

    for ( auto objects_iter = Objects.begin(); objects_iter != Objects.end(); ++objects_iter )
    {
      tref Object = objects_iter->ref();

      QStringList RelationshipChangeList;

      if ( ui->ChangeAtrributeCheckBox->isChecked() )
      {
        if ( !AttributeChangeList.isEmpty() )
        {
          dbe::config::api::set::attribute ( Object, AttributeChange, AttributeChangeList );
        }
      }

      if ( ui->ChangeRelationshipCheckBox->isChecked() )
      {
        if ( ui->AddCheckBox->isChecked() )
        {
          RelationshipChangeList =
            dbegraph::linked::through::relation<QStringList> (
              Object, RelationshipChange );
        }

        RelationshipChangeList << ui->RelationshipEdit->currentText();
        dbe::config::api::set::relation ( Object, RelationshipChange, RelationshipChangeList );
      }
    }
  }
}

void dbe::BatchChangeWidget::FindMatching()
{
  QString ClassName = ui->ClassCombo->currentText();
  bool SubClasses = ui->SubclassesCheckBox->isChecked();

  std::vector<tref> const & tmprefs = dbe::config::api::info::onclass::objects (
                                        ClassName.toStdString(), SubClasses );

  std::vector<dref> candidates;
  std::copy ( tmprefs.begin(), tmprefs.end(), std::back_inserter ( candidates ) );

  if ( !candidates.empty() )
  {
    filter ( candidates, ClassName );
  }

  if ( candidates.empty() )
  {
    QMessageBox::information ( this, tr ( "Message" ), tr ( "No objects match filter!" ) );
  }
  else
  {
    int NumberOfColumns = 1;
    bool AttributeEnabled = false;
    bool RelationshipEnabled = false;

    if ( ui->AttributeComboBox->isEnabled() )
    {
      AttributeEnabled = true;
      NumberOfColumns++;
    }

    if ( ui->RelationshipComboBox->isEnabled() )
    {
      RelationshipEnabled = true;
      NumberOfColumns++;
    }

    m_filter_table = std::unique_ptr<QTableWidget> ( new QTableWidget() );
    m_filter_table->setWindowTitle ( tr ( "Objects matching filter" ) );
    m_filter_table->setColumnCount ( NumberOfColumns );
    m_filter_table->setRowCount ( candidates.size() );

    for ( size_t i = 0; i < candidates.size(); i++ )
    {
      QTableWidgetItem * AttributeValue;
      QTableWidgetItem * RelationshipValue;

      QTableWidgetItem * ObjectUid = new QTableWidgetItem (
        tr ( "%1@%2" ).arg ( candidates.at ( i ).UID().c_str() ).arg (
          candidates.at ( i ).class_name().c_str() ) );
      m_filter_table->setItem ( i, 0, ObjectUid );

      if ( AttributeEnabled )
      {
        dunedaq::conffwk::attribute_t Attribute = dbe::config::api::info::attributematch (
                                               ui->AttributeComboBox->currentText(), ClassName );

        QStringList AttributeValueList
        {
          dbe::config::api::get::attribute::list<QStringList> ( candidates.at ( i ).ref(),
          Attribute ) };

        AttributeValue = new QTableWidgetItem ( AttributeValueList.join ( " " ) );
        m_filter_table->setItem ( i, 1, AttributeValue );
      }

      if ( RelationshipEnabled )
      {
        dunedaq::conffwk::relationship_t Relationship = config::api::info::relation::match (
                                                     ui->RelationshipComboBox->currentText(), ClassName );

        QStringList RelationshipList
        {
          dbegraph::linked::through::relation<QStringList> (
            candidates.at ( i ).ref(), Relationship ) };

        RelationshipValue = new QTableWidgetItem ( RelationshipList.join ( " " ) );
        m_filter_table->setItem ( i, 2, RelationshipValue );
      }
    }

    QStringList Headers;
    Headers << "Object Id";

    if ( AttributeEnabled )
    {
      Headers << ui->AttributeComboBox->currentText();
    }

    if ( RelationshipEnabled )
    {
      Headers << ui->RelationshipComboBox->currentText();
    }

    m_filter_table->setHorizontalHeaderLabels ( Headers );

    QHeaderView * HeaderView = m_filter_table->horizontalHeader();
    HeaderView->setStretchLastSection ( true );
    m_filter_table->resize ( 500, 300 );
    m_filter_table->show();
  }
}

void dbe::BatchChangeWidget::EnableCheckBox ( QString RelationshipName )
{
  QString ClassName = ui->ClassCombo->currentText();
  const dunedaq::conffwk::class_t & ClassInfo = dbe::config::api::info::onclass::definition (
                                             ClassName.toStdString(), false );
  std::vector<dunedaq::conffwk::relationship_t> RelationshipList = ClassInfo.p_relationships;

  for ( dunedaq::conffwk::relationship_t & i : RelationshipList )
  {
    if ( i.p_name == RelationshipName.toStdString() )
    {
      if ( ( i.p_cardinality == dunedaq::conffwk::zero_or_many ) || ( i.p_cardinality
                                                                 == dunedaq::conffwk::one_or_many ) )
      {
        ui->AddCheckBox->setEnabled ( true );
      }
      else
      {
        ui->AddCheckBox->setEnabled ( false );
      }

      break;
    }
  }
}

void dbe::BatchChangeWidget::UpdateRelationshipFilter ( int )
{
  ui->RelationshipValueFilter->clear();
  ui->RelationshipValueFilter->addItem ( "*" );

  QString ClassName = ui->ClassCombo->currentText();
  QString RelationshipName = ui->RelationshipComboBox->currentText();

  const dunedaq::conffwk::class_t & ClassInfo = dbe::config::api::info::onclass::definition (
                                             ClassName.toStdString(), false );
  std::vector<dunedaq::conffwk::relationship_t> RelationshipList = ClassInfo.p_relationships;

  if ( RelationshipList.size() != 0 )
  {
    dunedaq::conffwk::relationship_t ChosenRelationship;

    for ( dunedaq::conffwk::relationship_t & i : RelationshipList )
    {
      if ( i.p_name == RelationshipName.toStdString() )
      {
        ChosenRelationship = i;
        break;
      }
    }

    std::vector<tref> const & Objects = dbe::config::api::info::onclass::objects (
                                          ChosenRelationship.p_type );

    QStringList ObjectsUid;

    for ( tref const & Object : Objects )
    {
      ObjectsUid.append ( Object.UID().c_str() );
    }

    ui->RelationshipValueFilter->addItems ( ObjectsUid );
  }
}

void dbe::BatchChangeWidget::UpdateRelationshipNewValues ( int )
{
  ui->RelationshipEdit->clear();

  QString ClassName = ui->ClassCombo->currentText();
  QString RelationshipName = ui->NewRelationshipComboBox->currentText();

  const dunedaq::conffwk::class_t & ClassInfo = dbe::config::api::info::onclass::definition (
                                             ClassName.toStdString(), false );
  std::vector<dunedaq::conffwk::relationship_t> RelationshipList = ClassInfo.p_relationships;

  if ( RelationshipList.size() != 0 )
  {
    dunedaq::conffwk::relationship_t ChosenRelationship;

    for ( dunedaq::conffwk::relationship_t & i : RelationshipList )
    {
      if ( i.p_name == RelationshipName.toStdString() )
      {
        ChosenRelationship = i;
        break;
      }
    }

    std::vector<tref> const & Objects = dbe::config::api::info::onclass::objects (
                                          ChosenRelationship.p_type );

    QStringList ObjectsUid;

    for ( tref const & Object : Objects )
    {
      ObjectsUid.append ( Object.UID().c_str() );
    }

    ui->RelationshipEdit->addItems ( ObjectsUid );
  }
}
