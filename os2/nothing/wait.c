#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
	printf("hello world pid:%d\n", getpid());
	int rc=fork();

	if(rc<0){
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(rc==0){
		printf("hello, I am child pid:%d\n", getpid());
	}
	else{
		int wc=wait(NULL);
		printf("hello, I am parent of %d !!! wc:%d, pid:%d\n", rc, wc, getpid());
	}
}
