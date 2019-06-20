/*--------------------------------------------------------------------*/
/* conference server */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

extern char *  recvdata(int sd);
extern int     senddata(int sd, char *msg);

extern int     startserver();
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/* main function*/
int main(int argc, char *argv[])
{
  int    serversock;   		/* server socket*/
  int 	 err;				/* err index to indicate if the operation fails*/
  fd_set liveskset, temp;   /* set of live client sockets and temporary copy */
  int    liveskmax;   		/* maximum socket */

  /* check usage */
  if (argc != 1) {
    fprintf(stderr, "usage : %s\n", argv[0]);
    exit(1);
  }

  /* get ready to receive requests */
  serversock = startserver();
  if (serversock == -1) {
    exit(1);
  }
  
  /*
    TODO:
    init the live client set 
  */
	FD_ZERO(&liveskset);
	FD_SET(serversock,&liveskset);
	liveskmax = serversock;
	
  /* receive and process requests */
  while (1) {
    int    itsock;      /* loop variable */
	
    /*
      TODO:
      using select() to serve both live and new clients
    */
	
	//copy liveskset to workig copy temp
	temp = liveskset;
	//use select() to server both live and new clients
    err = select(liveskmax+1, &temp, NULL, NULL, NULL);
	//if select fails
	if(err < 0) on_error("ERROR doing select");
	
    /* process messages from clients */
    for (itsock=3; itsock <= liveskmax; itsock++) {
      /* skip the listen socket */
      if (itsock == serversock) continue;
	  /* TODO: message from client */
      if (FD_ISSET(itsock, &temp)) {
		char *  clienthost;  /* host name of the client */
		ushort  clientport;  /* port number of the client */
	
		/*
		TODO:
		obtain client's host name and port
		using getpeername() and gethostbyaddr()
		*/
		struct sockaddr_in client_addr;
		//socklen_t of client_addr
		socklen_t len = sizeof(client_addr);
		//get struct sockaddr_in client_addr of client by using getpeername
		err = getpeername(itsock, (struct sockaddr*)&client_addr, &len);
		//if getpeername fails
		if(err < 0) on_error("could not get peer name");
		
		struct hostent *client;
		//get struct hostent client by using gethostbyaddr
		client = gethostbyaddr((const char *)&client_addr.sin_addr, sizeof(client_addr.sin_addr), AF_INET);
		//if gethostbyaddr fails
		if(client == NULL) on_error ("client not exist. gethostbyaddr");
		//get clientport from client_addr.sin_port
		clientport = ntohs(client_addr.sin_port);
		//get clienthost from client -> h_name
		clienthost = client -> h_name;
	
		/* read the message */
		char * msg = recvdata(itsock);
		//if the msg is empty
		if (!msg) {
			/* disconnect from client */
			printf("admin: disconnect from '%s(%hu)'\n", clienthost, clientport);

			/*
			TODO:
			remove this client from 'liveskset'  
			*/
			FD_CLR(itsock,&liveskset);
	  
			close(itsock);
		} 
		//else the msg is not empty
		else {
			/*
			TODO:
			send the message to other clients
			*/
			int numsock;
			//traverse all the sockets 
			for (numsock = 3; numsock <= liveskmax; numsock++) 
				//if it is in liveskset
				if (FD_ISSET(numsock,&liveskset))  
					//if it is not itself or serversocket
					if(numsock != itsock && numsock != serversock)
						//send the message to all the client sockets
						senddata(numsock,msg);
	  
			/* print the message */
			printf("%s(%hu): %s", clienthost, clientport, msg);
			free(msg);
		}
      }
    }
	
	/* TODO: connect request from a new client */
    if (FD_ISSET(serversock,&temp)) {

		/*
		TODO:
		accept a new connection request
		*/
		//new connected client socket
		int clientsock;
		struct sockaddr_in newclient_addr;
		//socklen_t of new connected client 
		socklen_t newclient_len = sizeof(newclient_addr);
		//accept the connect request from the new client 
		clientsock = accept(serversock,(struct sockaddr*) &newclient_addr, &newclient_len);
		
		//if accept succeeds
		if (clientsock >= 0) {
			char *  clienthost;  /* host name of the client */
			ushort  clientport;  /* port number of the client */

			/*
			  TODO:
			  get client's host name and port using gethostbyaddr() 
			*/
			struct hostent *newclient;
			//get struct hostent newclient by using gethostbyaddr
			newclient = gethostbyaddr((const char *)&newclient_addr.sin_addr, sizeof(newclient_addr.sin_addr), AF_INET);
			//if gethostbyaddr fails
			if(newclient == NULL) on_error("gethostbyaddr failure");
			//get clientport from newclient_addr.sin_port
			clientport = ntohs(newclient_addr.sin_port);
			//get clienhost from newclient -> h_name
			clienthost = newclient -> h_name;
			printf("admin: connect from '%s' at '%hu'\n", clienthost, clientport);

			/*
			TODO:
			add this client to 'liveskset'
			*/
			FD_SET(clientsock,&liveskset);
			//if the new socket has a larger value than liveskmax, replace with new value
			if(clientsock > liveskmax)
				liveskmax = clientsock;
		} 
		//if accept fails
		else {
			perror("accept");
			exit(0);
		}
	}
  }
}
/*--------------------------------------------------------------------*/
