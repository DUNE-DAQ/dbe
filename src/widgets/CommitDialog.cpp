/// Including DBE
#include "dbe/CommitDialog.hpp"

dbe::CommitDialog::~CommitDialog() = default;

dbe::CommitDialog::CommitDialog ( QWidget * parent )
  : QDialog ( parent ),
    ui ( new Ui::CommitDialog )
{
  /// Setting Ui
  setupUi ( this );

  /// Setting initial values
  setModal ( true );
  setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
  setToolTip ( "Enter a commit message (optional) and press the ok button." );
  show();

  /// Setting Controller
  SetController();
}

void dbe::CommitDialog::SetController()
{
  connect ( OkButton, SIGNAL ( clicked() ), this, SLOT ( OkCommitMessage() ),
            Qt::UniqueConnection );
  connect ( CancelButton, SIGNAL ( clicked() ), this, SLOT ( CancelCommitMessage() ),
            Qt::UniqueConnection );
}

QString dbe::CommitDialog::GetCommitMessage() const
{
  return CommitMessageLine->text();
}

void dbe::CommitDialog::CommitMessageEdited ()
{
  if ( !CommitMessageLine->text().isEmpty() )
  {
    OkButton->setEnabled ( true );
  }
  else
  {
    OkButton->setEnabled ( false );
  }
}

void dbe::CommitDialog::OkCommitMessage()
{
  accept();
}

void dbe::CommitDialog::CancelCommitMessage()
{
  reject();
}
