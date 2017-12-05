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


#define BUFFER_SIZE 1500 //maximum tcp packet size
#define BACKLOG 5 // quantidade de requests até trocar os logs
#define not_blacklisted 1

struct addrinfo hints, *serv_info, host_addr;
struct sockaddr_storage their_addr;

FILE *fd;

/* int verifyWhiteAndBlackList(const char *buffer)
		- Retorna um inteiro indicando se está na blacklist (-1), se está na whitelist(1), ou em nenhuma das listas (0)

		- Buffer da dados será comparado ao arquivo de blacklst e whitelist
*/

int verifyWhiteAndBlackList(const char *buffer)
{
	//Returns 1 case in whitelist
	//Returns -1 case in blacklist
	//0 otherwise

	char *buffer_new = (char *)calloc(BUFFER_SIZE, sizeof(char));

	memcpy(buffer_new, buffer, BUFFER_SIZE);
	char *url = strstr(buffer_new, "GET ");

	if (url != NULL)
	{
		url += sizeof("GET ") - 1;
		char *end = strstr(url, "HTTP") - 1;
		url[end - url] = '\0';
	}
	else
	{
		printf("url==NULL and buffer_new=%s\n", buffer_new);
		return 0;
	}

	FILE *white_file;
	FILE *black_file;
	black_file = fopen("blacklist.txt", "r");
	white_file = fopen("whitelist.txt", "r");
	char line[256];

	if (white_file == NULL)
	{ //arquivo nao existe
		printf("Cant found whitelist.txt\n");
	}
	else
	{ //verifica white list
		while (fgets(line, sizeof(line), white_file))
		{
			int size = strlen(line);
			line[size - 1] = '\0';
			if (strstr(url, line))
			{ //terms is in whitelist
				printf("Term (whitelist): %s\n", line);
				whiteLogWrite(buffer);
				free(buffer_new);
				return 1;
			}
		}
		fclose(white_file);
	}

	if (black_file == NULL)
	{
		printf("Cant found blacklist.tx\n");
	}
	else
	{
		while (fgets(line, sizeof(line), black_file))
		{
			int size = strlen(line);
			line[size - 1] = '\0';
			if (strstr(url, line))
			{
				printf("Term (blacklist): %s\n", line);
				blackLogtWrite(buffer);
				free(buffer_new);
				return -1;
			}
		}
	}

	free(buffer_new);
	return 1;
}

/* int verifyDenyTerm(const char *buffer)
		- Retorna um inteiro indicando se o buffer contém um deny term

		- Buffer da dados será comparado ao arquivo de deny terms
*/

int verifyDenyTerm(const char *buffer)
{
	//returns -1 if found a deny term
	// returns 1 otherwise

	FILE *file_terms = NULL;
	file_terms = fopen("C:/Users/Bruno/Desktop/UnB/UnB - 2 2017/Teleinformática e redes 2/Trab/websocket/src/denyTerms.txt", "r");

	char term[256];

	if (!file_terms)
	{ //arquivo nao existe
		printf("File denyTerms.txt not found\n");
	}
	else
	{
		while (fgets(term, sizeof(term), file_terms))
		{
			int size = strlen(term);
			term[size - 1] = '\0';
			if (strstr(buffer, term))
			{ //termo do arquvo encontrado no buffer
				printf("Deny Term found: %s\n", term);
				deniedLogWrite(fgets(term, sizeof(term), file_terms));
				return -1;
			}
		}
		fclose(file_terms);
	}
	printf("No deny terms found\n");
	return 1;
}

/*void deniedLogWrite(char* term) 
		- Cria e escreve um arquivo de log com os denied terms

		- Denied term a ser escrito no arquivo de log
*/

void deniedLogWrite(char* term) 
{
	FILE* denied_log;
	denied_log = fopen ("websocket/denied_log.txt", "a"); 
	if (denied_log == NULL)
	{
		printf("Error in denied_log file\n");
	} else
	{
		fprintf(denied_log, "Contains denied term: %s\n",term );
		fclose(denied_log);
	}
}

/* void blackLogtWrite(const char* buffer)
		- Cria e escreve um arquivo de log com as url blacklisted

	- Buffer contém a URL da blacklist
*/

void blackLogtWrite(const char* buffer)
{
	FILE* black_log;
	black_log = fopen ("websocket/black_log.txt", "a");
	if (black_log == NULL) 
	{
		printf("Error in black_log file\n");
	} else
	{
		fprintf(black_log, "Tried to request url from blacklist: %s", buffer);
		fclose(black_log);
	}
}

/*void whiteLogWrite(const char* buffer)
	- Cria e escreve um arquivo de log com as url da white list

	- Buffer contém a URL da whitelist
*/

void whiteLogWrite(const char* buffer) 
{
	FILE* log_white;
	log_white = fopen ("websocket/white_log.txt", "a");
	
	if (log_white == NULL) 
	{
		printf("Error in white_log file\n");
	} else
	{
		fprintf(log_white, "Tried to request url from blacklist: %s", buffer);
		fclose(log_white);	
	}
	
}
/* Função de passagem de endereços na socket */
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int directory(char *host, char *path)
/* Essa função cria os diretórios para o proxy cache. Optou-se por fazer dessa forma ao invés
de salvar apenas as URLs em txt para efeitos de controle se a página já foi ou não acessada
e se está gravada em memória, e verificação dos cabeçalhos. A saída fornece uma flag se a página
está em cache (já foi acessada) ou não (primeira vez de acesso). */
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

/*! \fn void connect_host(char *host, char *port, char *path, char *v, int newsockfd)
		\brief Responsável por fazer a conexão com o host, setando seus parametros

		\param host Host passado como parametro ("from host")
		\param port Porta passada como parametro inicial (argc)
		\param path
		\param v
		\param newsockfd Socket que recebe os dados de volta 
*/

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


	if (not_blacklisted == verifyDenyTerm(host))
	{

		if (path != NULL)
			sprintf(buffer, "GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n", path, v, host);
		else
			sprintf(buffer, "GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n", v, host);
		// sending:

		n = send(sockfd1, buffer, strlen(buffer), 0);

		printf("\n%s\n", buffer);

		if (n < 0)
			perror("Error writing to socket");
		else
		{
			do
			{
				bzero((char *)buffer, 500);
				//receive data froms server and store at buffer
				n = recv(sockfd1, buffer, 500, 0);
				fprintf(fd, "%s", buffer);

				if (n < 0)
					perror("Error reading from socket");
				else
					send(newsockfd, buffer, strlen(buffer), 0);
			} while (n > 0);
		}
	}
}

void cache(char *buffer)
/*Carrega o buffer de armazenamento temporariamente para o cache*/
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

int send_file(char *host, char *port, char *path, char *v, int newsockfd)
/* Passagem de arquivos para montagem de novo cabeçalho */
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


// t1= verbo http, t2 = url, t3 = versão do protocolo
void request(int newsockfd)
/* trabalha nas requisições de servidor, armazenamento em buffer pro cache de acordo com os requisitos de rede e filtragem. Deve
retornar um HTML. */
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


	//TODO: verify if blacklisted


	if (verifyWhiteAndBlackList(buffer) == not_blacklisted)
	{
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
	else
	{
		//TODO url blacklisted. Show html?
	}
}


/*! \fn int start_server(int sockfd)
		\brief Responsável por iniciar o socket. Retorna o inteiro retornado pela função accept

		\param sockfd Socket que será iniciado pela função
*/



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





int set_server(const char *Host, const char *service)
/*Configura o servidor chamanado as funções declaradas anteriormente, bem como algumas da biblioteca socket.h
Utiliza como base Socket API */
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
		fprintf(stderr, "Indique a porta para o proxy!!!!\n");
		exit(1);
	}

	sockfd = set_server(NULL, argv[1]);
	newsockfd = start_server(sockfd);
	request(newsockfd);
}
