
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int getfilesize(const char* filename)
{
    struct stat buf;
    stat(filename, &buf);
    return buf.st_size;
}

// 拷贝片段
void copy_frag(const char* srcfile, const char* dstfile, int start, int len)
{
    int fdi = open(srcfile, O_RDONLY);
    int fdo = open(dstfile, O_WRONLY);

    lseek(fdi, start, SEEK_SET);
    lseek(fdo, start, SEEK_SET);
    char buf[4096];

    while(len > 0)
    {
        int copylen = sizeof(buf) < len ? sizeof(buf) : len;

        int ret = read(fdi, buf, copylen);
        if(ret <= 0) break;

        write(fdo, buf, ret);
        len -= ret;
    }

    close(fdi);
    close(fdo);
}

void copyfile(int jobs, const char* srcfile, const char* dstfile)
{
    int filesize = getfilesize(srcfile);
    int fd = open(dstfile, O_RDWR|O_CREAT, 0777);
   // ftruncate(fd, 0);
    close(fd);
   truncate(dstfile, 0);

    // 13 / 4 = 3
    int frag = filesize / jobs;

    int i;
    for(i=0; i<jobs; ++i)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            copy_frag(srcfile, dstfile, frag*i, frag);
            return;
        }
    }

    // 拷贝剩余的部分
    copy_frag(srcfile, dstfile, frag*i, filesize - frag*jobs);

    // 等待子进程拷贝结束
    for(i=0; i<jobs; ++i)
    {
        wait(NULL);
    }
}

// ./a.out -j 4 srcfile dstfile
int main(int argc, char* argv[])
{
    if(argc != 5)
    {
        printf("argument error\n");
        return 1;
    }

    int jobs = atoi(argv[2]);
    const char* srcfile = argv[3];
    const char* dstfile = argv[4];


    copyfile(jobs, srcfile, dstfile);
    return 0;
}
