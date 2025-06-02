#ifndef SCHEMANOTEEDITOR_HPP
#define SCHEMANOTEEDITOR_HPP
#include <QDialog>

#include "dbe/SchemaGraphicNote.hpp"

namespace dbse {
  namespace Ui {
    class SchemaNoteEditor;
  } // namespace Ui

  class SchemaNoteEditor : public QDialog {
    Q_OBJECT
  public:
    SchemaNoteEditor(SchemaGraphicNote* note, QWidget* parent=0);
    ~SchemaNoteEditor();
  signals:
    void note_accepted(SchemaGraphicNote* note);
    void cancelled(SchemaGraphicNote* note);
  private:
    SchemaGraphicNote* m_note;
    Ui::SchemaNoteEditor* m_ui;
  private slots:
    void update_text();
    void cancel_note();
  };
} //namespace dbse
#endif // SCHEMANOTEEDITOR_HPP
