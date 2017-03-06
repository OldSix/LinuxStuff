#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


using namespace std;

int main()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9989);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9988);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char name[1024];
    pid_t pid = fork();
    if(pid == 0)
    {
        while(1)
        {
            char buf[4096];
            recv(sock, buf, sizeof(buf), 0);
            printf("%s\n", buf);
        }
    }
    else
    {
        while(1)
        {
            char buf[4096];
            fgets(buf, sizeof(buf), stdin);
            buf[strlen(buf)-1]='\0';
            if(strlen(buf) == 0)
                continue;

            if(strncmp(buf, "set", 3) == 0)
            {
                sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
                char* p = strtok(buf, ":");
                p=strtok(NULL, ":");
                strcpy(name, p);
            }
            else
            {
                char temp[4096];
                sprintf(temp, "from %s:%s", name, buf);
                sendto(sock, temp, strlen(temp)+1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
            }
        }
            
    }
    return 0;
}
