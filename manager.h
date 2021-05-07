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

#include "connection.h"

#define MAXCONNS 10

class Manager {
  private:
    Connection *conns[MAXCONNS];
    int nConnections;

    // Find the next unused connection slot (the next available slot
    // in 'conns' array).
    int findFreeConn();

    // The last error message
    char *errmsg;

  public:
    Manager();

    int connect(int argc, char **argv);
    int disconnect(int c);

    int inUse(int i) { return(conns[i]?1:0); };

    Connection *connection(int n) { return((n >= MAXCONNS)?NULL:conns[n]); };

    // Get the error message, if there was one, for the last message
    char *getErrorMsg();

};
