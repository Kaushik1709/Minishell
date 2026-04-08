#include "header.h"

char *external_cmds[152];
int pid =0;
int status;
void scan_input(char *prompt, char *input_string)
{
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    extract_external_commands(external_cmds);
    
    while(1)
    {
        printf("%s", prompt);
        scanf("%[^\n]", input_string);
        getchar();
        if(strcasecmp(input_string,"PS1") == 0)
        {
            printf("PS1 is not a Command\n");
            continue;
        }
        if(strncasecmp(input_string, "PS1=", 4) == 0)
        {
            char*new_prompt = input_string + 4;

            if(*new_prompt == ' ')
            {
                printf("PS1 is not a Command\n");
                continue;
            }
            if(*new_prompt == '\0')
            {
                printf("Command not found\n");
                continue;
            }
            strcpy(prompt, new_prompt);
            strcat(prompt, " ");
            continue;
            
        }
        else
        {
            char *cmd = get_command(input_string);
            
            int type= check_command_type(cmd);
            if(type == BUILTIN)
            {
                
                execute_internal_commands(input_string);
            }
            else if(type == EXTERNAL)
            {
                
                pid =fork();

                switch(pid)
                {
                case -1:
                    perror("");
                    break;
                case 0:
                    signal(SIGINT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    execute_external_commands(input_string);
                    exit(0);
                default:
                    waitpid(pid , &status, WUNTRACED);
                    
                }
            }
            
        }
    }
    
}

extern char prompt[25];
extern char input_string[256];
Slist *head = NULL;
void signal_handler(int signum)
{
    if(signum == SIGINT)
    {
        if(pid == 0)
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
        
    }
    if(signum == SIGTSTP)
    {
        if(pid == 0)
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
        else if(pid != 0)
        {
            
           insert_at_first( &head, pid, input_string);
        }
    }
    if(signum == SIGCHLD)
    {
        waitpid(-1, &status, WNOHANG);   
    }
}


int insert_at_first(Slist **head, int pid, char *input_string)
{
    Slist *new = malloc(sizeof(Slist));
    if(new == NULL)
    {
        return -1;
    }
    new->pid = pid;
    strcpy(new->input_string, input_string);
    
    new->link = *head;
    *head = new;
    return 0;
}
void print_list(Slist *head)
{
    if(head == NULL)
    {
        printf("INFO : List is empty\n");
        return ;
    }
    
    int count =1;
    while(head)
    {
        printf("[%d] %d %s\n", count, head->pid, head->input_string);
        head = head->link;
        count++;
    }
}

int delete_first(Slist **head)
{
    if(*head == NULL)
    {
        return -1;
    }
    Slist *temp =*head;
    *head= (*head)->link;
    free(temp);
    return 0;
}