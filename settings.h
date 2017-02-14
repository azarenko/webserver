#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libconfig.h>

config_t cfg;

/* Connection backlog (# of backlogged connections to accept). */
int CONNECTION_BACKLOG;

// configuration file path
const char* configFilePath;

// my address 
const char* addr;

// my port
int port;

// 0:nothing; 1:critical_error; 2:all_warnings;
int debug;

// over this in second kill session if not new pocket
int waitinactive;

// time for repetative send data (example send data to monitor)
int oftentime;

// wait this time if not received all data in one pocket
int betweentimeout;

//
// primary database
//
const char* primarypglogin;

// Password to be used if the server demands password authentication.
const char* primarypgpwd;

// Name of host to connect to. If this begins with a slash, it specifies Unix-domain communication 
// rather than TCP/IP communication; the value is the name of the directory in which the socket 
// file is stored. The default behavior when host is not specified is to connect to a Unix-domain 
// socket in /tmp (or whatever socket directory was specified when PostgreSQL was built). 
// On machines without Unix-domain sockets, the default is to connect to localhost.
const char* primarypghost;

// Port number to connect to at the server host, or socket file name extension for Unix-domain connections.
const char* primarypgport;

// base name for psql database
const char* primarydbname;

// Maximum wait for connection, in seconds (write as a decimal integer string).
// Zero or not specified means wait indefinitely. It is not recommended to use a timeout of less than 2 seconds.
int primarytimeout;

// This option determines whether or with what priority an SSL connection will be negotiated with the server.
// There are four modes: 
// disable will attempt only an unencrypted SSL connection;
// allow   will negotiate, trying first a non-SSL connection, then if that fails, trying an SSL connection;
// prefer  (the default) will negotiate, trying first an SSL connection, then if that fails, trying a regular non-SSL connection;
// require will try only an SSL connection.
const char* primaryssl;

//
// secondary database
//

// PostgreSQL user name to connect as. Defaults to be the same as the operating system name 
// of the user running the application.
const char* secondarypglogin;

// Password to be used if the server demands password authentication.
const char* secondarypgpwd;

// Name of host to connect to. If this begins with a slash, it specifies Unix-domain communication 
// rather than TCP/IP communication; the value is the name of the directory in which the socket 
// file is stored. The default behavior when host is not specified is to connect to a Unix-domain 
// socket in /tmp (or whatever socket directory was specified when PostgreSQL was built). 
// On machines without Unix-domain sockets, the default is to connect to localhost.
//secondarypghost  = 144.76.7.199
const char* secondarypghost;

// Port number to connect to at the server host, or socket file name extension for Unix-domain connections.
const char* secondarypgport;

// base name for psql database
const char* secondarydbname;

// Maximum wait for connection, in seconds (write as a decimal integer string).
// Zero or not specified means wait indefinitely. It is not recommended to use a timeout of less than 2 seconds.
int secondarytimeout;

// This option determines whether or with what priority an SSL connection will be negotiated with the server.
// There are four modes: 
// disable will attempt only an unencrypted SSL connection;
// allow   will negotiate, trying first a non-SSL connection, then if that fails, trying an SSL connection;
// prefer  (the default) will negotiate, trying first an SSL connection, then if that fails, trying a regular non-SSL connection;
// require will try only an SSL connection.
const char* secondaryssl;

// transfring simple data to
const char* transferaddr;
int transferport;

int isdaemon;

const char* pidfile;

int usememcache;

int readSettings(const char* configFilePath);
