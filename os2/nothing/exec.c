#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(void){
	printf("hello world :) pid: %d\n", getpid());
	int rc=fork();

	if (rc<0){
		fprintf(stderr, "fork error :(");
		exit(1);
	}
	else if(rc==0){
		printf("hello, I am child!! pid: %d\n", getpid());
		char *myargs[3];
		myargs[0]=strdup("wc"); //word counter
		myargs[1]=strdup("exec.c"); //
		myargs[2]=NULL; //배열의 끝 표시
		execvp(myargs[0],myargs); //"wc" 실행

		printf("this shouldn't print out");
	}
	else{
		int wc=wait(NULL);
		printf("hello, I am parent of %d!! wc: %d, pid: %d\n", rc, wc, getpid());
	}
	exit(0);
}
