#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <time.h>
#include <utmpx.h>
#include <dirent.h>
#include <errno.h>
#include <ncurses.h>
#include <pwd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

#define STDIN_PIPE 0x1
#define STDOUT_PIPE 0x2

extern int errno;

/* Splits the string by space and returns the array of tokens
*
*/

struct forExec{
	char *argv[MAX_TOKEN_SIZE];
//	struct forExec *next;
	struct forExec *back;
};

char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++){
		char readChar = line[i];
		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		} 
		else {
			token[tokenIndex++] = readChar;
		}
	}
	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}



int usePipe(char **tokens){
	int cnt=0;
	for(int i=0;tokens[i]!=NULL;i++){
		if(!strcmp(tokens[i], "|")) //파이프 사용
			return 0;
	}
	return 1;
}

struct forExec *newNode(struct forExec *exec){
	struct forExec *new = malloc(sizeof(struct forExec));
	new->back=exec;
	return new;
}

float* runTtop(float *pre, int flag){
	char buf[MAX_INPUT_SIZE];           
	char **tokens;
	int fd;
	int hour, min, sec;
	
	DIR *dir_ptr=NULL;
	struct dirent *file=NULL;

	//date
	time_t curTime;
	struct tm *curDate;
	time(&curTime);
	curDate=localtime(&curTime);
	bzero(buf, sizeof(buf)); //바이트스트림 0으로 채움
	strcpy(buf, asctime(curDate));
	tokens=tokenize(buf);
	printf("top - %s up ", tokens[3]);
	//uptime
	if((fd=open("/proc/uptime", O_RDONLY))<0){
		fprintf(stderr, "uptime open error\n");
		exit(1);
	}
	bzero(buf, sizeof(buf)); //바이트스트림 0으로 채움
	read(fd, buf, sizeof(buf));
	tokens=tokenize(buf);
	sec=atoi(tokens[0]);
	int startT=sec;
	min=sec/60;
	hour=min/60;
	sec%=60;
	min%=60;
	printf("%d:%d, ", hour, min);

	//users
	struct utmpx *utx;
	int cnt=0;
	setutxent();
	while((utx=getutxent())!=NULL){
		if(utx->ut_type != USER_PROCESS)
			continue;
		cnt++;
	}
	printf("%d user, ", cnt);
	close(fd);
	//loadAvg
	if((fd=open("/proc/loadavg", O_RDONLY))<0){
		fprintf(stderr, "loadavg open error\n");
		exit(1);
	}
	bzero(buf, sizeof(buf));
	read(fd, buf, sizeof(buf));
	tokens=tokenize(buf);
	printf("load average: %s, %s, %s\n", tokens[0], tokens[1], tokens[2]);
	close(fd);

	//task
	bzero(buf, sizeof(buf));
	strcpy(buf, "/proc/");
	if((dir_ptr=opendir("/proc"))==NULL){
		fprintf(stderr, "/proc opendir error\n");
		exit(1);
	}
	cnt=0;

	int R=0;
	int S=0;
	int T=0;
	int Z=0;

	while((file=readdir(dir_ptr))!=NULL){
		if((int)file->d_name[0]>47 && (int)file->d_name[0]<58){ // /proc에서 pid 갯수
			bzero(buf, sizeof(buf));
			strcpy(buf, "/proc/");
			strcat(buf, file->d_name);
			strcat(buf, "/stat");
			if((fd=open(buf, O_RDONLY))<0){
				fprintf(stderr, "%s open error %d\n", buf, errno);
				exit(1);
			}
			bzero(buf, sizeof(buf));
			read(fd, buf, sizeof(buf));

			close(fd);
			tokens=tokenize(buf);
			

			switch(*tokens[2]){
				case 'R': 
					R++;
					break;
				case 'I':

				case 'S':
					S++;
					break;
				case 'T':
					T++;
					break;
				case 'Z':
					Z++;
					break;
			}
			cnt++;
		}
	}
	closedir(dir_ptr);
	printf("Tasks: %d total, %d running, %d sleeping, %d stopped, %d zombie\n", cnt, R, S, T, Z);


	//%CPU

	
	
	float preSum=0;
	float preCpu[8];


	memcpy(preCpu, pre, sizeof(preCpu));

	if(flag==1){
		for(int j=0; j<8; j++){
			preSum+=preCpu[j];
		}
	}

	if((fd=open("/proc/stat", O_RDONLY))<0){
		fprintf(stderr,"/proc/stat open error\n");
		exit(1);
	}

	bzero(buf, sizeof(buf));
	read(fd, buf, sizeof(buf));
	close(fd);

	char *ptr = strtok(buf, "\n"); //stat에서 한 줄만 추출 

	tokens=tokenize(ptr);

	float sum=0;
	float cpu[8]; //user mode, nice, system mode, idle task

	float *ptrCpu=(float*)malloc(sizeof(float));
	int i=1;

	while(tokens[i]!=NULL){
		cpu[i-1]=atoi(tokens[i]);
		sum+=cpu[i-1];
		i++;
	}
	

	float subCpu[8];

	if(flag==1){
		sum-=preSum;
		for(int i=0; i<8; i++){
			subCpu[i]=cpu[i]-preCpu[i];
		}
	}
	else{
		memcpy(subCpu, cpu, sizeof(cpu));
	}

	ptrCpu=cpu;
	printf("%%CPU(s): %.1f us, %.1f sy, %.1f ni, %.1f id, %.1f wa, %.1f hi, %.1fsi , %.1fst\n",
			(subCpu[0]/sum)*100,(subCpu[2]/sum)*100,(subCpu[1]/sum)*100,(subCpu[3]/sum)*100 ,(subCpu[4]/sum)*100 ,(subCpu[5]/sum)*100 ,(subCpu[6]/sum)*100 ,(subCpu[7]/sum)*100 );


//Mib Mem

	float mem[4];
	float swap[4];
	float kib=0.000976562;
	float memper=0;

	if((fd=open("/proc/meminfo", O_RDONLY))<0){
		fprintf(stderr,"/proc/meminfo open error\n");
		exit(1);
	}
	
	bzero(buf, sizeof(buf)); 
	read(fd, buf, sizeof(buf));
	ptr=strtok(buf, "\n");
	tokens=tokenize(ptr);

	int k=0;

	while(1){
		if(!strcmp(tokens[0], "MemTotal:")){
			mem[0]=atoi(tokens[1]);
			memper=mem[0];
			mem[0]*=kib;
		}
		else if(!strcmp(tokens[0], "MemFree:")){
			mem[1]=atoi(tokens[1]);
			mem[1]*=kib;
		}
		else if(!strcmp(tokens[0], "MemAvailable:")){
			swap[3]=atoi(tokens[1]);
			swap[3]*=kib;
		}
		else if(!strcmp(tokens[0], "Active(anon):")){
			mem[3]=atoi(tokens[1]);
			mem[3]*=kib;
		}
		else if(!strcmp(tokens[0], "SwapTotal:")){
			swap[0]=atoi(tokens[1]);
			swap[0]*=kib;
		}
		else if(!strcmp(tokens[0], "SwapFree:")){
			swap[1]=atoi(tokens[1]);
			swap[1]*=kib;
			break;
		}

		ptr=strtok(NULL, "\n");
		tokens=tokenize(ptr);
		k++;
	}



	mem[2]=mem[0]-mem[1]-mem[3];
	swap[2]=swap[0]-swap[1];


	printf("MiB Mem : %10.1f total, %7.1f free,  %7.1f used, %7.1f buff/cache\n",
			mem[0], mem[1], mem[2], mem[3]);

	printf("MiB Swap: %10.1f total, %7.1f free,  %7.1f used, %7.1f avail Mem\n",
			swap[0], swap[1], swap[2], swap[3]);

	//final
	int pid=0;
	int ni=0;
	int virt=0;
	int res=0;
	int shr=0;
	int rssFile=0;
	int rssShmem=0;
	int time=0;

	char state[2];
	char pr[5];
	char command[50];
	char maxBuf[MAX_INPUT_SIZE*2];
	

	bzero(buf, sizeof(buf));
	strcpy(buf, "/proc/");
	if((dir_ptr=opendir("/proc"))==NULL){
		fprintf(stderr, "/proc opendir error\n");
		exit(1);
	}
	printf("      PID  USER     PR    VIRT     RES     SHR     S      %%CPU    %%MEM        TIME+     COMMAND\n");

	while((file=readdir(dir_ptr))!=NULL){
		if((int)file->d_name[0]>47 && (int)file->d_name[0]<58){ // /proc에서 pid 갯수
			bzero(buf, sizeof(buf));
			strcpy(buf, "/proc/");
			strcat(buf, file->d_name);
			strcat(buf, "/stat");
			if((fd=open(buf, O_RDONLY))<0){ //  proc/pid/stat open
				fprintf(stderr, "%s open error %d\n", buf, errno);
				exit(1);
			}

			bzero(buf, sizeof(buf));
			read(fd, buf, sizeof(buf));
			close(fd);
			tokens=tokenize(buf);
			
			strcpy(state,tokens[2]);
			pid=atoi(tokens[0]);
			if(!strcmp(tokens[17], "-100"))
				strcpy(pr, "rt");
			else
				strcpy(pr, tokens[17]);
			ni=atoi(tokens[18]);

			sec=0;
			min=0;
			hour=0;
			int time=0;
			int total=0;
			float t=0;
			sec=atoi(tokens[13]);
			time=atoi(tokens[14]);
			t=atoi(tokens[21]);
			total=sec+time;

			float cpu=(float)total/((float)startT-t/100);

			time/=100;
			total/=100;
			sec-=time;

			min=sec/60;
			hour=min/60;
			sec%=60;
			min%=60;


			bzero(buf, sizeof(buf));
			strcpy(buf, "/proc/");
			strcat(buf, file->d_name);
			strcat(buf, "/status");

			if((fd=open(buf, O_RDONLY))<0){ //    /proc/pid/status open
				fprintf(stderr, "%s open error %d\n", buf, errno);
				exit(1);
			}
			bzero(maxBuf, sizeof(maxBuf));
			bzero(buf, sizeof(buf));
			read(fd, maxBuf, sizeof(buf));
			close(fd);

			ptr=strtok(maxBuf, "\n");
			
	
			char test[50];
			strcpy(test, ptr);
			strcat(test, "\n");
			virt=0;
			res=0;
			shr=0;
			rssFile=0;
			rssShmem=0;
			float memcpu=0;
			while(1){
				tokens=tokenize(test);
				if(!strcmp(tokens[0], "Name:")){
					strcpy(command, tokens[1]);	
				}
				else if(!strcmp(tokens[0], "VmSize:")){
					virt=atoi(tokens[1]);
				}
				else if(!strcmp(tokens[0], "VmRSS:")){
					res=atoi(tokens[1]);
					memcpu=((float)res/memper)*100;
	
				}
				else if(!strcmp(tokens[0], "RssFile:")){
					rssFile=atoi(tokens[1]);
					ptr=(NULL, "\n");
					rssShmem=atoi(tokens[1]);
					shr=rssFile+rssShmem;
					break;
				}
				else if(!strcmp(tokens[0], "Threads:")){
					break;
				}

				bzero(test, sizeof(test));

				ptr=strtok(NULL, "\n");
				strcpy(test, ptr);
				strcat(test, "\n");
			}
			if(cpu<0)
				cpu=0;
			if(memcpu<0)
				memcpu=0;

			printf("%8d %5s %7d %7d %7d %7d %5s   %6.2f   %6.2f %5d:%02d.%02d   %s\n", pid, pr, ni, virt, res, shr, state, cpu, memcpu, hour, min, sec, command);
		}
	}
	closedir(dir_ptr);

	printf("\n");

	return ptrCpu;
}
void runPs(){
	DIR *dir_ptr=NULL;
	struct dirent *file=NULL;
	char maxBuf[MAX_INPUT_SIZE*2];
	char buf[MAX_INPUT_SIZE];
	int fd;
	char **tokens;
	
	char test[50];

	char bash[30];
	char pps[30]="ps";
	char bashPID[10];
	char psPID[10];
	int clear=0;

	if((dir_ptr=opendir("/proc"))==NULL){
		fprintf(stderr, "/proc opendir error\n");
		exit(1);
	}

	while((file=readdir(dir_ptr))!=NULL){
		if((int)file->d_name[0]>47 && (int)file->d_name[0]<58){ // /proc에서 pid 갯수

			bzero(buf, sizeof(buf));
			strcpy(buf, "/proc/");
			strcat(buf, file->d_name);
			strcat(buf, "/status");
			if((fd=open(buf, O_RDONLY))<0){ //  proc/pid/stat open
				fprintf(stderr, "%s open error %d\n", buf, errno);
				exit(1);
			}

			bzero(maxBuf, sizeof(maxBuf));
			bzero(buf, sizeof(buf));
			read(fd, maxBuf, sizeof(buf));
			close(fd);

			char *ptr=strtok(maxBuf, "\n");
			
	
			char test[50];
			strcpy(test, ptr);
			strcat(test, "\n");

			clear=0;

			while(1){
				tokens=tokenize(test);
				if(!strcmp(tokens[0], "Name:")&& clear==0){
					if(!strcmp(tokens[1], "bash")){
						strcpy(bash, tokens[1]);	
						strcpy(bashPID, file->d_name);
						clear=1;
						break;
					}
					else
						break;
					
				}
				////////////////////

				bzero(test, sizeof(test));

				ptr=strtok(NULL, "\n");
				strcpy(test, ptr);
				strcat(test, "\n");
			}
		}
	}
	
	bzero(buf, sizeof(buf));
	strcpy(buf, "/proc/");
	strcat(buf, bashPID);
	strcat(buf, "/stat");
	if((fd=open(buf, O_RDONLY))<0){ //  proc/pid/stat open
		fprintf(stderr, "%s open error %d\n", buf, errno);
		exit(1);
	}

	bzero(buf, sizeof(buf));
	read(fd, buf, sizeof(buf));
	close(fd);
	tokens=tokenize(buf);
	
	int sec=0;
	int min=0;
	int hour=0;
	int time=0;
	int total=0;
	float t=0;
	char pts[6]="pts/2";// 
	sec=atoi(tokens[13]);
	time=atoi(tokens[14]);
	total=sec+time;

	time/=100;
	total/=100;
	sec-=time;

	min=sec/60;
	hour=min/60;
	sec/=60;
	min%=60;
	closedir(dir_ptr);
	printf(" PID    TTY   TIME      CMD\n");
	printf(" %s  %s  %02d:%02d:%02d %s \n", bashPID, pts, hour, min, sec, bash);
	printf(" %d  %s  %02d:%02d:%02d %s \n", getpid(), pts, hour, min, sec, pps);

}

void run(struct forExec *exec, int flag){ //바로 실행하려면 flag==1
	pid_t pid;
	int fd[2];
	float* preCpu;
	if(exec->back==NULL || flag==1){ //non pipe || right command of pipe

		if(!strcmp(exec->argv[0],"ttop")){
			preCpu=runTtop(preCpu, 0);
			while(1){
				sleep(3);

				preCpu=runTtop(preCpu, 1); 
			}
		}
		else if(!strcmp(exec->argv[0], "pps")){
			runPs();
			exit(1);
		}
		else if((execvp(exec->argv[0], exec-> argv))==-1){
			fprintf(stderr, "SSUShell : Incorrect command\n");
			exit(1);
		}
	}
	else{
		pipe(fd);
		pid=fork();
		if(pid==0){ //child process
			dup2(fd[1], 1);
			close(fd[0]);
			run(exec->back, 0);
		}
		else{ //parent process( grep | pipe)
			dup2(fd[0], 0);
			close(fd[1]);
			run(exec, 1);
		}
		wait(NULL);
	}
}

int main(int argc, char* argv[]) {
	char line[MAX_INPUT_SIZE];            
	char **tokens;              
	char **command1;
	char **command2;
	int i;
	int fd[2];
	int rc;
	pid_t pid;


	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r"); //읽기 전용으로 argv[1] open
		if(fp < 0) {
			printf("File doesn't exists."); //error handling
			return -1;
		}
	}

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line)); //바이트스트림 0으로 채움
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		}
		else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line); //개행이 들어올 때까지
			getchar();
		}
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line); //토큰화 완료
   


		//node header
		struct forExec *exec = malloc(sizeof(struct forExec));
		exec->back=NULL;

		int cnt=-1;

		while(tokens[cnt+1]!=NULL){
			for(int j=0; tokens[cnt+1]!=NULL; j++){
				cnt++;
				if(strcmp(tokens[cnt], "|")) //"|"가 아니라면
					exec->argv[j]=tokens[cnt];
				else //tokens[cnt]=="|" 
					break;
			}
			if(tokens[cnt+1]!=NULL)//new node
				exec=newNode(exec);
		}
		if(fork()==0)
			run(exec, usePipe(tokens));
		wait(NULL);


		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
