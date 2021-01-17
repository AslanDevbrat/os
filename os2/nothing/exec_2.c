#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void){
	int rc=fork();

	if (rc<0){
		fprintf(stderr, "fork error :(");
		exit(1);
	}
	else if(rc==0){
		close(STDOUT_FILENO);
		open("exec.output", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);


		char *myargs[3];
		myargs[0]=strdup("wc"); //word counter
		myargs[1]=strdup("exec.c"); //
		myargs[2]=NULL; //배열의 끝 표시
		execvp(myargs[0],myargs); //"wc" 실행

		printf("this shouldn't print out");
	}
	else{
		int wc=wait(NULL);
	}
	exit(0);
}
