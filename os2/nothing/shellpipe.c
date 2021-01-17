#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define READ 0
#define WRITE 1

int main(int argc, char *argv[]){
	char str[1024];
	char **command1, **command2;
	int fd[2];
	pid_t pid;

	printf("[shell]");
	fgets(str,sizeof(str), stdin);
	str[strlen(str)-1]='\0';

	if(strchr(str, '|')!=NULL){ //파이프 사용
		command1=strtok(str, "|");
		command2=strtok(NULL, "|");
	}

	pipe(fd);
	printf("cmd1: %s, cmd: %s\n", command1, command2);

	pid=fork();
	printf("pid : %d\n", pid);

	if(pid==0){
		close(fd[READ]);
		dup2(fd[WRITE], 1); //쓰기용 파이프를 표준출력에 복제
		close(fd[WRITE]);
		printf("자식\n");
		execlp(command1, command1, NULL);
	}
	else{
		wait(NULL);
		close(fd[WRITE]);
		dup2(fd[READ], 0); //읽기용 파이프를 표준입력에 복제
		close(fd[READ]);
		printf("부모\n");
		execlp(command2, command2, NULL);
	}

}
