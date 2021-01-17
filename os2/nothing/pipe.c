#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF 1024
#define READ 0
#define WRITE 1

int main(void){
	int fd[2];
	char buf[MAX_BUF];
	pid_t pid;

	if(pipe(fd)<0){
		fprintf(stderr, "pipe error && minjeong chulchul\n");
		exit(1);
	}

	if((pid=fork())<0){
		fprintf(stderr, "fork error :(\n");
		exit(1);
	}
	printf("\n");
	if(pid>0){ //parent process
		close(fd[READ]);
		strcpy(buf, "I AM YOUR FATHER\n");
		write(fd[WRITE], buf, strlen(buf));
	}
	else{ //child
		close(fd[WRITE]);
		read(fd[READ], buf, MAX_BUF);
		printf("child got message: %s\n", buf);

	}
	exit(0);
}
