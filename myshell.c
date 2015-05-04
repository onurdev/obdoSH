#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

struct hist_data{
	struct hist_data *next;
	struct hist_data *prev;
	char name[];
};
struct hist_data *first_hist;

typedef void (*sighandler_t)(int);
char c = '\0';
char  line[100];
void fork_exec(char line[], char *envp[]);
char** parse_line(char line[]);
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
	
	char line[100];
	size_t size;
	int stringLength;

	int i=0;
	while(c != EOF) {
		c = getchar();

		if (c == '\033') { // if the first value is esc
		    getchar(); // skip the [
		    switch(getchar()) { // the real value
		        case 'A':
		            fflush(stdout);
		            printf("\33[2K\rUP\n");
		            fflush(stdout);
		            fflush(stdout);
		            fflush(stdout);
		            fflush(stdout);
		            fflush(stdout);
		            fflush(stdout);
		            fflush(stdout);
		            break;
		        case 'B':
		            // code for arrow down
		            break;
		        case 'C':
		            // code for arrow right
		            break;
		        case 'D':
		            // code for arrow left
		            break;
		    }
		}

		line[i]=c;
		i++;
		if(c == '\n'){
			line[i-1]='\0';
			i=0;
			if(checkBuiltInFunctions(line) == 0) break;
            fork_exec(line,envp);
			printf("obdoSH -> ");
		}

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

void fork_exec(char line[], char *envp[]){
    
	char **args;
	args=parse_line(line);

    int id=fork();
    
    if(id==0) {
		
		printf("child executing\n");
	    execvpe(args[0],args,envp);
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
char** parse_line(char line[]){
	printf("parsing line : \"%s\"\n",line );
	
	int i=0,space_count=0;
	
	while(line[i]!='\0'){
		if(line[i]==' ')
			++space_count;
		++i;
	}
	char** args = calloc(space_count+1, sizeof(char*));

	i=0;
	int j=0;
	int prev=0;
	

	for(i=0;;++i){
		if(line[i]==' ' || line[i]=='\0'){
			
			args[j]=calloc(i-prev+1, sizeof(char));
			int k=0;
			for(;prev<i;++prev){
				args[j][k++]=line[prev];
			}
			args[j][k++]='\0';
			++j;
			++prev;
		}
		if(line[i]=='\0')break;
	}
	return args;
}
