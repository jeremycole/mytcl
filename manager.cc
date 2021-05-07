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

#include <stdio.h>

#include "manager.h"

// ---------------------------------------------------------
Manager::Manager() {
  nConnections=0;
  for(int i=0; i < MAXCONNS; i++) {
    conns[i] = NULL;
  };
};

// ---------------------------------------------------------
int Manager::findFreeConn() {
  int ret=-1;

  for(int i=0; (i < MAXCONNS)&&(ret == -1); i++) {
    if(conns[i] == NULL) ret = i;
  };
  return(ret);
};

// ---------------------------------------------------------
char *Manager::getErrorMsg() {
  return errmsg;
};

// ---------------------------------------------------------
int Manager::connect(int argc, char **argv) {
  int ret;

  if(nConnections >= MAXCONNS) {
    ret = -1;
  } else {
    Connection *newconn = new Connection(argc, argv);
    if(!newconn->isconnected()) {
      errmsg = newconn->getErrorMsg();
      delete newconn;
      ret = -1;
    } else {
      int connNum = findFreeConn();
      conns[connNum] = newconn;
      nConnections++;
      ret = connNum;
    };
  };
  return(ret);
};



// ---------------------------------------------------------
int Manager::disconnect(int c) {
  int ret;

  if(!conns[c]) {
    ret = 0;
  } else {
    delete conns[c];
    conns[c] = NULL;
    nConnections--;
    ret = 1;
  };
  return(ret);
};
