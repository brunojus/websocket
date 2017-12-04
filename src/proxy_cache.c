#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "blacklist.h"

#define BUFFER_SIZE 1500 //maximum tcp packet size
#define BACKLOG 5
#define not_blacklisted 1

struct addrinfo hints, *serv_info, host_addr;
struct sockaddr_storage their_addr;

FILE *fd;

int verifyDenyTerm(const char* buffer) {
	//returns -1 if found a deny term
	// returns 1 otherwise

	FILE  *fterms;
	fterms = fopen ("denyTerms.txt", "r");
	char term[256];


	if (!fterms ) { //arquivo nao existe
		printf("File denyTerms.txt not found\n");
	}
	else { //verifying whitelist
		while (fgets(term, sizeof(term), fterms)) {
			int size = strlen (term);
			term[size-1] = '\0'; //apagar o \n do fgets
			if (strstr(buffer,term)) { //TERM FOUND IN BUFFER
				printf("Deny Term found: %s\n", term);
				return -1;
			}
		}
		fclose (fterms);
	}
	printf("No deny terms found\n");
	return 1;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void request(int newsockfd)
{

	char s[INET6_ADDRSTRLEN];
	char buffer[BUFFER_SIZE];

	int flag = 0, n, i;
	char t1[512], t2[512], t3[16];
	char *temporary = NULL;
	char *port;
	bzero(buffer, BUFFER_SIZE);
	recv(newsockfd, buffer, BUFFER_SIZE, 0);
	sscanf(buffer, "%s %s %s", t1, t2, t3);

	printf("%s\n\n\n\n\n\n\n\n\n", t1);
	printf("%s\n\n\n\n\n\n\n\n\n", t2);
	printf("%s\n\n\n\n\n\n\n\n\n", t3);

	//TODO: verify if blacklisted

	if (((strncmp(t1, "GET", 3) == 0)) && ((strncmp(t3, "HTTP/1.1", 8) == 0) || (strncmp(t3, "HTTP/1.0", 8) == 0)) && (strncmp(t2, "http://", 7) == 0))
	{
		strcpy(t1, t2);
		flag = 0;
		for (i = 7; i < strlen(t2); i++)
		{
			if (t2[i] == ':')
			{
				flag = 1;
				break;
			}
		}
		temporary = strtok(t2, "//");

		if (flag == 0)
		{
			port = "80";
			temporary = strtok(NULL, "/");
		}
		else
		{
			temporary = strtok(NULL, ":");
		}
		sprintf(t2, "%s", temporary);

		printf("host = %s", t2);

		if (flag == 1)
		{
			temporary = strtok(NULL, "/");
			port = temporary;
		}

		strcat(t1, "^]");
		temporary = strtok(t1, "//");
		temporary = strtok(NULL, "/");

		if (temporary != NULL)
			temporary = strtok(NULL, "^]");
		printf("\npath = %s\nPort = %s\n", temporary, port);
		i = directory(t2, temporary);

		if (i == 1)
		{
			printf("From Host\n");
			connect_host(t2, port, temporary, t3, newsockfd);
		}
		else
		{
			printf("From cache\n");
			send_file(t2, port, temporary, t3, newsockfd);
		}
	}

	else
	{
		send(newsockfd, "400 : BAD REQUEST\nONLY HTTP REQUESTS ALLOWED", 44, 0);
	}
}

void connect_host(char *host, char *port, char *path, char *v, int newsockfd)
{
	struct addrinfo *p;
	int newsocket1, sockfd1, n;
	memset(&host_addr, 0, sizeof(host_addr));
	host_addr.ai_family = AF_UNSPEC;
	host_addr.ai_socktype = SOCK_STREAM;
	char s[INET6_ADDRSTRLEN];
	char buffer[1024];

	int rv;
	if ((rv = getaddrinfo(host, port, &host_addr, &serv_info)) != 0)
	{

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = serv_info; p != NULL; p = p->ai_next)
	{
		//addrinfo->ainext : ponteiro para o prox addrinfo
		if ((sockfd1 = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd1, p->ai_addr, p->ai_addrlen) == -1)
		{
			//second parameter: addrinfo
			// se o status da conexao for 0, ok. se for -1, alguma parte nao pode conectar apropriadamente, close
			close(sockfd1);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

	sprintf(buffer, "\nConnected to %s  IP - %s\n", host, s);
	printf("\n%s\n", buffer);

	if (not_blacklisted == verifyDenyTerm(buffer))
	{

		if (path != NULL)
			sprintf(buffer, "GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n", path, v, host);
		else
			sprintf(buffer, "GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n", v, host);
		// sending:

		n = send(sockfd1, buffer, strlen(buffer), 0);

		printf("\n%s\n", buffer);

		if (n < 0)
			error("Error writing to socket");
		else
		{
			do
			{
				bzero((char *)buffer, 500);
				//receive data froms server and store at buffer
				n = recv(sockfd1, buffer, 500, 0);
				fprintf(fd, "%s", buffer);

				if (n < 0)
					error("Error reading from socket");
				else
					send(newsockfd, buffer, strlen(buffer), 0);
			} while (n > 0);
		}
	}
}

int start_server(int sockfd)
{
	printf("Server Start Running.........\n");
	char s[INET6_ADDRSTRLEN];
	socklen_t sin_size;
	int newsockfd, pid;
	while (1)
	{
		sin_size = sizeof their_addr;
		newsockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (newsockfd == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

		printf("Connected client:\n");
		printf("server:got connection from %s\n", s);

		pid = fork();
		if (pid == 0)
			break;
	}
	return newsockfd;
}

int directory(char *host, char *path)
{
	struct stat st = {0};
	char *temporary = NULL;
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		printf("..........\n");
	else
		perror("getcwd() error");

	mkdir("cache", 0700);
	chdir("cache");
	mkdir(host, 0700);
	chdir(host);
	temporary = strtok(path, "/");
	char *extension;
	int flag = 0;

	while (temporary != NULL)
	{
		if ((stat(temporary, &st) == -1) && (temporary != NULL))
		{
			mkdir(temporary, 0700);
			flag = 1;
		}
		chdir(temporary);
		extension = temporary;
		temporary = strtok(NULL, "/");
	}

	if (temporary == NULL)
	{
		if (stat("Homepage", &st) == -1)
		{
			flag = 1;
		}

		fd = fopen("Homepage", "a+");
	}
	else
	{
		if (stat(extension, &st) == -1)
		{
			flag = 1;
		}

		fd = fopen(extension, "a+");
	}
	return flag;
}

int send_file(char *host, char *port, char *path, char *v, int newsockfd)
{
	struct addrinfo host_addr, *servinfo;
	struct addrinfo *p;
	int newsocket1, sockfd1, n;

	memset(&host_addr, 0, sizeof(host_addr));
	host_addr.ai_family = AF_UNSPEC;
	host_addr.ai_socktype = SOCK_STREAM;
	char s[INET6_ADDRSTRLEN];
	char buffer[1024];
	int rv;

	bzero((char *)buffer, 1024);
	cache(buffer);
	send(newsockfd, buffer, strlen(buffer), 0);
}

void cache(char *buffer)
{
	char temporary[128];
	while (1)
	{

		if ((fgets(temporary, 128, fd)) == NULL)
			break;
		strcat(buffer, temporary);
	}

	return;
}

int set_server(const char *Host, const char *service)
{
	struct addrinfo *p;

	int sim = 1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int sockfd;
	int i;
	int n;

	n = getaddrinfo(Host, service, &hints, &serv_info);

	if (n != 0)
	{
		fprintf(stderr, "getaddrinfo %s\n", gai_strerror(n));
		return 1;
	}

	for (p = serv_info; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("server:socket\n");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sim, sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server:bind");
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "server: falha ao ligar\n");
		return 2;
	}
	freeaddrinfo(serv_info);

	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	return sockfd;
}

int main(int argc, char **argv)
{
	int sockfd, newsockfd;

	if (argc < 2)
	{
		fprintf(stderr, "Indique a porta para o proxy:\n");
		exit(1);
	}

	sockfd = set_server(NULL, argv[1]);
	newsockfd = start_server(sockfd);
	request(newsockfd);
}