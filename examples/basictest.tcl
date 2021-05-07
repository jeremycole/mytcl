#!/usr/bin/tclsh
#    Edit this --^
#
# This example will connect to the database, run a simple query and print
# out the results in the form of a TCL list.
#

package require mytcl

set sql [sql connect "localhost" "root" "blah"]
#sql selectdb $sql "test"

set res [sql query $sql "select now(), version()"]
while {[set row [sql fetchrow $sql $res]] != ""} {
  puts $row
}
sql endquery $sql $res

sql disconnect $sql
