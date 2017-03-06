
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// ls
// mkdir aaa
// cp ../aa bb
// cd
void handle_cmd(char* cmd)
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

        handle_cmd(buf);
    }
}
