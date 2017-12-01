#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include <netdb.h>
#include<sys/types.h>
#include<netinet/in.h>

#define BACKLOG 5

struct addrinfo hints,*serv_info,host_addr;
struct sockaddr_storage their_addr;
FILE* fd;


int main(int argc,char **argv)
{
	int sockfd,newsockfd;

	if (argc<2)
	{
		fprintf (stderr,"Indique a porta para o proxy\n");
		exit(1);
	}

	sockfd=set_server(NULL,argv[1]);
	newsockfd=start_server(sockfd);
	request(newsockfd);
}
