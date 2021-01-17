#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


#define STDIN_PIPE 0x1
#define STDOUT_PIPE 0x2


void exec_command(char *command, int pipefd[2], int flags) {
        pid_t cpid = fork();
        if (cpid > 0)
                return;
        else if (cpid < 0)
                perror("fork");

        if (flags & STDIN_PIPE)
                if (dup2(pipefd[0], STDIN_FILENO) < 0)
                        perror("dup2");

        if (flags & STDOUT_PIPE)
                if (dup2(pipefd[1], STDOUT_FILENO) < 0)
                        perror("dup2");

        close(pipefd[0]);
        close(pipefd[1]);

        char *const argv[] = {command, NULL};
        char *const envp[] = {NULL};
        execve(command, argv, envp);
        perror("execv");
}


int main() {
        int pipefd1[2], pipefd2[2];
        pipe(pipefd1);
        pipe(pipefd2);

        exec_command("/bin/ls", pipefd1, STDOUT_PIPE);
        close(pipefd1[1]);
        exec_command("/usr/bin/wc", pipefd2, STDIN_PIPE);
        close(pipefd2[0]);

        int temp_pipefd[] = {pipefd1[0], pipefd2[1]};
        exec_command("/usr/bin/wc", temp_pipefd, STDIN_PIPE | STDOUT_PIPE);

        close(pipefd1[0]);
        close(pipefd2[1]);

        int wstatus;
        while (wait(&wstatus) > 0);

        return 0;
}
