#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>


int main(int argc, char *argv[]){
	char buf[128];
	pid_t pid;

	while(1){
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf)-1, stdin);

		buf[strlen(buf)-1]='\0';
		if(!strncmp(buf, "exit", strlen(buf)))
			return -1;

		pid=fork();

		if(pid<0){
			perror("fork error\n");
			return -1;
		}
		else if(pid==0){
			execlp(buf, buf, NULL);
			exit(0);
		}

		else
			wait(NULL);
	}
	return 0;
}
