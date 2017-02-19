#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <libpq-fe.h>

#include "settings.h"
#include "cJSON.h"

const PGconn** connections;
pthread_mutex_t* selectconnectionlock;
unsigned long long int connectionIndex;
pthread_mutex_t connectionm;


int proto(char* reqdata, int len, const char** responceMessage);

int db_login(PGconn **conn);

int execsql(PGconn *conn, char *sql, char *report);

PGresult *getexecsql(PGconn *conn, char * sql);

void clearres(PGconn *conn, PGresult *res);

PGconn* getConnection(void);
