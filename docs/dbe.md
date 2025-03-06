# The OKS database editor: `dbe_main`

## Prerequisite
 Before running either schemaeditor or dbe you must load the dbe spack
package with `spack load dbe`. This can have unwanted side effects
like running the wrong version of Python due to spack messing with
your PATH and LD_LIBRARY_PATH. To avoid this, keep your editing
sessions in a different window to your normal development or create an
alias /shell fucntion like:

```
function dbe_main () 
{ 
    bash -c "spack load dbe; command dbe_main $@"
}

```

## Starting the editor

 Just running the command `dbe_main` will bring up the database editor
 with no database loaded. You can then opena an existing database or
 create a new database frm the items on the File menu or the
 toolbar. If you want to specify an existing database on the command
 line, you must include the `-f` option before the name of the
 file. File names not beginning with '/' are taken to be relative to
 first the current directory, then each member of the list in
 `DUNEDAQ_DB_PATH` until a match is found.

## Navigating with the `Class view`

The `Class View` is a dockable widget originally on the left of the
main window. The`Class View` displays a list of class names and the
number of objects of that class that exist in the database. The items
in the `Class View` can be selected in the normal way with either the
mouse or the keyboard (arrow keys move up and down and open/close
lists of objects, alphabetic keys move to the first/next item starting
with that character). Activating a class name will display all
instances of the class in the current `Table View`. Opening the class
name by selecting the triangle to the left or with the right arrow
will expand the list of instances in the `Tree view`. Activating an
instance in the `Tree view` will open the `Object Editor` to edit that
instance.

![Class View widget](class-view.png)
Tooltips show the descriptions of the classes (assuming they have one).

To filter the list of classes to see just a subset that you are
interested in, there is a text entry field at the bottom of the `Class
View` where you can enter a regular expression to be matched against
the class names.

## Using the `Table view`

