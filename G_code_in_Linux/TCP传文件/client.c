#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("argument error\n");
        return -1;
    }

    char* src = argv[1];
    char* dst = argv[2];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    int namelen = strlen(dst) +1;
    send(sock, (char*)&namelen, 4, 0);
    send(sock, dst, namelen, 0);

    struct stat src_stat;
    stat(src, &src_stat);

    int filelen = (int)src_stat.st_size;
    send(sock, (char*)&filelen, 4, 0);

    int srcfd = open(src, O_RDONLY);
    while(filelen > 0)
    {
        char buf[4096];
        int ret = read(srcfd, buf, sizeof(buf));
        if(ret > 0)
        {
            send(sock, buf, ret, 0);
            filelen -= ret;
        }
    }
    close(srcfd);
    return 0;
}
