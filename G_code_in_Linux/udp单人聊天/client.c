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
    int sock = socket(AF_INET,SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9989);
    addr.sin_addr.s_addr = INADDR_ANY; 

    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9988);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while(1)
    {
        char buf[1024];
        fgets(buf, sizeof(buf), stdin);
        sendto(sock, buf, sizeof(buf), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        char recvbuf[4096];
        recv(sock, recvbuf, sizeof(recvbuf), 0);
        printf("%s\n", recvbuf);
    }
    return 0;
}
