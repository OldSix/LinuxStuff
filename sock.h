#ifndef __SOCK_H__
#define __SOCK_H__

#ifdef __cplusplus
extern "C" {
#endif
    #include <fcntl.h>
    #include <stdio.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    
    static int connect_server(const char *ip, unsigned short port)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            perror("connect server..");
            return 1;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);

        int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
        if (ret < 0) {
            close(fd);
            return -1;
        }
        return fd;
    }

    static int create_server(const char *ip, unsigned short port, int backlog)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            perror("create socket..");
            return fd;
        }
    
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);
    
        int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
        if (ret < 0) {
            perror("bind..");
            return -2;
        }
    
        listen(fd, backlog);
    
        return fd;
    }
    
    
    static int do_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
    {
        while (1) {
            int newfd = accept(fd, addr, addrlen);
            if (newfd < 0 && errno == EINTR) {
                continue;
            } 
            return newfd;
        }
    }
    
    /*
     * 这个函数要读取size个字节再返回
     *
     */
    static int do_recv(int fd, char *buf, int size)
    {
        int already_read = 0;
        while (size > 0) {
            int ret = read(fd, buf + already_read, size);
            if (ret > 0) {
                size -= ret;
                already_read += ret;
            } else if (ret == 0) {// 对方关闭socket
                break;
            } else if (ret < 0) {
                if (errno == EINTR)
                    continue;
                break;
            }
        }
    
        return already_read;
    }
    
    
    static int do_send(int fd, const char *buf, int size)
    {
        int already_send = 0;
        while (size > 0) {
            int ret = write(fd, buf + already_send, size);
            if (ret > 0) {
                size -= ret;
                already_send += ret;
            } else if (ret < 0) {
                if (errno == EINTR)
                    continue;
                break;
            }
        }
    
        return already_send;
    }
    
    static void set_nonblock(int fd)
    {
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(fd, F_SETFL, flags);
    }

#ifdef __cplusplus
}
#endif


#endif // __SOCK_H__



