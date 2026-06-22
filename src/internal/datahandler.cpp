// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

#include "dbe/datahandler.hpp"
#include  "dbe/treenode.hpp"
#include "dbe/dbcontroller.hpp"
#include "dbe/Conversion.hpp"

dbe::datahandler::datahandler()
  : root ( nullptr )
{
}

dbe::datahandler::~datahandler()
{
  delete root;
}

void dbe::datahandler::ResetData()
{
  delete root;
  root = nullptr;
}

void dbe::datahandler::FetchMore ( const treenode * ClassNode )
{
  emit FetchMoreData ( ClassNode );
}

dbe::treenode * dbe::datahandler::getnode ( std::string const & ClassName ) const
{
  return getnode ( QString::fromStdString ( ClassName ) );
}

dbe::treenode * dbe::datahandler::getnode ( const QString & ClassName ) const
{
  int current = 0;
  int const number_of_childs = root->ChildCount();

  for ( ;
        current != number_of_childs and root->GetChild ( current )->GetData ( 0 ).toString()
        != ClassName; ++current )
    ;

  return current == number_of_childs ? nullptr : root->GetChild ( current );
}

dbe::treenode * dbe::datahandler::getnode ( const std::string & ClassName,
                                            const std::string & ObjectName ) const
{
  return getnode ( QString::fromStdString ( ClassName ),
                   QString::fromStdString ( ObjectName ) );
}

dbe::treenode * dbe::datahandler::getnode ( const QString & ClassName,
                                            const QString & ObjectName ) const
{
  if ( dbe::treenode * classnode = getnode ( ClassName ) )
  {
    int current = 0;
    int const number_of_objects = classnode->ChildCount();

    for ( ;
          current < number_of_objects and classnode->GetChild ( current )->GetData ( 0 ).toString()
          != ObjectName; ++current )
      ;

    return current == number_of_objects ? nullptr : classnode->GetChild ( current );
  }

  return nullptr;
}

dbe::treenode * dbe::datahandler::getnode() const
{
  return root;
}

dbe::treenode * dbe::datahandler::findchild ( dbe::treenode * const top,
                                              QString const & name )
{
  QList<dbe::treenode *> childs = top->GetChildren();

  for ( dbe::treenode * child : childs )
  {
    if ( QString::fromStdString ( child->GetObject().UID() ) == name )
    {
      return child;
    }
  }

  return nullptr;
}
