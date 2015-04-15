#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

typedef void (*sighandler_t)(int);
char c = '\0';
char  line[100];
void fork_exec(char line[], char *argv[], char *envp[]);
void handle_signal(int signo)
{
	//printf("\n[sig%d]\n",signo);
	fflush(stdout);
	exit(0);
}

int main(int argc, char *argv[], char *envp[])
{
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);
	//printf("[start][MY_SHELL ] ");
	char* welcomeMessage = "\
This is a simple open source shell. You may find the source code at https://github.com/onurdev/obdoSH\n\
Licence: MIT Licence\n\
Authors: Osman Sekerlen, Onur Baris Dev.\n\n";
	printf(welcomeMessage);
	printf("obdoSH -> ");
	
	char *line = NULL;
	size_t size;
	int stringLength;
    while (getline(&line, &size, stdin) != -1) {
        stringLength = (int)strlen (line);
        line[stringLength-1]='\0';
        
		if(checkBuiltInFunctions(line) == 0) break;
		
        fork_exec(line,argv,envp);
        printf("obdoSH -> ");
    }
    
	return 0;
}

// if return value is nonzero there is a problem
int checkBuiltInFunctions(char line[]){
	int result = 0;
    // exit the shell if exit command is entered.
	result = strcmp(line, "exit\0");
	
	return result;
}

void fork_exec(char line[], char *argv[], char *envp[]){
    
    int id=fork();
    
    if(id==0) {
		
		printf("child executing\n");
		
	    execvpe(line,argv,envp);
		printf("child couldn't be executed\n");
		exit(0);
		
    } else if(id>0) {
		
		int status;
		printf("waiting\n");
	    
	    waitpid(id,&status,0);
		printf("waited\n");
		
    } else {
    	
		printf("id<0\n");
    }



}

