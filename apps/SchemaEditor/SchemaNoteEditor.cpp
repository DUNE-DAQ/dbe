

#include "dbe/SchemaNoteEditor.hpp"
#include "dbe/SchemaGraphicNote.hpp"
#include "ui_SchemaNoteEditor.h"

namespace dbse {
  SchemaNoteEditor::SchemaNoteEditor(SchemaGraphicNote* note, QWidget* parent)
    : QDialog(parent), m_note(note),  m_ui(new Ui::SchemaNoteEditor) {

    m_ui->setupUi(this);
    m_ui->text->setPlainText(m_note->text());
    m_ui->text->setFocus();
    connect(m_ui->buttons, SIGNAL(accepted()), this, SLOT(update_text()));
    connect(m_ui->buttons, SIGNAL(rejected()), this, SLOT(cancel_note()));
  }

  SchemaNoteEditor::~SchemaNoteEditor() {
    delete m_ui;
  }

  void SchemaNoteEditor::cancel_note() {
    emit cancelled(m_note);
  }

  void SchemaNoteEditor::update_text() {
    emit note_accepted(m_note);
    m_note->update_note(m_ui->text->toPlainText());
  }
} //namespace dbse
