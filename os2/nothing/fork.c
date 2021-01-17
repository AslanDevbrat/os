#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void){
	printf("hello world :) pid: %d\n", getpid());
	int rc=fork();

	if(rc<0){
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(rc==0){ //child process
		printf("hello, i am child :) pid: %d\n", getpid());
	}
	else{	//parent process
		printf("hello, i am parent :) pid: %d\n", getpid());
	}
	exit(0);
}
