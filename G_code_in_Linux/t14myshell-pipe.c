
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// ls
// mkdir aaa
// cp ../aa bb
// cd  内置命令
// ps aux | grep a.out
//

// 需要不需要重定向，是输入重定向，还是输出重定向，重定向到哪里
// int fd表示重定向的文件描述符号，如果是-1表示不需要重定向
// int target表示重定向的目标，应该是0或者1
void handle_simple_cmd(char* cmd, int fd, int target)
{
    char* args[1024];
    char* p = strtok(cmd, " ");
    int i = 0;
    while(p)
    {
        args[i++] = p;
        p = strtok(NULL, " ");
    }
    args[i] = NULL; // 表示参数结束位置

    if(strcmp(args[0], "cd") == 0)
    {
        // 切换当前目录
        chdir(args[1]);
        return;
    }

    pid_t pid = fork();

    if(pid == 0)
    {
        if(fd != -1) // 表示需要重定向
            dup2(fd, target); // target一定是0，或者1

        execvp(args[0], args);
        // 如果命令执行失败，应该让子进程退出
        printf("invalid command\n");
        exit(0);
    }
    else
    {
        wait(NULL); 
    }
}

void handle_cmd(char* cmd, int fd, int target);

// ps aux | grep a.out | awk '{print $2}' | xargs kill -kill
// ps aux | grep a.out
void handle_pipe_cmd(char* cmd)
{
    // ps aux 和 grep a.out
    char* cmd1 = strtok(cmd, "|");
    char* cmd2 = strtok(NULL, "\0");

    // cmd1 = ps aux;
    // cmd2 = grep a.out 
    // 主进程执行 cmd1
    // 启动一个子进程B执行 cmd2
    // 主进程执行cmd1时，需要启动一个子进程，这个子进程应该重定向标准输出
    // 子进程B的标准输入，应该重定向到管道读端
    //
    int fd[2];
    pipe(fd);

#if 0
    pid_t pidA = fork();
    if(pidA == 0)
    {
        // A 进程
        // 先重定向
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        handle_cmd(cmd1);

        exit(0);
    }
#endif
    // 主进程执行cmd1
    handle_cmd(cmd1, fd[1], 1);

    pid_t pidB = fork();
    if(pidB == 0)
    {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);

        handle_cmd(cmd2, -1, -1);

        // B进程
        exit(0);
    }

    close(fd[0]);
    close(fd[1]); // 主进程也要关闭管道的两端

    // 等待A，B进程的结束
    wait(NULL);
    wait(NULL);
}

// 处理命令
void handle_cmd(char* cmd, int fd, int target)
{
    // 看命令是不是有|，决定调用哪个函数
    if(NULL == index(cmd, '|'))
    {
        handle_simple_cmd(cmd, fd, target);
        return;
    }

    // ps aux | grep a.out
    handle_pipe_cmd(cmd);
}

int main()
{
    while(1)
    {
        printf("myshell> ");
        // 等待用户输入
        char buf[4096];
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = 0; // remove \n

        if(strlen(buf) == 0)
        {
            continue;
        }

        handle_cmd(buf, -1, -1);
    }
}
