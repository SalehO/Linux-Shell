#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "shell.h"
#include <time.h>
#include <errno.h>
//#include <signal.h>

static char buffer[BUFFER_SIZE];
static int b =0;

static pid_t bresults = 5;
static pid_t child_p =-1;
static pid_t child_p2=6;
static int ctrl_z =0;



void unix_error(char *msg) 
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void sigtuser2_handler(int sig) 
{
    printf("Hello User\n");
   // exit(0);
}

      
int main(int argc, char *argv[])
{
    sigset_t mask, prev_mask;
    sigemptyset(&mask);   
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);

	int i; //loop counter
	char *pos; //used for removing newline character
	char *args[MAX_TOKENS + 1];
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	pid_t foo  =-1;
    pid_t pip2;
    int fd[2];
    char curr_direc[700];


	for(;;)
    {
        if (signal(SIGUSR2, sigtuser2_handler) == SIG_ERR) 
        unix_error("signal error");
        b = 0;
        if(getcwd(curr_direc,sizeof(curr_direc)) == NULL) //geetinf current working directory path
        {
            fprintf(stderr,"ERROR getting current working directory!\n");
            continue;
        }
        time_t tim = time(NULL);
        struct tm t = *localtime(&tim);
        char* uname = getenv("USER"); 
		printf("\033[01;33m"); 
       // printf("another iteration of for loop");
		printf("<saleho> %d-%d-%d %d:%d @%s %s$ ",t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, uname, curr_direc); //print prompt
        printf("\033[0m"); 
        if(child_p != -1)
        {
            printf("\n");
        }
		fgets(buffer, BUFFER_SIZE, stdin);
		if( (pos = strchr(buffer, '\n')) != NULL){
			*pos = '\0'; //Removing the newline character and replacing it with NULL
		}
		// Handling empty strings.
		if(strcmp(buffer, "")==0)
			continue;
		// Parsing input string into a sequence of tokens
		size_t numTokens;
		*args = NULL;
		numTokens = tokenizer(buffer, args, MAX_TOKENS);
        if(!numTokens){continue;}
		args[numTokens] = NULL;
		if(strcmp(args[0],"exit") == 0) // Terminating the shell
			return 0;


            int k;
            int aflag =0;
            int rflag =0;
            int cflag =0;
            int pflag =0;
            int backup_k;
            int redirection_flag = 0;
            int change_dir =0;
            char* direc;
            //check for invalid compinatipon of redirections and/or piping
            for(k =0; k< numTokens;k++)
            {
                if((strcmp(args[k], "cd") == 0 && k ==0))
                {
                    change_dir =1;
                    direc = args[k+1];
                    break;
                }
                if( k == 0 &&strcmp(args[k], "&") == 0  )
                {
                    fprintf(stderr, "ERROR Wrong use of &\n");
                    cflag =1;
                }
                if(strcmp(args[k], "|") == 0 && pflag ==1)
                {
                        fprintf(stderr, "ERROR! Pipe can only be used once \n");
                        cflag =1;
                        break;
                }
                if(strcmp(args[k], "&>") == 0)
                {
                    backup_k = k;
                }
                if(strcmp(args[k], "&>") == 0)
                {
                    if(k >= backup_k)
                    {
                        fprintf(stderr, "ERROR! Overwrite stderr with file not allowed \n");
                        cflag =1;
                        break;
                    }
                }
                if(rflag == 1 && (strcmp(args[k], "&>") == 0 ))
                {
                    fprintf(stderr, " REDIRECTION ERROR\n");
                    redirection_flag =1;
                    break;
                }


                if(strcmp(args[k], ">") == 0 && rflag == 1)
                {
                    fprintf(stderr,"ERROR!! redirction not valid\n");
                    cflag =1;
                    break;
                   // exit(EXIT_FAILURE);
                }
                if(strcmp(args[k], "&") == 0 && aflag == 1)
                {
                    fprintf(stderr,"ERROR cannot use & twice in the same command!\n");
                    
                    //exit(EXIT_FAILURE);
                    cflag =1;
                    break;
                }
                if(strcmp(args[k], "|") == 0)
                {
                    pflag =1;
                }
                if(pflag == 1 && (strcmp(args[k], "<") == 0 ))
                {
                    fprintf(stderr,"ERROR, piping can only be used by itself!\n");
                    cflag =1;
                    break;
                }
                



                if(pflag == 1 && (strcmp(args[k], ">") == 0 ))
                {
                    fprintf(stderr,"ERROR, piping can only be used by itself!\n");
                    cflag =1;
                    break;
                }



                if(pflag == 1 && (strcmp(args[k], "&>") == 0 ))
                {
                    fprintf(stderr,"ERROR, piping can only be used by itself!\n");
                    cflag =1;
                    break;
                }


                if(pflag == 1 && (strcmp(args[k], "2>") == 0 ))
                {
                    fprintf(stderr,"ERROR, piping can only be used by itself!\n");
                    cflag =1;
                    break;
                }

                if(pflag == 1 && (strcmp(args[k], ">>") == 0 ))
                {
                    fprintf(stderr,"ERROR, piping can only be used by itself!\n");
                    cflag =1;
                    break;
                }
                if(rflag == 1 && (strcmp(args[k], ">>") == 0 ))
                {
                    fprintf(stderr,"ERROR, redirection not valid!!\n");
                    cflag =1;
                    break;
                }
                if(strcmp(args[k], ">") == 0 || strcmp(args[k], "&>") == 0 || strcmp(args[k], ">>") == 0 )
                {
                    rflag =1;
                }
                if(strcmp(args[k], "&") == 0)
                {
                    aflag =1;
                }
            }
            if(redirection_flag == 1)
            {
                continue;
            }
            if(cflag ==1)
            {
                continue;
            }

            if(change_dir ==1)
            {
                if(chdir(direc) ==0)
                {
                   // fprintf(stderr,"scuccesfully changed directory\n");
                    continue;
                }
                else
                {
                    //fprintf(stderr,"failed to change the directory\n");
                    continue;
                }
                
            }
        if(child_p != -1)
        {
            pid_t rresult = waitpid(child_p, &exit_status, WNOHANG);
        
            if(rresult == -1)
            {
                fprintf(stderr,"Error while waiting!!\n");
                exit(EXIT_FAILURE);
            }
            else if(rresult == child_p)
            {
               // printf("Backgreound process done!\n");
                kill(child_p,SIGKILL);
                child_p = -1;
            }
            else
            {
                //printf("Background is still running!\n");
            }
            //continue;
        }
        //if background process then fork and run in child and the parent waits to reaps the child without hanging
        if(*args[numTokens-1] == '&')
        {
            args[numTokens-1] = NULL;
            numTokens--;

           
			//printf("background===========================\n\n");
            if(child_p != -1 && b ==0)
            {
                fprintf(stderr,"ERORR, Can only run one background at a time\n\n");
                continue;
            }
            else
            {
				b=1;
            }
            
        }
		if(b == 1 && child_p == -1)
		{
           
			child_p = pid = fork();
			
		} 
		 else
         {
           
		 	pid = fork();	
         }

		if (pid == 0)//inside the main child process
        { 		
            int v ;
            int pipe_flag = 0;
            //handles output and input redirections
            for (v =0; v<numTokens;v++) //check if there is multiple & then error and continue
            {
                if(strcmp(args[v], ">") == 0)
                {
                   // printf("redirection!!\n");
                   int fd1 = open(args[v+1],  O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
                   if(fd1 == -1)
                   {
                       fprintf(stderr,"ERROR with file!\n");
                       exit(EXIT_FAILURE);
                      // continue;
                   }
                    dup2(fd1, STDOUT_FILENO);
                    close(*args[v+1]);
                    args[v] = NULL;

                }
                else if(strcmp(args[v], ">>") == 0)
                {
                   // printf("redirection!!\n");
                   int fd1 = open(args[v+1], O_RDWR|O_CREAT|O_APPEND, 0600);
                   if(fd1 == -1)
                   {
                       fprintf(stderr,"ERROR with file!\n");
                       exit(EXIT_FAILURE);
                      // continue;
                   }
                    dup2(fd1, STDOUT_FILENO);
                    close(*args[v+1]);
                    args[v] = NULL;

                }
                else if (strcmp(args[v], "2>") == 0)
                {
                   // printf("redirection!!\n");
                   int fd1 = open(args[v+1], O_RDWR|O_CREAT|O_APPEND, 0600);
                   if(fd1 == -1)
                   {
                       fprintf(stderr,"ERROR with file!\n");
                       exit(EXIT_FAILURE);
                      // continue;
                   }
                    dup2(fd1, STDERR_FILENO);
                    close(*args[v+1]);
                    args[v] = NULL;
                }
                else if(strcmp(args[v], "&>") == 0)
                {
                   // printf("redirection!!\n");
                   int fd1 = open(args[v+1],  O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
                   if(fd1 == -1)
                   {
                       fprintf(stderr,"ERROR with file!\n");
                       exit(EXIT_FAILURE);
                      // continue;
                   }
                    dup2(fd1, STDOUT_FILENO);
                    dup2(fd1, STDERR_FILENO);
                    close(*args[v+1]);
                    
                    args[v] = NULL;
                }
                else if(strcmp(args[v], "<") == 0)
                {
                   // printf("redirection!!\n");
                   int fd1 = open(args[v+1], O_RDONLY);
                   if(fd1 == -1)
                   {
                       fprintf(stderr,"ERROR file cannot be located!\n");
                       exit(EXIT_FAILURE);
                      // continue;
                   }
                    dup2(fd1, STDIN_FILENO);
                    close(*args[v+1]);
                    args[v] = NULL;
                }
                else if(strcmp(args[v], "|") == 0 )
                {
                    //printf("PIPPING!!\n");
                    pipe_flag =1;
                    args[v] = NULL;
                    break;
                    
                }

            }
            
            if(pipe_flag ==1)
            {
                pipe(fd);
                pip2 =fork();
                
                if(pip2 ==0)
                {
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    exec_result = execvp(args[0], &args[0]);
                    close(fd[0]);
                    close(fd[1]);
                    fprintf(stderr,"\nCannot execute %s. An error occured.\n", args[0]);
                   exit(EXIT_FAILURE);
                    
                }
            else
            {
                    if(waitpid(pip2,&exit_status, 0) == -1)
                    {fprintf(stderr, "ERROR while waiting!!\n");
                    exit(EXIT_FAILURE);}
                    dup2(fd[0], STDIN_FILENO);
                     close(fd[0]);
                    close(fd[1]);
                    exec_result = execvp(args[v+1], &args[v+1]);
                    close(fd[0]);
                    close(fd[1]);
                    fprintf(stderr,"\nCannot execute %s. An error occured.\n", args[v+1]);
                    exit(EXIT_FAILURE);
                }

            }


             if(pipe_flag !=1)
             {
    		  exec_result = execvp(args[0], &args[0]);	
    			if(exec_result == -1)
                { //Error checking
    				fprintf(stderr,"\nCannot execute %s. An error occured.\n", args[0]);
    				exit(EXIT_FAILURE);
		        }
                exit(EXIT_SUCCESS);
             }
		}
		 else
         { // Parrent Process
            if(b == 0)//regular processs
            {
                //printf("Regular lets wait\n" );
    			wait_result = waitpid(pid, &exit_status, 0);
    			if(wait_result == -1  )
                {
    				fprintf(stderr,"An error ocured while waiting for the process.\n");
    				exit(EXIT_FAILURE);
    			}
            }
		}
	}
	return 0;
}

//tokenize arguments and add them to array
size_t tokenizer(char *buffer, char *argv[], size_t argv_size)
{
    char *p, *wordStart;
    int c;
    enum mode { DULL, IN_WORD, IN_STRING } currentMode = DULL;
    size_t argc = 0;
    for (p = buffer; argc < argv_size && *p != '\0'; p++) {
        c = (unsigned char) *p;
        switch (currentMode) {
        case DULL:
            if (isspace(c))
                continue;
            // Catching "
            if (c == '"') {
                currentMode = IN_STRING;
                wordStart = p + 1;
                continue;
            }
            currentMode = IN_WORD;
            wordStart = p;
            continue;
        // Catching "
        case IN_STRING:
            if (c == '"') {
                *p = 0;
                argv[argc++] = wordStart;
                currentMode = DULL;
            }
            continue;
        case IN_WORD:
            if (isspace(c)) {
                *p = 0;
                argv[argc++] = wordStart;
                currentMode = DULL;
            }
            continue;
        }
    }
    if (currentMode != DULL && argc < argv_size)
        argv[argc++] = wordStart;
    return argc;
}
