#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define READ 0
#define WRITE 1
#define MAX_BUF 1024

int main(int argc, char* argv[]){
	char str[MAX_BUF];
	char **command1, **command2;
	int fd[2];

	printf("[shell]");
	fgets(str,sizeof(str), stdin);
	str[strlen(str)-1]='\0';

	if(strchr(str, '|')!=NULL){
		*command1=strtok(str, "|");
		*command2=strtok(NULL, "|");
	}
	printf("cmd1: %s, cmd2: %s\n", command1, command2);
	pipe(fd);

	if(fork()==0){
		close(fd[READ]);
		dup2(fd[WRITE], 1); //쓰기용 파이프를 표준출력에 복제
		close(fd[WRITE]);
		execvp(command1[0], command1);
		perror("pipe");
	}
	else{
		close(fd[WRITE]);
		dup2(fd[READ], 0); //읽기용 파이프를 표준입력에 복제
		close(fd[READ]);
		execvp(command2[0], command2);
		perror("pipe");
	}
}
