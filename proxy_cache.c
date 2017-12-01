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

int directory(char* host,char *path)
{
	struct stat st = {0};
	char* temporary = NULL;
	char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
					printf("..........\n");
        else
           	perror("getcwd() error");

	mkdir("cache", 0700);
	chdir("cache");
	mkdir(host,0700);
	chdir(host);
	temporary = strtok(path,"/");
	char *extension;
	int flag = 0;

	while(temporary!= NULL)
	{
		if ((stat(temporary, &st) == -1) && (temporary!=NULL))
		{
		    mkdir(temporary, 0700);
		    flag = 1;
		}
		chdir (temporary);
		extension = temporary;
		temporary = strtok(NULL,"/");
	}

	if(temporary==NULL)
	{
	if (stat("Homepage", &st) == -1)
		{
			flag=1;
		}


		arq = fopen("Homepage","a+");
	}
	else
	{
	if (stat(extension, &st) == -1)
		{
			flag=1;
		}


	arq = fopen(extension,"a+");
	}
	return flag;
}

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
