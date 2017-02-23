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
#define MAX_CONTENT_LEN 1024 * 512

#define DECL  "DECLARE cursor%u CURSOR FOR "
#define FETCH "FETCH ALL in cursor%u"
#define CLOSE "CLOSE cursor%u"


int proto(char* reqdata, int len, const char** responceMessage)
{
    *responceMessage = malloc(sizeof(char) * MAX_CONTENT_LEN);
    bzero(*responceMessage, MAXLENQUERY);
  
    cJSON *root = cJSON_Parse(reqdata);
    
    cJSON *type = cJSON_GetObjectItem(root, "type");
    if(type == NULL)
    {
        sprintf(*responceMessage, "%s", "{error:'Error persing type property'}");
        syslog(LOG_ERR, *responceMessage);
	  
        return 200;
    }
    
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
    
    sprintf(query,"SELECT %s('%s');", type->valuestring, reqdata);
    
    res = getexecsql(conn, query);
    if(res)
    {
        if (PQgetisnull(res,0,0))
        {
            if(debug>1)syslog(LOG_ERR, "exec sql error");
        } 
        else 
        {
            sprintf(*responceMessage,"%s",PQgetvalue(res, 0, 0));
            if(debug>1)syslog(LOG_ERR,"exec sql ok. responce = %s", *responceMessage);
        }         
    }
    clearres(conn, res);
    
    pthread_mutex_unlock(&selectconnectionlock[connectionId]);
    return 200;
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
