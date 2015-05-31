#include <stdio.h>
#include <stdlib.h>


char** set(){
char** args=calloc(2 , sizeof(char*));
int i,j;
for(i=0;i<2;++i){
		args[i] = calloc(5 , sizeof(char));
		for (j = 0; j < 3; ++j)
		{	
		char* s="a";;
		args[i][j]=s[0];
		printf("%c, ",s[0]);
		}
		args[i][3]='\0';
	}
	printf("\n\n\n");
	return args;
}


int main(){
	char **args;
	//args=calloc(2 , sizeof(char*));	
	args=set();
	int i,j;

	for(i=0;i<2;++i){
		for (j = 0; j < 3; ++j)
		{	
		printf("%c, ",args[i][j]);
		}
		printf("\n");
	}
	printf("\n\n\n");
	
	for ( i = 0; i < 2; ++i)
	{
		printf("%s\n", args[i]);
	}
}
