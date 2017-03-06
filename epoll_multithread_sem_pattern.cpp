#include<sys/epoll.h>
#include<pthread.h>
#include<list>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<semaphore.h>
#include"sock.h"

std::list<int> socks;
int epollfd;

pthread_mutex_t mutex;
sem_t sem;
int quit;

void epoll_add(int epollfd, int fd, int events)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    // always try to modify first
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1)
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void acceptAll(int server)
{
    while (1) {
        int newfd = accept(server, NULL, NULL);
        if (newfd >= 0) {
            set_nonblock(newfd);
            epoll_add(epollfd, newfd, EPOLLIN|EPOLLONESHOT);
        } else {
            if (errno == EAGAIN)
                continue;
            break;
        }
    }
}

void *thread_func(__attribute__((unused))void *arg)
{
    while (1) {
        int ret = sem_wait(&sem);

        if (ret > 0) {
            if (quit == 1 && socks.size() == 0)
                break;

            pthread_mutex_lock(&mutex);
            int fd = *socks.begin();
            socks.pop_front();
            pthread_mutex_unlock(&mutex);

            while (1) {
                char buf[1024];
                ret = read(fd, buf, sizeof(buf));
                if (ret > 0) {
                    printf("buf=%s,fd=%d\n", buf, fd);// work on it
                } else if (ret == 0) {
                    close(fd); // epoll will automatically remove fd from itself.
                    break;
                } else {
                    if (errno == EAGAIN) { // already read all the data, should remove EPOLLONESHOT attr after all.
                        epoll_add(epollfd, fd, EPOLLIN|EPOLLONESHOT);
                    } else {
                        close(fd);
                    }
                    break;
                }
            }

        } else {
            if (errno == EINTR)
                continue;
        }
    }


    return NULL;
}

int main(void)
{
    // server init
    int server = create_server("0.0.0.0", 9988, 250);
    set_nonblock(server);

    // thread init : mutex, sem
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&mutex, &attr);
    sem_init(&sem, 0, 0);// only shared between the threads of a process

    pthread_t tid1;
    pthread_create(&tid1, NULL, thread_func, NULL);
    pthread_t tid2;
    pthread_create(&tid2, NULL, thread_func, NULL);
    pthread_t tid3;
    pthread_create(&tid3, NULL, thread_func, NULL);

    epollfd = epoll_create(512);
    epoll_add(epollfd, server, EPOLLIN);

    struct epoll_event ev_out[8];

    while (1) {
        int ret = epoll_wait(epollfd, ev_out, 8, 2000);
        if (ret > 0) {
            int i;
            for (i = 0; i < ret; ++i) {
                int fd = ev_out[i].data.fd;
                if (fd == server) {
                    acceptAll(fd);
                } else {
                    pthread_mutex_lock(&mutex);
                    socks.push_back(fd);
                    pthread_mutex_unlock(&mutex);

                    sem_post(&sem);
                }
            }
        } else if (ret == 0 || (ret < 0 && errno == EINTR)) {// no fd became ready or interrupted by a signal
            continue;
        } else {// error happen
            printf("error happened..\n");
            exit(1);
        }
    }

    quit = 1;
    sem_post(&sem);
    sem_post(&sem);
    sem_post(&sem);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

    return 0;
}















