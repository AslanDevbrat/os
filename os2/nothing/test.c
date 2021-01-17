#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
	int x=0;
	fork();
	x=1;
	printf("pid=%d, x=%d\n", getpid(), x);
	exit(0);
}
