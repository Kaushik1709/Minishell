#include "header.h"

char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "fg", "bg", "jobs", NULL};


char cmd[256];

char *get_command(char *input_string)
{
	int i=0;
	
	while(input_string[i] != ' ' && input_string[i] != '\0')
	{
		cmd[i] = input_string[i];
		i++;
	}
	cmd[i] = '\0';
	return cmd;
}

extern char *external_cmds[152];
int check_command_type(char *command)
{
	for(int i=0; builtins[i] != NULL; i++)
	{
		if(strcmp(command , builtins[i]) == 0)
		{
			return BUILTIN;
		}
	}

	for(int i=0; external_cmds[i] != NULL; i++)
	{
		if(strcmp(command , external_cmds[i]) == 0)
		{
			return EXTERNAL;
		}
	}
	return NO_COMMAND;
}

void extract_external_commands(char **external_cmds)
{
	FILE *fp= fopen("external.txt", "r");
	if(fp == NULL)
	{
		perror("fopen");
		return;
	}
	char buff[256];
	int i=0;
	while(fgets(buff, sizeof(buff), fp) && i < 152)
	{
		buff[strcspn(buff, "\r\n")] = '\0'; 

		external_cmds[i] = malloc(strlen(buff) +1);
		if(external_cmds[i] == NULL)
		{
			perror("malloc");
			fclose(fp);
			return;
		}

		strcpy(external_cmds[i], buff);
		i++;
	}
	fclose(fp);
	external_cmds[i] = NULL;

}

extern int status;
extern Slist *head;
void execute_internal_commands(char *input_string)
{
	/*1. exit     2. pwd      3. cd 
	  4. echo $$  5. echo $?  6. echo $SHELL
	  7. jobs 	  8.fg        9.bg
	 */
	if(strcmp (input_string , "exit") == 0)
	{
		exit(0);
	}
	else if(strcmp (input_string , "pwd") == 0)
	{
		char buff[50];
		getcwd(buff,50);
		printf("%s\n", buff);
	}
	else if(strncmp(input_string ,"cd", 2) == 0)
	{
		char buff[50];
		chdir(input_string + 3);
		getcwd(buff,50);
		printf("%s\n", buff);
	}
	else if(strcmp (input_string , "echo $$") == 0)
	{
		printf("%d\n", getpid() );
	}
	else if(strcmp (input_string , "echo $?") == 0)
	{
		if(WIFEXITED(status))
		{
			printf("%d\n", WEXITSTATUS(status));
		}
		else if(WIFSIGNALED(status))
		{
			printf("%d\n", 128 + WTERMSIG(status));
		}
	}
	else if(strcmp (input_string , "echo $SHELL") == 0)
	{
		printf("%s\n", getenv("SHELL") );
	}
	else if(strcmp (input_string, "jobs") == 0 )
	{
		print_list(head);
	}
	else if(strcmp(input_string, "fg") == 0)
	{
		if(head == NULL)
		{
			printf("Error: no such job\n");
			return;
		}
		kill(head->pid, SIGCONT);
		waitpid(head->pid, &status, WUNTRACED);
		if(!WIFSTOPPED(status))
		{
			delete_first(&head);
		}
		
	}
	else if(strcmp(input_string, "bg") == 0)
	{
		if(head == NULL)
		{
			printf("Error: no such job\n");
			return;
		}
		signal(SIGCHLD, signal_handler);
		kill(head->pid, SIGCONT);
		delete_first(&head);
	}
	else
	{
    	int pid = fork();
    	if(pid == 0)
    	{
        	execute_external_commands(input_string);
        	perror("execvp");
        	exit(1);
    	}
    	waitpid(pid, &status, 0);
	}
}

void execute_external_commands(char *input_string)
{
	char argv2[50][10];
	char *argv[50];
	
	
	int i=0; // index for input string(column)
	int k=0; // token count(row)

	while(input_string[i] != '\0')
	{
		int j=0; // index for char for every string
		while(input_string[i] == ' ') // skip spaces
		i++;

		if(input_string[i] == '\0')
		break;

		while(input_string[i] != ' ' && input_string[i] != '\0' && j < 9)
		{
			argv2[k][j++] = input_string[i++];
		}
		argv2[k][j]= '\0'; // terminate token
		argv[k] = argv2[k];
		k++;
	}
	argv[k] = NULL;
	
	int cmd[k];
	cmd[0] = 0;
	int ind =1;
	int flag =0;

	for(int i=1; i < k; i++ )
	{
		if(strcmp (argv[i] , "|") == 0)
		{
			flag =1;
			argv[i] = NULL;
			cmd[ind++] = i+1;
		}
	}
	if(flag ==0)
	{
		execvp(argv[0], argv);
        perror("execvp");
        exit(1);
	}
	/*-------- pipe execution -------*/
	int fd[2];
    for(int i =0; i < ind; i++)
    {
        if(ind-1 != i)
        {
            pipe(fd);
        }

        int pid = fork();

        if(pid > 0) // parent process
        {
            if(ind-1 != i)
            {
                dup2(fd[0],STDIN_FILENO );
                close(fd[0]);
                close(fd[1]);
            }
        }

        if(pid == 0) //child process
        {
            if(ind-1 != i)
            {
                close(fd[0]);
                dup2(fd[1],STDOUT_FILENO );
            }
            execvp( argv[cmd[i]], argv + (cmd[i]) );
			perror("execvp");
			_exit(1);
        }
    }
	for(int i = 0; i < ind; i++)   //  wait for all children
    {
        wait(NULL);
    }
}