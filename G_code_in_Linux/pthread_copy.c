#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

struct Args
{
    char src[1024];
    char dst[1024];
    int start;
    int fraglen;
};

void* pthread_func(void* ptr)
{
    struct Args* args = (struct Args*)ptr;
    int fdi = open(args->src, O_RDONLY);
    int fdo = open(args->dst, O_WRONLY);

    lseek(fdi, args->start, SEEK_SET);
    lseek(fdo, args->start, SEEK_SET);
   
    char buf[4096];
    while(1)
    {
        int copylen = args->fraglen < sizeof(buf) ? args->fraglen : sizeof(buf);
        int ret = read(fdi, buf,copylen); 
        if(ret <= 0)
        {
            break;
        }
        write(fdo, buf, ret);
        args->fraglen -= ret;
    }

    close(fdi);
    close(fdo);
    free(args);
}

void copyfile(const int n, const char* src, const char* dst)
{
    struct stat src_stat;
    stat(src, &src_stat);

    int srclen = src_stat.st_size;
    int fraglen = srclen / n;

    int fd = open(dst, O_CREAT|O_RDWR);
    ftruncate(fd, srclen);
    close(fd);

    pthread_t* tids = (pthread_t*)malloc(sizeof(pthread_t) * n);

    int i = 0;
    while(i < n)
    {
        struct Args* args = (struct Args*)malloc(sizeof(struct Args));
        strcpy(args->src, src);
        strcpy(args->dst, dst);
        args->start = i * fraglen;
        args->fraglen = fraglen;
        pthread_create(tids + i, NULL, pthread_func, (void*)args);
        ++i;
    }

    
        struct Args* args = (struct Args*)malloc(sizeof(struct Args));
        strcpy(args->src, src);
        strcpy(args->dst, dst);
        args->start = i * fraglen;
        args->fraglen = srclen - n * fraglen;

        pthread_func((void*)args);

        for(int j = 0; j < n; ++j)
        {
            pthread_join(tids[j], NULL);
        }

        free(tids);
}


// ./a.out n a.c b.c
int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        printf("argument error\n");
        return -1;
    }

    const int n = atoi(argv[1]);
    const char* src = argv[2];
    const char* dst = argv[3];

    copyfile(n, src, dst);
    return 0;
}
