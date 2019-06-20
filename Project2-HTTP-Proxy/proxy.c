#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>

#include "utils.h"

extern int     sendrequest(int sd);
extern char *  readresponse(int sd);
extern void    forwardresponse(int sd, char *msg);
extern int     startserver();

int main(int argc, char *argv[])
{
  int servsock;    /* server socket descriptor */

  fd_set livesdset, servsdset;   /* set of live client sockets and set of live http server sockets */
  
  /* TODO: define largest file descriptor number used for select */
  int livesdmax;
  
  struct pair * table = malloc(sizeof(struct pair)); /* table to keep client<->server pairs */

  char *msg;

  /* check usage */
  if (argc != 1) {
    fprintf(stderr, "usage : %s\n", argv[0]);
    exit(1);
  }

  /* get ready to receive requests */
  servsock = startserver();
  if (servsock == -1) {
    exit(1);
  }

  table->next = NULL;

  /* TODO: initialize all the fd_sets and largest fd numbers */
  FD_ZERO(&livesdset);          //Empty livesdset
  FD_ZERO(&servsdset);			//Empty servsdset
  FD_SET(servsock,&livesdset);	//Put servsock into livesdset
  livesdmax = servsock; 		//Set the largest fd to be servsock
  
  while (1) {
    int frsock;

    /* TODO: combine livesdset and servsdset 
     * use the combined fd_set for select */
	fd_set combinedset;     //Create a new fd_set to combine livesdset and servsdset
	FD_ZERO(&combinedset);	//Empty combinedset
	//Iterate over all possible fd number
	for(frsock = 0; frsock <= livesdmax; frsock++)
		//if fd is either in livesdset or in servsdset
		if(FD_ISSET(frsock, &livesdset) || FD_ISSET(frsock, &servsdset))
			//put the fd in combinedset
			FD_SET(frsock, &combinedset);

	/* TODO: select from the combined fd_set */
    if(select(livesdmax+1, &combinedset, NULL, NULL, NULL) == -1) {
        fprintf(stderr, "Can't select.\n");
        continue;
    }

	/* TODO: iterate over file descriptors */
    for (frsock = 0; frsock <= livesdmax+1; frsock++) {
			
        if (frsock == servsock) continue;

		/* TODO: input from existing client? */
		if(FD_ISSET(frsock, &livesdset)) {
			
			/* forward the request */
			int newsd = sendrequest(frsock);
			if (!newsd) {
				printf("admin: disconnect from client\n");	
				/*TODO: clear frsock from fd_set(s) */
				FD_CLR(frsock,&livesdset);		//Remove the client socket from livesdset
				FD_CLR(frsock,&combinedset);	//Remove the client socket from combinedset	
			} 
			else {
				insertpair(table, newsd, frsock);			
				/* TODO: insert newsd into fd_set(s) */
				FD_SET(newsd, &servsdset);		//Put newsd in the servsdset
				FD_SET(newsd, &combinedset);	//Put newsd in the combinedset
				//If newsd is larger than current largest fd number, set it as new livesdmax
				if(newsd > livesdmax)
					livesdmax = newsd;
			}
		} 
	
		/* TODO: input from the http server? */
		if(FD_ISSET(frsock, &servsdset)) {
					
			char *msg;
			struct pair *entry=NULL;	
			struct pair *delentry;
			msg = readresponse(frsock);
			printf("The message read from HTTP server is %s\n",msg);
			if (!msg) {
				fprintf(stderr, "error: server died\n");
				exit(1);
			}
				
			/* forward response to client */
			entry = searchpair(table, frsock);
			if(!entry) {
				fprintf(stderr, "error: could not find matching clent sd\n");
				exit(1);
			}
			forwardresponse(entry->clientsd, msg);
			delentry = deletepair(table, entry->serversd);

			/* TODO: clear the client and server sockets used for 
			 * this http connection from the fd_set(s) */
			FD_CLR(frsock, &servsdset);				//Remove the server socket from servsdset
			FD_CLR(frsock, &combinedset);			//Remove the server socket from combinedset
			FD_CLR(entry->clientsd, &livesdset);	//Remove the client socket from livesdset
			FD_CLR(entry->clientsd, &combinedset);	//Remove the client socket from combinedset
		}
    }
	
    /* input from new client*/
    if(FD_ISSET(servsock, &combinedset)) {
		struct sockaddr_in caddr;
      	socklen_t clen = sizeof(caddr);
      	int csd = accept(servsock, (struct sockaddr*)&caddr, &clen);
		printf("accepted.\n");
		if (csd != -1) {
			/* TODO: put csd into fd_set(s) */
			FD_SET(csd, &livesdset);	//Put csd in the livesdset
			FD_SET(csd, &combinedset);	//Put csd in the combinedset
			//If csd is larger than current largest fd number, set it as new livesdmax
			if(csd > livesdmax)
				livesdmax = csd;
		} 
		else {
			perror("accept");
            exit(0);
		}
    }
  }
}
