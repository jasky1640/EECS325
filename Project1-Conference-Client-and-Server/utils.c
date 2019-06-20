/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <error.h>

#define MAXNAMELEN 256
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }
/*--------------------------------------------------------------------*/


/*----------------------------------------------------------------*/
int startserver()
{
	int err;
	int     sd;        /* socket */
	char *  serverhost;  /* hostname */
	ushort  serverport;  /* server port */

  /*
    TODO:
    create a TCP socket 
  */
	sd = socket(AF_INET, SOCK_STREAM, 0);
  /*
    TODO:
    bind the socket to some random port, chosen by the system 
  */
	struct sockaddr_in addr;
	//randomly assigned by system
	addr.sin_port = 0;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//bind the socket to the dynamic port chosen by system
	err = bind (sd, (struct sockaddr*)&addr, sizeof(addr));
	//if bind fails
	if(err < 0) on_error("Could not bind socket\n");
	
  /* ready to receive connections */
	err = listen(sd, 5);
	//if listen fails
	if(err < 0) on_error("Could not listen on socket\n");


   /*
    TODO:
    obtain the full local host name (serverhost)
    use gethostname() and gethostbyname()
  */
	char serverpartialname[MAXNAMELEN];
	//get partial name of server
	err = gethostname(serverpartialname,MAXNAMELEN);
	//if gethostname fails
	if(err < 0) on_error("Could not get the host name");
	struct hostent *server;
	//use gethostbyname to get hostent struct of server
	server = gethostbyname(serverpartialname);
	//if gethostbyname fails
	if(server == NULL) on_error("GETHOSTBYNAME ERROR. CANNOT FIND THE HOST");
	//get full name from hostent server
	serverhost = server -> h_name;
	
	
  /*
    TODO:
    get the port assigned to this server (serverport)
    use getsockname()
  */
	//socklen_t of sockaddr_in
	socklen_t len = sizeof(addr);
	//use getsockname to get the port assigned to serverport
	err = getsockname(sd, (struct sockaddr *) &addr, &len);
	//if getsockname fails
	if(err < 0) perror("getsockname");
	//use ntohs to convert to network format
	serverport = ntohs(addr.sin_port);
	
  /* ready to accept requests */
	printf("admin: started server on '%s' at '%hu'\n", serverhost, serverport); 
	return sd;
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
  establishes connection with the server
*/
int connecttoserver(char *serverhost, ushort serverport)
{
	int     sd;          /* socket */
	ushort  clientport;  /* port assigned to this client */
	struct sockaddr_in server_addr;

  /*
    TODO:
    create a TCP socket 
  */
	sd = socket(AF_INET, SOCK_STREAM, 0);	
	if(sd < 0)
		on_error("ERROr OEPNNING SOCKET");
	memset(&server_addr, 0, sizeof(server_addr));
  /*
    TODO:
    connect to the server on 'serverhost' at 'serverport'
    use gethostbyname() and connect()
  */
	struct hostent *server;
	server = gethostbyname(serverhost);
	if(server == NULL){
		fprintf(stderr,"ERROR,NO SUCH HOST");
		exit(0);
	}
	server_addr.sin_port = htons(serverport);
	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr = inet_addr(server -> h_addr);
	memcpy(&server_addr.sin_addr, server->h_addr_list[0], server->h_length);
	if(connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <0)
		on_error("ERROR CONNECTING");
	
  /*
    TODO:
    get the port assigned to this client
    use getsockname()
  */
	struct sockaddr_in sin;
	socklen_t sin_len = sizeof(sin);
	if(getsockname(sd, (struct sockaddr *) &sin, &sin_len) < 0)
		perror("getsockname");
	clientport = ntohs(sin.sin_port);
	
  /* succesful. return socket */
	printf("admin: connected to server on '%s' at '%hu' thru '%hu'\n", serverhost, serverport, clientport);
	return(sd);
}
/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
int readn(int sd, char *buf, int n)
{
  int     toberead;
  char *  ptr;

  toberead = n;
  ptr = buf;
  while (toberead > 0) {
    int byteread;

    byteread = read(sd, ptr, toberead);
    if (byteread <= 0) {
      if (byteread == -1)
	perror("read");
      return(0);
    }
    
    toberead -= byteread;
    ptr += byteread;
  }
  return(1);
}

char *recvdata(int sd)
{
  char *msg;
  long  len;
  
  /* get the message length */
  if (!readn(sd, (char *) &len, sizeof(len))) {
    return(NULL);
  }
  len = ntohl(len);

  /* allocate memory for message */
  msg = NULL;
  if (len > 0) {
    msg = (char *) malloc(len);
    if (!msg) {
      fprintf(stderr, "error : unable to malloc\n");
      return(NULL);
    }

    /* read the message */
    if (!readn(sd, msg, len)) {
      free(msg);
      return(NULL);
    }
  }
  
  return(msg);
}

int senddata(int sd, char *msg)
{
  long len;

  /* write lent */
  len = (msg ? strlen(msg) + 1 : 0);
  len = htonl(len);
  write(sd, (char *) &len, sizeof(len));

  /* write message data */
  len = ntohl(len);
  if (len > 0)
    write(sd, msg, len);
  return(1);
}
/*----------------------------------------------------------------*/
