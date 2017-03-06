#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


int count = 0;

void regular_count(const char* path)
{
    DIR* dir = opendir(path);
    if(dir == NULL)
    {
        perror("opendir");
        return;
    }
    while(1)
    {
        struct dirent* de =  readdir(dir);
        if(de == NULL)
        {
            break;
        }
        if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            continue;
        }
        if(de->d_type == DT_REG)
        {
            ++count;
        }
        else if(de->d_type == DT_DIR)
        {
            char buf[1024];
            sprintf(buf, "%s/%s",path,de->d_name);
            regular_count(buf);
        }
    }
    closedir(dir);
}

// ./a.out dir
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("argument error\n");
        return -1;
    }

    const char* path = argv[1];

    regular_count(path);
    printf("count is %d\n", count);
    return 0;
}
