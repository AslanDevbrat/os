#include "types.h"
#include "stat.h"
#include "param.h"
#include "user.h"
#include "processInfo.h"
 
int main(void)
{
	struct processInfo processInfo;
	int pid=1;
	printf(1,"PID\tPPID\tSIZE\tNumber of Context Switch\n");
	for(int i=0; i<NPROC; i++){
		if((get_proc_info(pid, &processInfo)!=-1)){


			printf(1,"%d\t%d\t%d\t%d\n", processInfo.pid, processInfo.ppid,
					processInfo.psize, processInfo.numberContextSwitches);

		}
			pid++;
	}
	exit();
}
