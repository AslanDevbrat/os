#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void charatatime(char *str);

int main(void){
	pid_t pid;

	if((pid=fork())<0){
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(pid==0)
		charatatime("output from child\n");
	else
		charatatime("output from parent\n");

	exit(0);
}

static void charatatime(char *str){
	char *ptr;
	int print_char;

	setbuf(stdout, NULL); //버퍼링 사라짐


	for(ptr=str; (print_char =  *ptr++)!=0;){
		putc(print_char, stdout);
		usleep(10);
	}
}
