hello: hello.c myshell.c
	gcc -o hello.o hello.c
	gcc -o mysh.o myshell.c
clean:
	rm -f hello.o, mysh.o