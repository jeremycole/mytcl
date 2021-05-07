/*
    MyTCL, package to integrate TCL with MySQL databases
    Copyright (C) 2003  Jeremy Cole

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <string.h>
#include <tcl.h>

#include "manager.h"

#define MYTCL_VERSION	"MyTCL version 0.85"

#define HANDLE_PREFIX 	"sql"
#define RESULT_PREFIX	"res"

// -------------------------------------------------------------
// Convert a tcl style connection to an integer
// returns -1 on format error,
int stripPrefix(char *txt, const char* prefix) {
  unsigned int pLen = strlen(prefix);
  return(((strlen(txt) <= pLen) || (strncmp(txt, prefix, pLen)!=0))?-1:atoi(txt+pLen));
};

// -------------------------------------------------------------
int sql_selectdb(Tcl_Interp *interp, Connection *conn, char *dbname) {
  int ret;

  if (conn->selectdb(dbname)) {
    Tcl_SetResult(interp, dbname, TCL_VOLATILE);
    ret = TCL_OK;
  } else {
    Tcl_SetResult(interp, conn->getErrorMsg(), TCL_VOLATILE);
    ret = TCL_ERROR;
  };
  return(ret);
};

// -------------------------------------------------------------
int sql_exec(Tcl_Interp *interp, Connection *conn, char *cmd) {
  int ret;

  if(cmd != NULL) {
    if (conn->exec(cmd)) {
      ret = TCL_OK;
    } else {
      Tcl_SetResult(interp, conn->getErrorMsg(), TCL_VOLATILE);
      ret = TCL_ERROR;
    };
  } else {
    Tcl_AppendResult(interp, "Usage: sql exec handle command", NULL);
    ret = TCL_ERROR;
  };
  return(ret);
};

int sql_escape(Tcl_Interp *interp, char *str) {
  char *escstr;

  escstr = (char *)malloc(strlen(str)*2+1);
  mysql_escape_string(escstr, str, strlen(str));
  Tcl_SetResult(interp, escstr, TCL_VOLATILE);
  free(escstr);

  return(TCL_OK);
};

// -------------------------------------------------------------
int sql_isconnected(Tcl_Interp *interp, Connection *conn) {
  char ret[2];
  strcpy(ret, (conn->isconnected())?"1":"0");
  Tcl_SetResult(interp, ret, TCL_STATIC);
  return(TCL_OK);
};

// -------------------------------------------------------------
int sql_disconnect(Tcl_Interp *interp, Manager *mgr, int connid) {
  if(interp); // Suppress unused parameter warning
  return((mgr->disconnect(connid))?TCL_OK:TCL_ERROR);
};

// -------------------------------------------------------------
int sql_query(Tcl_Interp *interp, Connection *conn, char *cmd) {
  int ret;
  int handle = -1;
  char tmp[32];

  if(cmd != NULL) {
    if((handle = conn->query(cmd)) < 0) {
      Tcl_SetResult(interp, conn->getErrorMsg(), TCL_VOLATILE);
      ret = TCL_ERROR;
    } else {
      sprintf(tmp, "%s%d", RESULT_PREFIX, handle);
      Tcl_SetResult(interp, tmp, TCL_VOLATILE);
      ret = TCL_OK;
    };
  } else {
    Tcl_SetResult(interp, "Usage: sql query handle command", TCL_STATIC);
    ret = TCL_ERROR;
  };
  return(ret);
};

// -------------------------------------------------------------
int sql_endquery(Tcl_Interp *interp, Connection *conn, char *handle) {
  int resHandle = 0;

  if(interp); // Suppress unused parameter warning

  if(handle) {
    resHandle = stripPrefix(handle, RESULT_PREFIX);
  };
  conn->endquery(resHandle);
  return(TCL_OK);
};

// -------------------------------------------------------------
int sql_numrows(Tcl_Interp *interp, Connection *conn, char *handle) {
  int resHandle = 0;
  int nrows = 0;
  char retval[20];

  if(handle) {
    resHandle = stripPrefix(handle, RESULT_PREFIX);
  };

  nrows = conn->numrows(resHandle);

  sprintf(retval, "%d", nrows);
  Tcl_SetResult(interp, retval, TCL_VOLATILE);
  return(TCL_OK);
};

// -------------------------------------------------------------
int sql_affectedrows(Tcl_Interp *interp, Connection *conn) {
  int nrows = conn->affectedrows();
  char retval[20];

  sprintf(retval, "%d", nrows);
  Tcl_SetResult(interp, retval, TCL_VOLATILE);
  return(TCL_OK);
};

// -------------------------------------------------------------
int sql_insertid(Tcl_Interp *interp, Connection *conn) {
  int id = conn->insertid();
  char retval[20];

  sprintf(retval, "%d", id);
  Tcl_SetResult(interp, retval, TCL_VOLATILE);
  return(TCL_OK);
};

// -------------------------------------------------------------
int sql_fetchrow(Tcl_Interp *interp, Connection *conn, char *handle) {
  int ret;
  int resHandle = 0;

  if(handle) {
    resHandle = stripPrefix(handle, RESULT_PREFIX);
  };

  if(resHandle < 0) {
    Tcl_SetResult(interp, "Invalid result handle.", TCL_VOLATILE);
    ret = TCL_ERROR;
  } else {

    Row *row;
    if((row = conn->fetchrow(resHandle)) == NULL) {
      // No data to fetch.
      Tcl_ResetResult(interp);
      ret = TCL_OK;
    } else {

      for(int i=0; i < row->numColumns(); i++) {
        Tcl_AppendElement(interp, row->getColumn(i));
      };
      delete row;
      ret = TCL_OK;
    };
  };
  return(ret);
};

// -------------------------------------------------------------
int sql_version(Tcl_Interp *interp) {
  Tcl_SetResult(interp, MYTCL_VERSION, TCL_STATIC);

  return(TCL_OK);
};

// -------------------------------------------------------------
//
int sql_command(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv) {
  int ret;

  if(argc == 1) {
    Tcl_SetResult(interp, "Usage: sql command ?handle?", TCL_STATIC);
    return TCL_ERROR;
  } else {

    // Get a pointer to the sample object from the clientData:
    Manager *mgr = (Manager *)clientData;
    ret = TCL_OK;

    int c = -1;

    // -----------------------------------
    if(strcmp(argv[1], "connect")==0) {
      c = mgr->connect(argc-2, (char **)argv+2);
      if(c < 0) {
        char *basemsg = "Unable to Connect: ";
        char *errmsg = mgr->getErrorMsg();
        char *msg = Tcl_Alloc(strlen(errmsg)+strlen(basemsg));
        strcpy(msg, basemsg);
        strcat(msg, errmsg);
        Tcl_SetResult(interp, msg, TCL_DYNAMIC);
        ret = TCL_ERROR;
      } else {
        char *han = Tcl_Alloc(16);
        sprintf(han, "%s%d", HANDLE_PREFIX, c);
        Tcl_SetResult(interp, han, TCL_DYNAMIC);
        ret = TCL_OK;
      };

    } else if(strcmp(argv[1], "escape")==0) {
      if(!(argc < 3)) {
        ret = sql_escape(interp, (char *)argv[2]);
      } else {
        Tcl_SetResult(interp, "Usage: sql escape string", TCL_STATIC);
        ret = TCL_ERROR;
      };
    } else if(strcmp(argv[1], "version")==0) {
      ret = sql_version(interp);
    } else {

      // Every other command needs a handle. Get it.
      int connid = -1;
      if(argc <= 2) {
        Tcl_SetResult(interp, "Usage: sql command ?handle?", TCL_STATIC);
        ret = TCL_ERROR;
      } else if((connid = stripPrefix((char *)argv[2], HANDLE_PREFIX)) < 0) {
        Tcl_AppendResult(interp, "mytcl: Invalid handle: ", argv[2], ".", NULL);
        ret = TCL_ERROR;
      } else if(!mgr->inUse(connid)) {
        // This connection is not currently being used
        Tcl_AppendResult(interp, "mytcl: Not connected on handle ", argv[2], ".", NULL);
        ret = TCL_ERROR;
      } else {

        Connection *conn = mgr->connection(connid);
        // take care of the command:
        if(strcmp(argv[1], "exec") == 0) {
          ret = sql_exec(interp, conn, (char *)argv[3]);
        } else if(strcmp(argv[1], "query") == 0) {
          ret = sql_query(interp, conn, (char *)argv[3]);
        } else if(strcmp(argv[1], "endquery") == 0) {
          ret = sql_endquery(interp, conn, (char *)argv[3]);
        } else if(strcmp(argv[1], "fetchrow") == 0) {
          ret = sql_fetchrow(interp, conn, (char *)argv[3]);
        } else if(strcmp(argv[1], "numrows") == 0) {
          ret = sql_numrows(interp, conn, (char *)argv[3]);
        } else if(strcmp(argv[1], "affectedrows") == 0) {
          ret = sql_affectedrows(interp, conn);
        } else if(strcmp(argv[1], "insertid") == 0) {
          ret = sql_insertid(interp, conn);
        } else if(strcmp(argv[1], "disconnect") == 0) {
          ret = sql_disconnect(interp, mgr, connid);
        } else if(strcmp(argv[1], "selectdb")==0) {
          ret = sql_selectdb(interp, conn, (char *)argv[3]);
        } else if(strcmp(argv[1], "isconnected")==0) {
          ret = sql_isconnected(interp, conn);
        } else {
          Tcl_AppendResult(interp, "mytcl: Unknown command: ", argv[1], ".", NULL);
          ret = TCL_ERROR;
        };
      };
    };
  };
  return ret;
};

// -------------------------------------------------------------
// It's necessary to declare the Init procedure as extern C to make
// tcl happy.
//
extern "C" {
  int Mytcl_Init(Tcl_Interp *interp);
}

// -------------------------------------------------------------
// The initialization function. Tcl calls this function when you
// load the package. Its name must be the name of the package, with
// the first letter capitalized, the rest lower, and '_Init' appended
// to the end of it.
//
int Mytcl_Init(Tcl_Interp *interp) {
  Manager *s = new Manager();

  Tcl_CreateCommand(interp, "sql", sql_command, (ClientData)s, (Tcl_CmdDeleteProc *)NULL);
       
  return((Tcl_PkgProvide(interp, "Mytcl", "1.0") == TCL_ERROR)?TCL_ERROR:TCL_OK);
};
