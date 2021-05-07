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

#include <tcl.h>
#include <string.h>
#include "connection.h"

// -------------------------------------------------------------
Connection::Connection() {
  for(int i=0; i < maxResults; i++) {
    results[i] = NULL;
  };
  connect(0,NULL);
};

// -------------------------------------------------------------
Connection::Connection(int argc, char** argv) {
  for(int i=0; i < maxResults; i++) {
    results[i] = NULL;
  };
  mysql = NULL;
  connect(argc, argv);
};

// -------------------------------------------------------------
char *Connection::getErrorMsg() {
  char *msg = (char *)mysql_error(mysql);
  return((*msg == '\0')?errormsg:msg);
}

// -------------------------------------------------------------
// Return 0 in case of error, 1 otherwise
int Connection::connect(int argc, char **argv) {
  int ret;
  char *host = NULL;
  char *user = NULL;
  char *pass = NULL;

  if(argc > 0 && argv[0]) { host = argv[0]; }
  if(argc > 1 && argv[1]) { user = argv[1]; }
  if(argc > 2 && argv[2]) { pass = argv[2]; }

  mysql = mysql_init(mysql);
  if(!(mysql_real_connect(mysql, host, user, pass, NULL, 0, NULL, 0))) {
    connected = 0;
    ret = -1;
  } else {
    connected = 1;
    ret = 1;
  };
  return(ret);
};

// -------------------------------------------------------------
int Connection::selectdb(char *dbname) { 
  int ret;

  if (!connected || !dbname) {
    ret = 0;
  } else {
    ret = (mysql_select_db(mysql, dbname))?0:1;
  };
  return(ret);
};

// -------------------------------------------------------------
int Connection::exec(char *cmd) {
  return((mysql_real_query(mysql, cmd, strlen(cmd)))?0:1);
};

// -------------------------------------------------------------
int Connection::numrows(int resHandle=0) {
  return((!results[resHandle])?0:mysql_num_rows(results[resHandle]));
};

// -------------------------------------------------------------
int Connection::affectedrows() {
  return(mysql_affected_rows(mysql));
};

// -------------------------------------------------------------
int Connection::insertid() {
  return(mysql_insert_id(mysql));
};

// -------------------------------------------------------------
int Connection::getFreeResultHandle() {
  int ret=-1;

  for (int i=0; (i < maxResults)&&(ret==-1); i++) {
    if (!results[i]) { ret = i; };
  };

  return(ret);
};

// -------------------------------------------------------------
int Connection::query(char *cmd) {
  int ret;
  int resHandle = getFreeResultHandle();
  MYSQL_RES *res;

  if(resHandle < 0) {
    // No result handles left open
    sprintf(errormsg, "Too many pending results, maximum %d allowed.\n", maxResults);
    ret = -1;
  } else {
    if(mysql_real_query(mysql, cmd, strlen(cmd))) { 
      ret = -2;
    } else {
      if(!(res = mysql_store_result(mysql))) { 
        return -3;
      } else {
        results[resHandle] = res;
        ret = resHandle;
      };
    };
  };
  return(ret);
};

// -------------------------------------------------------------
void Connection::endquery(int resHandle=0) {
  if(results[resHandle]) {
    mysql_free_result(results[resHandle]);
    results[resHandle] = NULL;
  };
};


// -------------------------------------------------------------
// Note: A new Row is allocated. Has to be freed by calling
// party.
Row *Connection::fetchrow(int resHandle=0) {
  Row *ret;

  if (!results[resHandle]) {
    sprintf(errormsg, "Result handle %d not in use.\n", resHandle);
    ret = NULL;
  } else {
    MYSQL_ROW row;
    MYSQL_RES *res = results[resHandle];
    row = mysql_fetch_row(res);
    if(!row) {
      ret = NULL;
    } else {
      Row *srow = new Row(mysql_num_fields(res));
      for(unsigned int i=0 ; i < mysql_num_fields(res); i++) {
        srow->setColumn(i, row[i]);
      };
      ret = srow;
    };
  };
  return(ret);
};

// -------------------------------------------------------------
Connection::~Connection() {
  for(int i=0; i < maxResults ; i++) {
    if(results[i]) endquery(i);
  };
  mysql_close(mysql);
};
