#include <QMessageBox>
#include "OracleWidget.h"
#include "ui_OracleWidget.h"
#include "messenger.h"

dbe::OracleWidget::~OracleWidget() = default;

dbe::OracleWidget::OracleWidget ( QWidget * parent )
  : QWidget ( parent ),
    ui ( new Ui::OracleWidget )
{
  ui->setupUi ( this );
  SetController();
}

void dbe::OracleWidget::SetController()
{
  connect ( ui->RunButton, SIGNAL ( clicked() ), this, SLOT ( ProcessOracleCommand() ),
            Qt::UniqueConnection );
}

/**
 * Build the connection command from the arguments given to the widget and emit the signal such that
 * the connection can be made as needed from the appropriate window
 */
void dbe::OracleWidget::ProcessOracleCommand()
{
  QString connect_str = ui->connection_string_input->text();

  if ( connect_str.isEmpty() )
  {
    ERROR ( "Database information invalid", "Oracle connection string has not been set" );
    return;
  }

  QString schema_str = ui->schema_string_input->text();

  if ( schema_str.isEmpty() )
  {
    ERROR ( "Database information invalid", "Oracle working schema has not been set" );
    return;
  }

  QString version_str = ui->OracleLine->text();

  if ( version_str.isEmpty() )
  {
    ERROR ( "Database information invalid", "Oracle schema version has not been set" );
    return;
  }

  QString oracleCommand (
    ui->PluginLabel->text() + connect_str + ":" + schema_str + ":" + version_str );

  emit OpenOracleConfig ( oracleCommand );
}
