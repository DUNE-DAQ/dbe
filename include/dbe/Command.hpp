/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/Command.h to include/dbe/Command.hpp).
 */

#ifndef COMMAND_H
#define COMMAND_H

#include "dbe/Conversion.hpp"
#include "dbe/Exceptions.hpp"
#include "dbe/confaccessor.hpp"
#include "dbe/confobject_desc.hpp"
#include "dbe/confobject_extra.hpp"
#include "dbe/config_reference_copy.hpp"

#include "dbe/dbcontroller.hpp"

#include "conffwk/ConfigObject.hpp"
#include "conffwk/Schema.hpp"

#include <QUndoCommand>
#include <QUuid>

namespace dbe
{
namespace actions
{
/**
 * Class to uniformly encode a command's state.
 *
 * It can only be used by derived classes
 */
class state:
  public QUndoCommand
{
private:
  QUuid const uuid;
  mutable bool rewind;
  mutable bool forward;
  mutable bool invalid;

  bool canRedo() const;

  bool canUndo() const;

protected:
  state ( QUndoCommand * parent = nullptr, QUuid const & uuid = 0 );

public:
  bool isvalid() const;

  void toggle();

  void failed() const;
  void setundoable ( bool s = true ) const;
  void setredoable ( bool s = true ) const;

  bool undoable() const;

  bool redoable() const;

  void reload() const;

  QUuid const & source() const;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Class to encode basic interface for operations on objects
 */
class onobject:
  public state
{
public:
  onobject ( tref obj, QUndoCommand * parent = nullptr, QUuid const & uuid = 0 );

protected:
  tref checkedref() const;

private:
  dref this_object;
};
//------------------------------------------------------------------------------------------

namespace object
{

//------------------------------------------------------------------------------------------
class create:
  public state
{
public:
  create ( dbe::t_config_object_preimage const & img, QUuid const & src = 0,
           QUndoCommand * parent = 0 );

  void undo();
  void redo();
private:
  dbe::t_config_object_preimage this_object_key;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class remove:
  public onobject
{
public:
  remove ( tref item, QUuid const & uuid = 0, QUndoCommand * parent = 0 );
  ~remove();
  void redo();
  void undo();

private:
  gref this_remainder;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class rename:
  public onobject
{
public:
  rename ( tref object, std::string const & Id, QUuid const & src = 0,
           QUndoCommand * parent = nullptr );
  void redo();
  void undo();
private:
  std::string oldname;
  std::string newname;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class changerefs:
  public onobject
{
public:
  changerefs ( tref object, dunedaq::conffwk::relationship_t const & relation,
               std::vector<std::string> const & object_names_tolink, QUuid const & src = 0,
               QUndoCommand * Parent = nullptr );
  void redo();
  void undo();
private:
  dunedaq::conffwk::relationship_t this_relation;
  std::vector<tref> this_current_neighbors;
  std::vector<tref> this_target_neighbors;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class move:
  public onobject
{
public:
  move ( tref ObjectChanged, std::string const & File, QUuid const & src = 0,
         QUndoCommand * parent = nullptr );
  ~move();
  void redo();
  void undo();
private:
  std::string source_file;
  std::string destination_file;
};
//------------------------------------------------------------------------------------------
}// end namespace object

namespace file
{
/*
 * This namespace handles operations related to files , but not those of object in files
 */
//------------------------------------------------------------------------------------------
class add:
  public state
{
public:
  add ( std::string const & db_file, std::string const & include_file, QUuid const & src = 0,
        QUndoCommand * parent = 0 );
  void redo();
  void undo();
private:
  std::string m_db_file;
  std::string m_include_file;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class remove:
  public state
{
public:
  remove ( std::string & db_file, std::string & include_file, QUuid const & src = 0,
           QUndoCommand * parent = 0 );
  void redo();
  void undo();
private:
  std::string m_db_file;
  std::string m_include_file;
};
//------------------------------------------------------------------------------------------
}// end namespace file

} //end namespace actions
} // end namespace dbe
#endif // COMMAND_H
