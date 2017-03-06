#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 250);

    int fd = accept(sock, NULL, NULL);

    int namelen;
    recv(fd, (char*)&namelen, 4, 0);
    char name[1024];
    recv(fd, name, namelen, 0);

    int dstfd = open(name, O_CREAT|O_RDWR, 0777);

    int filelen;
    recv(fd, (char*)&filelen, 4, 0);

    char buf[4096];
    while(filelen > 0)
    {
        int ret = recv(fd, buf, sizeof(buf), 0);
        if(ret > 0)
        {
            write(dstfd, buf, ret);
            filelen -= ret;
        }
    }
    close(dstfd);
    close(fd);
    return 0;
}
