#ifndef BUILDINGBLOCKEDITORS_H
#define BUILDINGBLOCKEDITORS_H

#include "conffwk/Schema.hpp"

#include "ui_RelationshipWidgetForm.h"
#include "ui_StringAttributeWidgetForm.h"
#include "ui_NumericAttributeWidgetForm.h"
#include "ui_EditCombo.h"

#include <QWidget>
#include <QStatusBar>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QMenu>
#include <QAction>
#include <QSortFilterProxyModel>

#include<memory>

namespace dbe
{
//------------------------------------------------------------------------------------------------

class editor_data_state
{

public:
  virtual ~editor_data_state();

  bool is_valid() const
  {
    return m_valid;
  }

  bool must_not_be_null() const
  {
    return m_notnull;
  }

  bool must_be_set() const
  {
    return m_obligatory;
  }

protected:
  explicit editor_data_state ( bool isvalid, bool isnotnull, bool isobligatory )
    : m_valid ( isvalid ),
      m_notnull ( isnotnull ),
      m_obligatory ( isobligatory )
  {}

  bool m_valid;
  bool m_notnull;
  bool m_obligatory;

};

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
/**
 * Base class for editor input blocks
 */

template<typename T>

class editor_data: public editor_data_state
{

public:
  typedef T t_virtue;

  explicit editor_data ( T const & virtue )
    : editor_data_state ( true, virtue.p_is_not_null, true ),
      this_virtue ( virtue )
  {}

  virtual ~editor_data();

  t_virtue const & get() const
  {
    return this_virtue;
  }

  void set_obligatory ( bool is )
  {
    m_obligatory = is;
  }

  void set_valid ( bool const is )
  {
    m_valid = is;
  }

  void set_not_null ( bool const is )
  {
    m_notnull = is;
  }

private:
  t_virtue this_virtue;
};

//------------------------------------------------------------------------------------------------

namespace widgets
{
namespace editors
{
//------------------------------------------------------------------------------------------------

class base: public QWidget
{
  Q_OBJECT

public:
  virtual void SetEditor() = 0;

  template<typename T = editor_data_state> std::shared_ptr<T> dataeditor()
  {
    return std::dynamic_pointer_cast<T> ( p_data_editor );
  }

  virtual void setdata ( QStringList const & );
  virtual void setdefaults ( QString const & );

  void setchanged ( bool );
  bool ischanged() const;

  virtual QStringList getdata();

  const QString c_input_placeholder{"Add a new value"};
  const QString c_no_object_placeholder{"No Object currently set"};
protected:
  base ( std::shared_ptr<editor_data_state> editordata, QWidget * parent =
           nullptr, bool owned = false );

  virtual void buildtooltip() = 0;
  virtual void closeEvent ( QCloseEvent * Event );

  std::shared_ptr<editor_data_state> p_data_editor;

  QString this_defaults;
  QStringList this_data;

  bool this_is_owned;
  bool this_value_changed;
  bool this_initial_load;

signals:
  void signal_force_close();
  void signal_edit_end();
  void signal_internal_value_change();
  void signal_value_change();

public slots:
  virtual void slot_set_initial_loaded();

};

//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------

class relation: public base, private dbe::Ui::RelationshipForm
{
  Q_OBJECT

public:

  typedef dunedaq::conffwk::relationship_t t_virtue;
  typedef editor_data<t_virtue> t_build_block_editor;

  relation ( t_virtue const & relation, QWidget * parent = nullptr,
             bool owned = false, bool readonly=false );

  bool GetIsMultiValue() const;

  bool eventFilter ( QObject * Target, QEvent * Event );

  void SetEditor() override;

private:

  void SetController();
  void SetFirstItem();

  void buildtooltip() override;

  void CreateObjectEditor ( const std::string& objectID );

  std::shared_ptr<t_build_block_editor> p_base_data_editor;

  bool IsMultiValue;
  bool m_readonly;

  QStatusBar * StatusBar;
  QStringList CurrentDataList;
  QMenu * ContextMenu;
  QAction * RemoveAction;
  QAction * MoveTop;
  QAction * MoveBottom;
  QAction * MoveUp;
  QAction * MoveDown;
  QAction * EditAction;
  QListWidgetItem * FirstItem;
  QListWidgetItem * CurrentItem;

private slots:
  void FetchData();
  void EndSignal();
  void AddToDataList ( const QString & DataValue );
  void RemoveFromDataList();
  void UpdateActions();
  void closeEvent ( QCloseEvent * Event );
  void DataWasFetched ( QStringList ListOfObjects );
  void CreateObjectEditor ( QListWidgetItem * Item );
  void CustomContextMenuRequested ( const QPoint & pos );
  void RemoveSlot();
  void MoveTopSlot();
  void MoveBottomSlot();
  void MoveUpSlot();
  void MoveDownSlot();
  void EditSlot();
  void DummyMovement();
  void EditItemEntered ( QListWidgetItem * Item );

signals:
  void FetchDataDone ( QStringList Data );
  void LoadedInitials();
};

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------

class stringattr: public base, private Ui::StringAttributeWidgetForm
{
  Q_OBJECT

public:
  typedef dunedaq::conffwk::attribute_t t_virtue;
  typedef editor_data<t_virtue> t_build_block_editor;

  stringattr ( t_virtue const & attr, QWidget * parent = nullptr,
               bool owned = false, bool readonly=false );
  ~stringattr();

  void SetEditor() override;

  QTextEdit * GetLineEdit() const;

  void SetNullCheck ( bool Check );
  void SetMultiCheck ( bool Multi );
  void SetCheckDefaults ( bool Default );
  void SetFocusOnLine();

  bool eventFilter ( QObject * , QEvent * );
  void ClearText();

private:
  void buildtooltip() override;
  void closeEvent ( QCloseEvent *  );
  void SetController();
  void setdefaults ( const QString &  );
  void ShowPopupButton();
  void HidePopupButton();

  std::shared_ptr<t_build_block_editor> m_base_data_editor;

  bool m_readonly;
  QString DefaultValue;
  QPushButton * PopUpButton;
  QDialog * Dialog;
  QPushButton * OkButtonDialog;
  QPlainTextEdit * TextEditDialog;

signals:
	void signal_data_input_complete();

private slots:
  void UpdateActions ( );
  void AddToDataList();
  void ShowDialog();
  void UpdateFromTextEdit();
  void ToogleTextEditOkButton();
};

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------

class numericattr: public base, private Ui::NumericAttributeWidgetForm
{
  Q_OBJECT

public:
  typedef dunedaq::conffwk::attribute_t t_virtue;
  typedef editor_data<t_virtue> t_build_block_editor;

  numericattr ( t_virtue const & attr, QWidget * parent = nullptr,
                bool owned = false, bool readonly=false );

  void SetEditor() override;

  QLineEdit * GetLineEdit() const;

  virtual void setdefaults ( const QString & ValueDefault );

private:
  void buildtooltip() override;
  void closeEvent ( QCloseEvent * Event );
  void SetController();
  void ShowWarning ( QString Format = "", QString Range = "" );
  bool ValidateIntegerValue ( QString const & );
  bool ValidateFloatValue ( QString const & );
  bool checkRange( QString const & );

  std::shared_ptr<t_build_block_editor> this_base_data_editor;

  bool m_readonly;
  int this_base;
  int this_native_base;

signals:
  void signal_value_duplicated();

private slots:
  void checkIfDuplicated();
  void AddToList();
  void ChangeFormat ( int i );
  void UpdateActions ( QString Dummy );
  void ChangeFormatDec();
  void ChangeFormatHex();
  void ChangeFormatOct();
};

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------

class combo: public base, public Ui::EditCombo
{
  Q_OBJECT

public:
  typedef dunedaq::conffwk::attribute_t t_virtue;
  typedef editor_data<t_virtue> t_build_block_editor;

  combo ( t_virtue const & attr, QWidget * parent = nullptr,
          bool owned = false, bool readonly=false );

  void SetEditor() override;

  void SetData ( QStringList const & );
  void SetValidatorData ( QStringList const & Data, bool AcceptNoMatch = false );

  void setdata ( QStringList const & );

  QStringList getdata() override;

private:
  void buildtooltip() override;
  bool eventFilter ( QObject *, QEvent * );
  void wheelEvent ( QWheelEvent * );
  void SetController();

  std::shared_ptr<t_build_block_editor> m_base_data_editor;
  bool m_readonly;
private slots:
  void TryValidate ( QString );
  void ChangeDetected ( QString const & );
  void CheckDefaults ( int );
  bool CompareDefaults();
};

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------

class multiattr: public base
{
  Q_OBJECT

public:
  typedef dunedaq::conffwk::attribute_t t_virtue;
  typedef editor_data<t_virtue> t_build_block_editor;

  multiattr ( t_virtue const & attr, QWidget * parent = nullptr,
              bool owned = false, bool readonly=false );

  void SetEditor() override;

private:
  void buildtooltip();
  void closeEvent ( QCloseEvent * Event );
  void SetStatusBar();
  bool eventFilter ( QObject * Target, QEvent * Event );

  std::shared_ptr<t_build_block_editor> m_base_data_editor;

  bool m_readonly;
  QStatusBar * StatusBar;
  QPushButton * OkButton;
  QPushButton * RemoveButton;
  QListWidget * ListWidget;
  base * BaseWidget;
  QMenu * ContextMenu;
  QAction * RemoveAction;

private slots:
  void AddToDataList ( const QString & Data );
  void RemoveFromDataList();
  void UpdateActions();
  void LineValueChanged();
  void LineValueChanged ( QListWidgetItem * p );
  void ListOrderChange ( const QModelIndexList & IndexList );
  void EndSignal();
  void CustomContextMenuRequested ( const QPoint & pos );
  void RemoveSlot();
};

//------------------------------------------------------------------------------------------------
} // end namespace editors
} // end widgets


}//end namespace dbe

#endif // BUILDINGBLOCKEDITORS_H
