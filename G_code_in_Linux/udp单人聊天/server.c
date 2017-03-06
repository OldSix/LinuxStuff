#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

int main()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9988);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while(1)
    {
        char buf[1024];
        recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &addrlen);
        printf("%s\n", buf);

        char sendbuf[4096];
        fgets(sendbuf, sizeof(sendbuf), stdin);
        sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    }

    return 0;
}
