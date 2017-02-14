#include <math.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <libpq-fe.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#include "proto.h"
#include "settings.h"

#define MAXLENQUERY     2048
#define REPORTLEN       256
#define POCKETMAXLEN    1500
#define MAXCHIELDS      4096
#define FD_COPY(f, t)   (void)(*(t) = *(f))
#define INITPACKETLEN 17
#define SERIALIZESENSORLEN 2048

#define DECL  "DECLARE cursor%u CURSOR FOR "
#define FETCH "FETCH ALL in cursor%u"
#define CLOSE "CLOSE cursor%u"

/**
 * @brief ...
 * 
 * @param reqdata ...
 * @param len ...
 * @param outPacketId ...
 * @param client_ip ...
 * @param client_port ...
 * @return int
 */
int proto(char* reqdata, int len, const char** outPacketId, char *client_ip, unsigned short client_port)
{
    cJSON *root = cJSON_Parse(reqdata);
    
    cJSON *id = cJSON_GetObjectItem(root, "id");
    if(id == NULL)
    {
        syslog(LOG_ERR, "Error persing id");
        return 415;
    }
    *outPacketId = id->valuestring;
    
    cJSON *data = cJSON_GetObjectItem(root, "data");
    if(data == NULL)
    {
        syslog(LOG_ERR, "Error persing data");
        return 415;
    }
    
    if(data->type != cJSON_Array && data->type != cJSON_String)
    {
        syslog(LOG_ERR, "Error persing type of data member");
        return 415;
    }
    
    if(data->type == cJSON_String)
    {
        cJSON *point = cJSON_Parse(data->valuestring);        
        return parsepoint(point, client_ip, client_port);
    }
    
    if(data->type == cJSON_Array)
    {
        int length = cJSON_GetArraySize(data);
        int i;
        for(i = 0; i < length; i++)
        {
            cJSON * subitem = cJSON_GetArrayItem(data, i);
	    int status = parsepoint(subitem, client_ip, client_port);
            if(status != 200)
            {
                return status;
            }
        }
    }
    
    return 200;
}

/**
 * @brief ...
 * 
 * @param point ...
 * @param client_ip ...
 * @param client_port ...
 * @return int
 */
int parsepoint(cJSON* point, char *client_ip, unsigned short client_port)
{
    DevicePoint dp;
    
        dp.client_ip = client_ip;
        dp.client_port = client_port;
    
        cJSON* device_id = cJSON_GetObjectItem(point, "device_id");
        if(device_id == NULL)
        {
	    syslog(LOG_ERR, "Error persing device_id");
            return 415;
        }
        dp.device_id = device_id->valuestring;
        
        cJSON* name = cJSON_GetObjectItem(point, "name");
        if(name == NULL)
        {
	    syslog(LOG_ERR, "Error persing name");
            return 415;
        }
        dp.name = name->valuestring;
        
        cJSON* datetime_ts = cJSON_GetObjectItem(point, "datetime_ts");
        if(datetime_ts == NULL)
        {
	    syslog(LOG_ERR, "Error persing datetime_ts");
            return 415;
        }
        if(datetime_ts->type == cJSON_String)
        {
            dp.datetime_ts = atol(datetime_ts->valuestring);
        }
        else
        {
            dp.datetime_ts = datetime_ts->valueint;
        }
        
        cJSON* latitude = cJSON_GetObjectItem(point, "latitude");
        if(latitude == NULL)
        {
	  syslog(LOG_ERR, "Error persing latitude");
            return 415;
        }
        dp.latitude = latitude->valuedouble;
        
        cJSON* longitude = cJSON_GetObjectItem(point, "longitude");
        if(longitude == NULL)
        {
	  syslog(LOG_ERR, "Error persing longitude");
            return 415;
        }
        dp.longitude = longitude->valuedouble;
        
        cJSON* altitude = cJSON_GetObjectItem(point, "altitude");
        if(altitude == NULL)
        {
	  syslog(LOG_ERR, "Error persing altitude");
            return 415;
        }
        dp.altitude = altitude->valueint;
        
        cJSON* speed = cJSON_GetObjectItem(point, "speed");
        if(speed == NULL)
        {
	  syslog(LOG_ERR, "Error persing speed");
            return 415;
        }
        dp.speed = speed->valueint;
        
        cJSON* course = cJSON_GetObjectItem(point, "course");
        if(course == NULL)
        {
	  syslog(LOG_ERR, "Error persing course");
            return 415;
        }
        dp.course = course->valueint;
        
        cJSON* accuracy = cJSON_GetObjectItem(point, "accuracy");
        if(accuracy == NULL)
        {
	  syslog(LOG_ERR, "Error persing accuracy");
            return 415;
        }
        dp.accuracy = accuracy->valueint;
        
        cJSON* nsat = cJSON_GetObjectItem(point, "nsat");
        if(nsat == NULL)
        {
	   syslog(LOG_ERR, "Error persing nsat");
            return 415;
        }
        dp.nsat = nsat->valueint;
        
        cJSON* lastupdate_ts = cJSON_GetObjectItem(point, "lastupdate_ts");
        if(lastupdate_ts == NULL)
        {
	   syslog(LOG_ERR, "Error persing lastupdate_ts");
            return 415;
        }
        if(lastupdate_ts->type == cJSON_String)
        {
            dp.lastupdate_ts = atol(lastupdate_ts->valuestring);
        }
        else
        {
            dp.lastupdate_ts = lastupdate_ts->valueint;
        }
        
        cJSON* device_info = cJSON_GetObjectItem(point, "device_info");
	if(device_info == NULL)
        {
	  syslog(LOG_ERR, "Error persing device_info");
            return 415;
        }
        cJSON* gprs = cJSON_GetObjectItem(device_info, "gprs");
        if(gprs == NULL)
        {
	  syslog(LOG_ERR, "Error persing gprs");
            return 415;
        }
        dp.gprs = gprs->valuestring;
        
        cJSON* wifi = cJSON_GetObjectItem(device_info, "wifi");
        if(wifi == NULL)
        {
	  syslog(LOG_ERR, "Error persing wifi");
            return 415;
        }
        dp.wifi = wifi->valuestring;
        
        cJSON* connected = cJSON_GetObjectItem(device_info, "connected");
        if(connected == NULL)
        {
	  syslog(LOG_ERR, "Error persing connected");
            return 415;
        }
        dp.connected = connected->valuestring;
        
        cJSON* battary = cJSON_GetObjectItem(device_info, "battary");
        if(battary == NULL)
        {
	  syslog(LOG_ERR, "Error persing battary");
            return 415;
        }
        dp.battary = battary->valuestring;
        
        cJSON* lastcharge_ts = cJSON_GetObjectItem(device_info, "lastcharge_ts");
        if(lastcharge_ts == NULL)
        {
	  syslog(LOG_ERR, "Error persing lastcharge_ts");
            return 415;
        }
        if(lastupdate_ts->type == cJSON_String)
        {
            dp.lastcharge_ts = atol(lastcharge_ts->valuestring);
        }
        else
        {
            dp.lastcharge_ts = lastcharge_ts->valueint;
        }
        
        return savepoint(&dp);
}

/**
 * @brief ...
 * 
 * @param conn ...
 * @return int
 */
int db_login(PGconn **conn)
{    
    pthread_mutex_lock(&connectionm);
    if (PQstatus(*conn) == CONNECTION_BAD) 
    {
        char *pgoptions=NULL, *pgtty=NULL;
        *conn = PQsetdbLogin(primarypghost, primarypgport, pgoptions, pgtty, primarydbname, primarypglogin, primarypgpwd);
        if (PQstatus(*conn) == CONNECTION_BAD) 
        { 
            if(debug>1)syslog(LOG_ERR,"Connection to database failed %s", PQerrorMessage(*conn));
            PQfinish(*conn);
                        
            *conn = PQsetdbLogin(secondarypghost, secondarypgport, pgoptions, pgtty, secondarydbname, secondarypglogin, secondarypgpwd);
            
            if (PQstatus(*conn) == CONNECTION_BAD) 
            { 
                if(debug>1)syslog(LOG_ERR,"Connection to database failed %s", PQerrorMessage(*conn));
                PQfinish(*conn);
		pthread_mutex_unlock(&connectionm);
                return 0;
            }
        }       
        pthread_mutex_unlock(&connectionm);
    }
    else
    {
	pthread_mutex_unlock(&connectionm);
        return 1;
    }
}

/**
 * @brief ...
 * 
 * @param conn ...
 * @param sql ...
 * @param report ...
 * @return int
 */
int execsql(PGconn *conn, char *sql, char *report)
{
    if (PQstatus(conn) == CONNECTION_BAD) 
    {
        return (2);
    }
    
    PGresult   *res;

    res = PQexec(conn, "BEGIN");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        if(debug)syslog(LOG_ERR,"BEGIN command failed");
        strncpy(report, PQerrorMessage(conn),99);
        PQclear(res);
        return (1);
    }
    if(debug>1)syslog(LOG_ERR,"BEGIN command ok");

    PQclear(res);
    res = PQexec(conn, sql);
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        if(debug)syslog(LOG_ERR,"PQexec command failed");
        strncpy(report, PQerrorMessage(conn),99);
        PQclear(res);
        return (2);
    }
    if(debug>1)syslog(LOG_ERR,"PQexec command ok");

    PQclear(res);
    res = PQexec(conn, "COMMIT");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) { 
        if(debug)syslog(LOG_ERR,"COMMIT command failed");
        strncpy(report, PQerrorMessage(conn),99);
        PQclear(res);
        return (3);
    }
    PQclear(res);
    if(debug>1)syslog(LOG_ERR,"COMMIT command ok");
    return (0); 
}

/**
 * @brief ...
 * 
 * @param conn ...
 * @param sql ...
 * @return PGresult*
 */
PGresult *getexecsql(PGconn *conn, char * sql)
{   
    if (PQstatus(conn) == CONNECTION_BAD) 
    {
        return NULL;
    }
    
    char       *sqlfull;
    int        sqllen;
    PGresult   *res;
    
    res = PQexec(conn, "BEGIN");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK){ 
            if(debug)syslog(LOG_ERR,"BEGIN failed");
            PQclear(res);
            return NULL;
    }
    PQclear(res);
    if(debug>1)syslog(LOG_ERR,"BEGIN ok");

    char request[MAXLENQUERY];
    bzero(request, MAXLENQUERY);    
    sprintf(request, DECL, conn);
    
    sqllen=strlen(request);              
    sqllen+=strlen(sql);
    sqlfull=(char*)malloc(sqllen+1);  
    memset(sqlfull, 0x00, sqllen+1);
    strcpy(sqlfull, request);            
    strncat(sqlfull, sql, strlen(sql)); 
    
    if(debug>3)syslog(LOG_ERR,"%s",sqlfull);
    res = PQexec(conn, sqlfull);
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK){
        if(debug)syslog(LOG_ERR,"DECLARE failed");
        PQclear(res);
        return NULL;
    }
    PQclear(res);
    free(sqlfull);
    if(debug>1)syslog(LOG_ERR,"DECLARE ok");

    bzero(request, MAXLENQUERY);
    sprintf(request, FETCH, conn);
    
    res = PQexec(conn, request);
    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK){
        if(debug)syslog(LOG_ERR,"FETCH failed");
        PQclear(res);
        return NULL;
    }
    if(debug>1)syslog(LOG_ERR,"FETCH ok");
    return res;
}

/**
 * @brief ...
 * 
 * @param conn ...
 * @param res ...
 * @return void
 */
void clearres(PGconn *conn, PGresult *res)
{
    if (PQstatus(conn) == CONNECTION_BAD) 
    {
        return;
    }

    if(res)
    {
    	PQclear(res);
    }    

    char request[MAXLENQUERY];
    bzero(request, MAXLENQUERY);    
    sprintf(request, CLOSE, conn);
    
    res = PQexec(conn, request);
    if(debug>1)syslog(LOG_ERR,"CLOSE ok");
    PQclear(res);
    res = PQexec(conn, "COMMIT");
    if(debug>1)syslog(LOG_ERR,"COMMIT ok");
    PQclear(res);    
} 

/**
 * @brief ...
 * 
 * @param dp ...
 * @return int
 */
int savepoint(DevicePoint *dp)
{
    int ret, num, ifexit;
    char id[sizeof(long)+1];
    PGresult *res;    
       
    int connectionId = connectionIndex++ % CONNECTION_BACKLOG;
    
    pthread_mutex_lock(&selectconnectionlock[connectionId]);
    PGconn* conn = connections[connectionId];
    
    if (PQstatus(conn) == CONNECTION_BAD) 
    {
        conn = NULL;
        if(db_login(&conn) == 0)
        {
            pthread_mutex_unlock(&selectconnectionlock[connectionId]);
            return 500;
        }
    }
    
    char query[MAXLENQUERY];
    bzero(query,MAXLENQUERY);
    
    ret = sprintf(query,"SELECT * FROM public.deviceauth('%s', '', '%s', %d, 24);", dp->device_id, dp->client_ip, dp->client_port);
    
    res = getexecsql(conn, query);
    if(res)
    {
        if (PQgetisnull(res,0,0))
        {
            sprintf(id,"0");
            ifexit=1;
            if(debug>1)syslog(LOG_ERR,"getexec sql id not found %s", dp->device_id);
        } 
        else 
        {
            ret = sprintf(id,"%s",PQgetvalue(res, 0, 0));
            ifexit=0;
            if(debug>1)syslog(LOG_ERR,"getexec sql found id=%s",id);
        }         
    }
    clearres(conn, res);
    
    if(ifexit)
    {
        pthread_mutex_unlock(&selectconnectionlock[connectionId]);
        return 401;
    }
    
    bzero(query,MAXLENQUERY);
    ret = sprintf(query,
    "SELECT public.device24save(\
    24::bigint,\
    %s::bigint,\
    %lu::bigint,\
    %f::double precision,\
    %f::double precision,\
    %d::real,\
    %d::integer,\
    %d::smallint,\
    %d::smallint,\
    %d::smallint,\
    %d::smallint,\
    %s::smallint,\
    %s::integer,\
    0::bigint,\
    ''::text,\
    FALSE,\
    ARRAY[0,0,%d%s]::double precision[],\
    ARRAY[]::text[]\
    );",
                 id,                 
                 dp->datetime_ts,
                 dp->latitude,
                 dp->longitude,
                 dp->speed, 
                 0,
                 dp->altitude,
		 dp->course,
                 dp->nsat, 
                 dp->accuracy,
                 dp->gprs,
                 dp->battary,
                 0,
                 ""
                );

    if(debug)syslog(LOG_ERR,"query: %s",query);
        
    int status = 200;
    res = getexecsql(conn, query);
    if(res)
    {
        if (PQgetisnull(res,0,0))
        {
          if(debug)syslog(LOG_WARNING,"can't insert track record errno %d",ret);
          if(debug>1)syslog(LOG_WARNING,"%s",query);
	  status = 500;		
        }              
    }
    else
    {
	status = 500;
    }
    clearres(conn, res); 
    
    pthread_mutex_unlock(&selectconnectionlock[connectionId]);
    return status;
}
