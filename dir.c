int dir(char* host,char *path)
{
	struct stat st = {0};
	char* temporary = NULL;
	char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
					printf("Waiting...\n");
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

	if(temporary == NULL)
	{
	if (stat("Homepage", &st) == -1)
		{
			flag = 1;
		}


		arq = fopen("Homepage","a+");
	}
	else
	{
	if (stat(extension, &st) == -1)
		{
			flag = 1;
		}


	arq = fopen(extension,"a+");
	}
	return flag;
}
