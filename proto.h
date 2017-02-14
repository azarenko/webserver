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

typedef struct DevicePoint
{
    char* device_id;
    char* name;
    long int datetime_ts;
    double latitude;
    double longitude;
    short altitude;
    short speed;
    short course;
    short accuracy;
    short nsat;
    long int lastupdate_ts;
    char* gprs;
    char* wifi;
    char* connected;
    char* battary;
    long int lastcharge_ts;
    char *client_ip;
    unsigned short client_port;
} DevicePoint;

int proto(char* reqdata, int len, const char** outPacketId, char *client_ip, unsigned short client_port);

int savepoint(DevicePoint *dp);

int parsepoint(cJSON* point, char *client_ip, unsigned short client_port);

int db_login(PGconn **conn);

int execsql(PGconn *conn, char *sql, char *report);

PGresult *getexecsql(PGconn *conn, char * sql);

void clearres(PGconn *conn, PGresult *res);

PGconn* getConnection(void);
