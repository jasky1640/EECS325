/*--------------------------------------------------------------------*/
/* conference client */
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

#define MAXMSGLEN  1024

extern char *  recvdata(int sd);
extern int     senddata(int sd, char *msg);

extern int     connecttoserver(char *servhost, ushort servport);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	int  sock,liveskmax;
	/* check usage */
	if (argc != 3) {
		fprintf(stderr, "usage : %s <servhost> <servport>\n", argv[0]);
		exit(1);
	}

	/* connect to the server */
	sock = connecttoserver(argv[1], atoi(argv[2]));
	//if connecttoserver fails
	if (sock == -1)
		exit(1);
	//fd_set liveskset and working copy temp
	fd_set liveskset, temp;
	//clear the livesket
	FD_ZERO (&liveskset);
	//add the sock connected to server into the liveskset
	FD_SET (sock, &liveskset);
	//add the keyboard input into the liveskset
	FD_SET (0,&liveskset); 
	//set the value of liveskmax
	liveskmax = sock;
  
	while (1) {
    
		/*
		  TODO: 
		  use select() to watch for user inputs and messages from the server
		*/
		//clear the working copy temp
		FD_ZERO(&temp);
		//copy from the saving copy livesket
		temp = liveskset;
		//use select() to watch for user inputs and messages from the server
		select(liveskmax+1, &temp, NULL, NULL, NULL);
		
		/* TODO: message from server */	
		if (FD_ISSET(sock, &temp)) {
		  char *msg;
		  msg = recvdata(sock);
		  if (!msg) {
			/* server died, exit */
			fprintf(stderr, "error: server died\n");
			exit(1);
		  }

		  /* print the message */
		  printf(">>> %s", msg);
		  free(msg);
		}
		
		/* TODO: input from keyboard */
		if (FD_ISSET(0,&temp)) {
		  char msg[MAXMSGLEN];
		  if (!fgets(msg, MAXMSGLEN, stdin))
			exit(0);
			senddata(sock, msg);
		}
	}
}
/*--------------------------------------------------------------------*/
