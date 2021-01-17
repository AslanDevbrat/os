#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>


int main(void){
	char *cwd=(char*)malloc(sizeof(char)*1024); //동적할당
	DIR *dir=NULL;
	struct dirent *entry=NULL;

	getcwd(cwd, 1024); //현재 작업 directory 가져옴

	if((dir=opendir(cwd))==NULL){
		fprintf(stderr, "current directory error\n");
		exit(1);
	}

	while((entry=readdir(dir))!=NULL){
		printf("%s\n", entry->d_name);
	}

	free(cwd);
	closedir(dir);

	exit(0);
}
