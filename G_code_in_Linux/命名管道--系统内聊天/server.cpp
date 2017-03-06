#include <signal.h>
#include <stdio.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/epoll.h>

#include <map>
#include <iostream>
#include <string>
using namespace std;

typedef struct chat_user_t
{
    string name;
    int fd_read;
    int fd_write;
} chat_user_t;

// 保存着所有的用户信息
map<string, chat_user_t*> users;

int epollfd;

const char* errmsg[] = {
    "ok",
    "user not exit",
    "unknown command"
};

void set_nonblock(int fd)
{
    uint32_t flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

// 信号处理函数
void handle_usr_signal(int sig, siginfo_t* info, void* p)
{
    // 发送信号的进程id
    // info->si_pid
    // 得到进程号就可以打开管道文件了
    char path1[1024];
    char path2[1024];
    sprintf(path1, "%d-1", (int)info->si_pid);
    sprintf(path2, "%d-2", (int)info->si_pid);

    chat_user_t* user = new chat_user_t;
    user->fd_read = open(path1, O_RDONLY);
    if(user->fd_read < 0)
    {
        delete user;
        return;
    }
    // 为了保证主进程只有一个阻塞点
    // 一般来说，一旦程序用了多路IO复用技术，那么这些所有的io都应该设置成非阻塞模式
    set_nonblock(user->fd_read);


    user->fd_write = open(path2, O_WRONLY);
    if(user->fd_write < 0)
    {
        close(user->fd_read);
        delete user;
        return;
    }

    // map的插入
    // usersfd.insert(std::pair<int, chat_user_t*>(user->fd_read, user));
    // usersfd[user->fd_read] = user;

    // 每当有用户加入聊天时，就把它加入epoll中
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = user;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, user->fd_read, &ev);

    printf("some one come in\n");
}

// 注册信号处理函数
void register_usr_signal()
{
    struct sigaction act;
    act.sa_handler = NULL;
    act.sa_sigaction = handle_usr_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO; // 可以接收发送端的参数
    act.sa_restorer = NULL;

    sigaction(SIGRTMIN, &act, NULL);
}

// 处理用户发送过来的消息
void handle_msg(char* msg, chat_user_t* user)
{
    // 是setname命令
    // trim 去掉前面和后面的空格,\t
    // setname xxx yyy
    if(strncmp(msg, "setname", 7) == 0)
    {
        strtok(msg, " ");
        char* username = strtok(NULL, " ");
        user->name = username;

        // 修改users数据结构
        users[user->name] = user;
        printf("%s setname\n", username);
    }
    else if(strncmp(msg, "to", 2) == 0)
    {
        // 发送信息
        char* header = strtok(msg, ":");
        char* content = strtok(NULL, "\0");

        char* to = strtok(header, " ");
        char* tousername = strtok(NULL, " ");

        if(strcmp(tousername, "all") == 0)
        {
            // to all: 
            return;
        }

        chat_user_t* touser = NULL;
        auto it = users.find(tousername);
        if(it == users.end()) // 用户不存在
        {
            // 如果用户不存在，那么就回错误信息给发送者
            write(user->fd_write, errmsg[1], sizeof(errmsg[1])); 
        }
        else
        {
            touser = it->second;

            char buf[4096];
            // from xxx: hello yy
            sprintf(buf, "from %s: %s", user->name.c_str(), content);
            write(touser->fd_write, buf, strlen(buf)+1);
        }
    }
    else 
    {
            // 如果用户不存在，那么就回错误信息给发送者
            write(user->fd_write, errmsg[2], sizeof(errmsg[2])); 
    }
}

void write_processid(const char* programname)
{
    // 程序名.run
    char buf[1024];
    sprintf(buf, "%s.run", programname);

    int fd = open(buf, O_RDWR|O_CREAT|O_EXCL, 0777);
    if(fd < 0)
    {
        exit(0);
    }

    sprintf(buf, "%d", (int)getpid());
    write(fd, buf, strlen(buf));
    close(fd);
}

int main(int argc, char* argv[])
{
    // 把服务器的进程id写入配置文件
    write_processid(argv[0]);

    epollfd = epoll_create(1024);
    // 把文件描述符放入epoll的工作，在信号处理函数中完成
    //
    register_usr_signal();


    struct epoll_event ev[8];
    while(1)
    {
        // 监听所有的用户的fd_read
        int ret = epoll_wait(epollfd, ev, 8, 2000);
        if(ret > 0) // 有人发信息
        {
            for(int i=0; i<ret; ++i)
            {
                chat_user_t* user = (chat_user_t*)ev[i].data.ptr;
                
                // 这个buffer会影响程序的规格，意味着这个聊天
                // 程序一次发言不能超过4096
                char buf[4096];
                int ret = read(user->fd_read, buf, sizeof(buf));
                if(ret == 0 || (ret < 0 && errno != EINTR))
                {
                    printf("client exit");
                   // usersfd.erase(user->fd_read);
                    users.erase(user->name);
                    close(user->fd_read);  // close会自动将文件描述符从epoll集合中删除
                    close(user->fd_write);
                    delete user;
                    continue;
                }

                // 处理buf
                // buf的内容可能的格式:
                // setname xxx
                // to yy: hello yy
                // to all: hello all
                // list
                handle_msg(buf, user);
            }
        }
        else if(ret < 0)
        {
            if(errno != EINTR)
            {
                break; // 退出程序
            }
            else
            {
                printf("interupt by signal\n");
            }
        }
    }
}

