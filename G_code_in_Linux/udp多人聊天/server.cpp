#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <map>
#include <string>

using namespace std;

int sock;

map<string, struct sockaddr_in> usermap;
struct msg
{
    char buf[4096];
    struct sockaddr_in client_addr;
};

void* handle_msg(void* ptr)
{
   struct msg* m = (struct msg*)ptr;
   //setname:xxx
   //from xxx:to yyy:content
   char* ptype = strtok(m->buf, ":");
   if(strcmp(ptype, "setname") == 0)
   {
       string name = string(strtok(NULL, ":"));
       usermap[name] = m->client_addr;
   }
   else
   {
       char* p = strtok(NULL, ":"); //to yyy

           char* content = strtok(NULL,"\0");
       char* q = strtok(p, " ");
       q = strtok(NULL, " ");
       string dstname = string(q);
       auto user = usermap.find(dstname);
       if(user == usermap.end())
       {
           sendto(sock, "no such user\n", strlen("no such user\n") + 1, 0, (struct sockaddr*)&m->client_addr, sizeof(m->client_addr));
       }
       else
       {
           char buf[4096];
           sprintf(buf, "%s:%s\n", ptype, content);
           sendto(sock, buf, strlen(buf) +1, 0 ,(struct sockaddr*)&user->second, sizeof(user->second));
       }
   }
}

int main()
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9988);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    char buf[4096];
    while(1)
    {
        recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &addrlen);
        struct msg* m = (struct msg*)malloc(sizeof(struct msg));
        strcpy(m->buf, buf);
        m->client_addr = client_addr;
        pthread_t pid;
        pthread_create(&pid, NULL, handle_msg, (void*)m);
        pthread_detach(pid);
    }
    return 0;
}
