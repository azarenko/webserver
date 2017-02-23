#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <event.h>
#include <signal.h>
#include <libconfig.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/types.h>
#include <locale.h>
#include <evhttp.h>
#include <event2/bufferevent.h>
#include <libpq-fe.h>

#include "cmdparam.h"
#include "settings.h"
#include "sockutils.h"
#include "proto.h"

#define MAX_CONTENT_LEN 1024 * 512

/*
 * Terminate server
 */
static void terminate(void)
{
    if (event_loopbreak()) 
    {
	syslog(LOG_ERR, "Error shutting down server");
    }
}

/**
 * @brief ...
 * 
 * @param signal ...
 * @return void
 */
static void sighandler(int signal) {
    switch(signal)
    {
        case SIGTERM: 
            syslog(LOG_INFO, "Received signal terminate.");
            terminate();
            break;
        case SIGHUP:
            syslog(LOG_INFO, "Received signal hangup.");
            if(readSettings(configFilePath) == EXIT_FAILURE)
            {
                syslog(LOG_ERR, "Configuration file error.");                
            }
            break;
    }
}

/**
 * @brief ...
 * 
 * @param host ...
 * @param port ...
 * @param backlog ...
 * @return int
 */
int httpserver_bindsocket(char* host, int port, int backlog) {
  int r;
  int nfd;
  nfd = socket(AF_INET, SOCK_STREAM, 0);
  if (nfd < 0) return -1;
 
  int one = 1;
  r = setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int));
 
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(host);
  addr.sin_port = htons(port);
 
  r = bind(nfd, (struct sockaddr*)&addr, sizeof(addr));
  if (r < 0) return -1;
  r = listen(nfd, backlog);
  if (r < 0) return -1;
 
  int flags;
  if ((flags = fcntl(nfd, F_GETFL, 0)) < 0
      || fcntl(nfd, F_SETFL, flags | O_NONBLOCK) < 0)
    return -1;
 
  return nfd;
}
 
 /**
  * @brief ...
  * 
  * @param arg ...
  * @return void*
  */
void* httpserver_Dispatch(void *arg) {
  event_base_dispatch((struct event_base*)arg);
  return NULL;
}
 
 /**
  * @brief ...
  * 
  * @param req ...
  * @return void
  */
 void httpserver_ProcessRequest(struct evhttp_request *req) {
    
    struct evbuffer *inbuff;    
    char content[MAX_CONTENT_LEN];
    int contentLen = 0;
    bzero(content, MAX_CONTENT_LEN);
    
    inbuff = evhttp_request_get_input_buffer(req);    
	while (evbuffer_get_length(inbuff)) 
    {
	contentLen += evbuffer_remove(inbuff, content + contentLen, 128);		
    }
      
    struct evbuffer *outbuff = evbuffer_new();
    
    char *client_ip;
    unsigned short client_port;

    evhttp_connection_get_peer(evhttp_request_get_connection(req), &client_ip, &client_port);
    
    char* responceMessage;
    int responceLength;
    
    int statuscode = proto(content, contentLen, &responceMessage);        
    
    evbuffer_add_printf(outbuff, "%s", responceMessage);
    
    free(responceMessage);
          
    char outcontentLengthStr[3];
    sprintf(outcontentLengthStr, "%zu", evbuffer_get_length(outbuff));
    
    // Set HTTP headers    
    evhttp_add_header(req->output_headers, "Content-Type", "application/json");
    evhttp_add_header(req->output_headers, "Connection", "close");
    evhttp_add_header(req->output_headers, "Content-Length", outcontentLengthStr);

    // Send reply
    evhttp_send_reply(req, statuscode, "OK", outbuff);

    // Free memory
    evbuffer_free(outbuff);    
}
 
 /**
  * @brief ...
  * 
  * @param req ...
  * @param arg ...
  * @return void
  */
 void httpserver_GenericHandler(struct evhttp_request *req, void *arg) {
    httpserver_ProcessRequest(req);
}
 
 /**
  * @brief ...
  * 
  * @param host ...
  * @param port ...
  * @param nthreads ...
  * @param backlog ...
  * @return int
  */
 int httpserver_start(char* host, int port, int nthreads, int backlog) 
{
  int r, i;
  int nfd = httpserver_bindsocket(host, port, backlog);
  if (nfd < 0) return -1;
  pthread_t ths[nthreads];
  for (i = 0; i < nthreads; i++) {
    struct event_base *base = event_init();
    if (base == NULL) return -1;
    struct evhttp *httpd = evhttp_new(base);
    if (httpd == NULL) return -1;
    r = evhttp_accept_socket(httpd, nfd);
    if (r != 0) return -1;
    evhttp_set_gencb(httpd, httpserver_GenericHandler, NULL);
    r = pthread_create(&ths[i], NULL, httpserver_Dispatch, base);
    if (r != 0) return -1;
  }
  for (i = 0; i < nthreads; i++) {
    pthread_join(ths[i], NULL);
  }
}

/**
 * @brief ...
 * 
 * @param argc ...
 * @param argv ...
 * @return int
 */
int main(int argc, char **argv)
{    
    setlocale(LC_ALL, "UTF-8");
    /*
     * init syslog
     */
    openlog(program_invocation_short_name, 0, LOG_USER);
    syslog(LOG_INFO, "Starting.");
    
    /*
     * parse command line params
     */
    if(getCmdParam(argc, argv, "-f", &configFilePath) == EXIT_FAILURE)
    {
        syslog(LOG_ERR, "Invalid input params. Use -f path_to_config.");
        goto exit;
    }
    
    /*
     * read settings
     */
    if(readSettings(configFilePath) == EXIT_FAILURE)
    {
        syslog(LOG_ERR, "Configuration file error.");
        goto exit;
    }    

    if(isdaemon == 1)
    {
	 daemon(1, 0);	 
    }

    /* Create pid file */

    FILE* pidf = fopen(pidfile, "w");

    if(pidf==NULL)
    {
        syslog(LOG_ERR, "Failed to write pid file");
	return;
    }

    fprintf(pidf, "%d", getpid());

    fclose(pidf);    

    /* Set signal handlers */
	sigset_t sigset;
	sigemptyset(&sigset);
	struct sigaction siginfo = {
		.sa_handler = sighandler,
		.sa_mask = sigset,
		.sa_flags = SA_RESTART,
	};
	sigaction(SIGHUP, &siginfo, NULL);
	sigaction(SIGTERM, &siginfo, NULL);
    
    pthread_mutex_init(&connectionm, NULL);
	
    connections = (PGconn**)malloc(sizeof(PGconn*) * CONNECTION_BACKLOG);
    selectconnectionlock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * CONNECTION_BACKLOG);
    
    int i;
    for(i = 0; i < CONNECTION_BACKLOG; i++)
    {
        connections[i] = NULL;
        db_login(&connections[i]);
        pthread_mutex_init(&selectconnectionlock[i], NULL);
    }
    
    if(httpserver_start(addr, port, CONNECTION_BACKLOG, CONNECTION_BACKLOG) == -1)
    {
        syslog(LOG_ERR, "Failed to start server");
        goto exit;
    }
  
  exit:
    free(connections);
    free(selectconnectionlock);
    syslog(LOG_INFO, "Stoping.");
    config_destroy(&cfg);
    closelog();    
    return EXIT_SUCCESS;
}
