/*
** broadcaster.c -- a datagram "client" like talker.c, except
**                  this one can broadcast
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT 15500	// the port users will be connecting to

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in their_addr; // connector's address information
	struct hostent *he;
	int numbytes;
	int broadcast = 1;
	//char broadcast = '1'; // if that doesn't work, try this

	if (argc != 4) {
		fprintf(stderr,"usage: broadcaster hostname port message\n");
		fprintf(stderr,"hostname can be 255.255.255.255 for broadcast\n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
		perror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	// this call is the difference between this program and talker.c:
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
		sizeof broadcast) == -1) {
		perror("setsockopt (SO_BROADCAST)");
		exit(1);
	}

	their_addr.sin_family = AF_INET;	 // host byte order
	their_addr.sin_port = htons( atoi(argv[2]) ); // short, network byte order
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

	if ((numbytes=sendto(sockfd, argv[3], strlen(argv[3]), 0,
			 (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
		perror("sendto");
		exit(1);
	}

	printf("sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));

	close(sockfd);

	return 0;
}
