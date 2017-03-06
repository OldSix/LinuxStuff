#include <stdio.h>
#include <signal.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

pid_t get_server_pid()
{
    int fd = open("chatserver.run", O_RDWR);
    char buf[1024];
    read(fd, buf, sizeof(buf));
    close(fd);

    return (pid_t)atoi(buf);
}

// 等待服务器发送数据
void* thread_recv(void* ptr)
{
    int fd_read = (int)(intptr_t)ptr;
    char buf[4096];
    while(1)
    {
        int ret = read(fd_read, buf, sizeof(buf));
        if(ret == 0) // 写端已经被关闭了
        {
            exit(0); // 整个进程退出
        }
        if(ret < 0)
        {
            if(errno == EINTR) // 读文件失败
                continue;

            exit(0);  // 读文件错误
        }
        printf("%s\n", buf); // 要求buf不带
    }
}

int main()
{
    // 创建两个管道文件
    pid_t pid = getpid();
    char buf1[4096];
    sprintf(buf1, "%d-1", (int)pid);
    mkfifo(buf1, 0777);
    char buf2[4096];
    sprintf(buf2, "%d-2", (int)pid);
    mkfifo(buf2, 0777);


    // 发送信号给服务器，我来了
    pid = get_server_pid();

    // 发送信号告诉服务器，新的客户端加入
    union sigval v;
    sigqueue(pid, SIGRTMIN, v);

    // 打开管道文件，一定在发送信号之后
    // 让客户端和服务器一起打开管道，否则会阻塞
    int fd_write = open(buf1, O_WRONLY);
    int fd_read = open(buf2, O_RDONLY);

    // 创建一个线程，负责信息的接收
    pthread_t tid;
    pthread_create(&tid, NULL, thread_recv, (void*)(intptr_t)fd_read);

    // 等待用户输入
    while(1)
    {
        char buf[4096];
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = 0;
        if(strlen(buf) == 0) // 空敲回车的处理
            continue;

        // setname xue
        // to yy: hello yy
        write(fd_write, buf, strlen(buf)+1); // 带上\0
    }
}




