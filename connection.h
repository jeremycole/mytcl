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

#include <mysql.h>

const int maxResults = 16;

class Row {
  private:
    char **columns;
    int  nColumns;
    int  capacity;

  public:
    Row(int c) { nColumns = 0 ; capacity = c; columns = new char*[capacity]; }

    // ---------
    int setColumn(int i, char *val) {
      int ret;
      if (i >= capacity) {
        ret = 0;
      } else {
        columns[i] = val;
        nColumns++; 
        ret = 1;
      };
      return(ret);
    };

    // ---------
    char *getColumn(int i) { return((i >= capacity)?NULL:columns[i]); };

    // ---------
    int numColumns() { return nColumns; }

    ~Row() { delete columns; }

};

class Connection {
  private:
    MYSQL *mysql;
    MYSQL_RES *results[maxResults];
    MYSQL_ROW row;
    int connected;

    // Look for and return the next free result handle.
    int getFreeResultHandle();

    char errormsg[255];

  public:
    Connection();
    Connection(int argc, char** argv);
    ~Connection();

    int connect(int argc, char **argv);
    int isconnected() { return(connected); };
    int selectdb(char *dbname);
    int exec(char *cmd);
    int query(char *cmd);
    int numrows(int resHandle);
    int affectedrows();
    int insertid();
    void endquery(int resHandle);
    Row *fetchrow(int resHandle);

    char *getErrorMsg();
};



