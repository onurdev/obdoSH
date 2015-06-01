#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <termios.h>     //termios, TCSANOW, ECHO, ICANON
#include <unistd.h>     //STDIN_FILENO
#include <stdlib.h>

#define ESC 27
int isInputValid(char c);
void printCommand(char command[], int curPos);
void setCharAt(int cursorPosition, char toChar, char currentCommand[]);
void removeCharAt(int cursorPosition, char currentCommand[]);
void moveCursorToLeftBy(int n);

struct hist_data{
	struct hist_data *next;
	struct hist_data *prev;
	char line[100];
};
struct hist_data *first_hist = NULL;
struct hist_data *currentHistoryAddress = NULL;

typedef void (*sighandler_t)(int);
char c = '\0';
char  line[100];
void fork_exec(int fdi,int fdo,char line[]);
char** parse_line(char line[]);
int checkBuiltInFunctions(char line[]);
void add_history(char* line);
void handle_signal(int signo);
void setPreviousCommandInHistory(char command[]);
void setNextCommandInHistory(char command[]);
void changeCommand(char line[], char current[], char old[], int* cursorPos);

int main(int argc, char *argv[])
{
	int c;
    static struct termios oldt, newt;
    
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);

    /*tcgetattr gets the parameters of the current terminal
     STDIN_FILENO will tell tcgetattr that it should write the settings
     of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;
    
    /*ICANON normally takes care that one line at a time will be processed
     that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);
    
    /*Those new settings will be set to STDIN
     TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    
    /*This is your part:
     I choose 'e' to end input. Notice that EOF is also turned off
     in the non-canonical mode*/
    char currentCommand[100] = "";
    int cursorPosition = 0;
    char line[100]="", oldCommand[100] = "";

char* welcomeMessage = "\
This is a simple open source shell. You may find the source code at https://github.com/onurdev/obdoSH\n\
Licence: MIT Licence\n\
Authors: Osman Sekerlen, Onur Baris Dev.\n\n";
	printf("%s", welcomeMessage);

    printCommand(currentCommand, cursorPosition);

    while((c=getchar())!= EOF){
        //printf("\n%c - %d\n", c, c);
        if (c == 127 && cursorPosition > 0)
        {
            removeCharAt(cursorPosition - 1, line);
            memset(currentCommand, 0, 100 * (sizeof currentCommand[0]) );
            strcpy(currentCommand, line);
            cursorPosition--;
            //printf("\n%c - %d\n", c, c);
        }
        if (c == 10)
        {
            line[strlen(line)]='\0';
            add_history(line);
			if(checkBuiltInFunctions(line)==0){   
            fork_exec(0, 1, line);
        }
            //reset current command
            memset(currentCommand, 0, 100 * (sizeof currentCommand[0]) );
            memset(line, 0, 100 * (sizeof line[0]) );
            cursorPosition = 0;
        }
        if (c == '\033') { // check the first char == esc
            //printf("\nIN ESC VALUES\n");
            getchar(); // skip '[' char its not being read anyway...
            switch(getchar()) { // the real value
                case 'A': // up arrow
                	setPreviousCommandInHistory(oldCommand);
                	changeCommand(line, currentCommand, oldCommand, &cursorPosition);
                    break;
                case 'B': // down arrow
                    setNextCommandInHistory(oldCommand);
                    changeCommand(line, currentCommand, oldCommand, &cursorPosition);
                    break;
                case 'C': // right arrow
                    if (cursorPosition < strlen(currentCommand)) cursorPosition++;
                    break;
                case 'D': // left arrow
                    if (cursorPosition > 0) cursorPosition--;
                    break;
            }
        } else {
            if (isInputValid(c) && strlen(line) < 70){
                setCharAt(cursorPosition, c, line);
                cursorPosition++;
                memset(currentCommand, 0, 100 * (sizeof currentCommand[0]) );
                strcpy(currentCommand, line);
            } else if (strlen(line) == 70){
                char* warning = "The number of characters in a line cannot exceed 70!";
                printf("\n%s\n", warning);
            }
        }

        // uncomment following line to see each char pressed...
        //printf("%c", c);
        printCommand(currentCommand, cursorPosition);
    }
    
    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    
    return 0;
}

void changeCommand(char line[], char current[], char old[], int* cursorPos){
	memset(current, 0, 100 * (sizeof current[0]) );
	if( strcmp(old, "")!=0)
		strcpy(current, old);
	else strcpy(current, line);
	*cursorPos = strlen(current);
}

// if return value is nonzero there is a problem
int checkBuiltInFunctions(char line[]){
    // exit the shell if exit command is entered.
	if( strcmp(line, "exit\0")==0){
		exit(0);
		return 1;
	}
	if(strcmp(line, "history\0")==0){
		puts("");
		struct hist_data *curr_hist=first_hist;
		int i=0;
		while(curr_hist != NULL){
			printf("%d: %s\n",i++,curr_hist->line );
			curr_hist=curr_hist->next;
		}
		return 1;
	}
	else return 0;
}

void fork_exec(int fdi,int fdo,char line[]){
    if (strchr(line,'|') == NULL){
 	  	//printf(" | not found\n");
		char **args;
		args=parse_line(line);

	    int id=fork();
	    
	    if(id==0) {
			
				 /* Close up standard input of the child */                
                /* Duplicate the input side of pipe to stdin */
                dup2(fdi,0);
			
			
				 /* Close up standard output of the child */
                /* Duplicate the output side of pipe to stdout */
                dup2(fdo,1);
			
			//printf("child executing\n");
		    execvp(args[0],args);
			printf("child couldn't be executed\n");
			exit(0);
			
	    } else if(id>0) {
			
			int status;
			//printf("waiting\n");		    
		    waitpid(id,&status,0);
			//printf("waited\n");
			
	    } else {
	    	
			printf("id<0\n");
	    }

	}else{
		//printf(" | found\n");
		char* pipe_place = strchr(line,'|');
		*pipe_place = '\0';
		char* first_proc_line = line;
		line = pipe_place+1;
		//printf("first: %s\n",first_proc_line );
		//printf("rest: %s\n",line );
		int fd[2];
		
		pipe(fd);
		
		char **args;
		args=parse_line(first_proc_line);
		int id=fork();
	    
	    if(id==0) {
		
				 /* Close up standard input of the child */                
                /* Duplicate the input side of pipe to stdin */
                dup2(fdi,0);
		
		
				 /* Close up standard output of the child */
                /* Duplicate the output side of pipe to stdout */
                dup2(fd[1],1);
                /*close the other end of the pipe*/
                close(fd[0]);

			
			//printf("child executing\n");
		    execvp(args[0],args);
			//printf("child couldn't be executed\n");
			exit(0);
			
	    } else if(id>0) {

	    	close(fd[1]);
			fork_exec(fd[0],1,line);
			
			int status;
			//printf("waiting\n");		    
		    waitpid(id,&status,0);
			//printf("waited\n");
			

	    } else {
			printf("id<0\n");
	    }


	}

}
char** parse_line(char line[]){
	//printf("parsing line : \"%s\"\n",line );
	
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

void add_history(char* line){
	
	if(first_hist == NULL){
		first_hist=(struct hist_data*)malloc(sizeof(struct hist_data));
		strcpy(first_hist->line,line);
		first_hist->next = NULL;
		first_hist->prev = NULL;
		currentHistoryAddress = first_hist;
	}else{

		struct hist_data *curr_hist=first_hist->next;
		struct hist_data *prev_hist=first_hist;
		while(curr_hist != NULL){
			prev_hist=curr_hist;
			curr_hist=curr_hist->next;
		}
		
		// add the item to the last
		curr_hist=(struct hist_data*)malloc(sizeof(struct hist_data));
		strcpy(curr_hist->line,line);
		prev_hist->next=curr_hist;
		curr_hist->prev=prev_hist;
		curr_hist->next=NULL;

		// update current history to last one for scrolling it.
		currentHistoryAddress = curr_hist;
	}
}

void setPreviousCommandInHistory(char command[]){
	if (currentHistoryAddress != NULL) {
		strcpy(command, currentHistoryAddress->line);
		if(currentHistoryAddress->prev != NULL)
			currentHistoryAddress = currentHistoryAddress->prev;
	} else strcpy(command, "");
}
void setNextCommandInHistory(char command[]){
	if(currentHistoryAddress != NULL){
		if(currentHistoryAddress->next != NULL){
			currentHistoryAddress = currentHistoryAddress->next;
			strcpy(command, currentHistoryAddress->line);
		} else strcpy(command, "");
	}
}

void handle_signal(int signo)
{
	//printf("\n[sig%d]\n",signo);
	fflush(stdout);
	exit(0);
}

void setCharAt(int cursorPosition, char toChar, char currentCommand[]){
    int i = 0, commandLength = strlen(currentCommand);
    if (commandLength == cursorPosition)
        currentCommand[commandLength] = toChar;
    else if (commandLength > cursorPosition){
        while (i < strlen(currentCommand) - cursorPosition) {
            int lastPos = strlen(currentCommand) - i;
            currentCommand[lastPos] = currentCommand[lastPos-1];
            i++;
        }
        currentCommand[cursorPosition] = toChar;
    }
}

void removeCharAt(int cursorPosition, char currentCommand[]){
    char removedCharCommand[100] = "";
    int i = 0, j=0;
    while(i < strlen(currentCommand)){
        if (i != cursorPosition){
            removedCharCommand[j] = currentCommand[i];
            j++;
        }
        i++;
    }
    strcpy(currentCommand, removedCharCommand);
}

void printCommand(char command[], int curPos){
    printf( "%c[2K", ESC ); //clears the entire line
    //printf( "%cJ", ESC ); //clears the lines below
    moveCursorToLeftBy(100); // moves cursor left by 100 chars (max line length)
    //simply to left most
    printf("obdosh -> %s", command);
    // after each key typed, set cursor to where it was...
    moveCursorToLeftBy((strlen(command) - curPos));
}

void moveCursorToLeftBy(int n){
    if(n > 0) printf( "%c[%dD", ESC, n);
}

int isInputValid(char c) {
    if (// alphabet
        c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' ||
        c == 'f' || c == 'g' || c == 'h' || c == 'i' || c == 'j' ||
        c == 'k' || c == 'l' || c == 'm' || c == 'n' || c == 'o' ||
        c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' ||
        c == 'u' || c == 'v' || c == 'w' || c == 'x' || c == 'y' || 
        c == 'z' || c == 'A' || c == 'B' || c == 'C' || c == 'D' || 
        c == 'E' || c == 'F' || c == 'G' || c == 'H' || c == 'I' || 
        c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' || 
        c == 'O' || c == 'P' || c == 'Q' || c == 'R' || c == 'S' || 
        c == 'T' || c == 'U' || c == 'V' || c == 'W' || c == 'X' || 
        c == 'Y' || c == 'Z' ||
        // other characters
        c == '`' || c == '1' || c == '2' || c == '3' || c == '4' ||
        c == '5' || c == '6' || c == '7' || c == '8' || c == '9' ||
        c == '0' || c == '-' || c == '=' || c == '~' || c == '!' ||
        c == '@' || c == '#' || c == '$' || c == '%' || c == '^' ||
        c == '&' || c == '*' || c == '(' || c == ')' || c == '_' ||
        c == '+' || c == '[' || c == ']' || c == '\\' || c == ';' ||
        c == '\'' || c == ',' || c == '.' || c == '/' || c == '{' ||
        c == '}' || c == '|' || c == ':' || c == '"' || c == '<' ||
        c == '>' || c == '?' || c == ' '
        ) {
        return 1;
    }
    return 0;
}