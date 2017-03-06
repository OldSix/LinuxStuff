
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
       #include <dirent.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <unistd.h>

// 打包
// ./tar dir dstfile
// 解包
// ./tar dstfile 
//
// /home/xueguoliang/aaa
// /home/xueguoliang/aaa/bbb/ccc.c
void tarfile(const char* root, const char* filepath, FILE* fp)
{
    // 将文件名写入，将文件的尺寸写入
    fprintf(fp, "%s\n", filepath+strlen(root));

    // 将文件尺寸写入
    struct stat stat_buf;
    stat(filepath, &stat_buf);
    fprintf(fp, "%d\n", (int)stat_buf.st_size);

    // 将文件内容写入
    FILE* f = fopen(filepath, "r");
    char buf[4096];
    while(1)
    {
        int ret = fread(buf, 1, sizeof(buf), f);
        if(ret <= 0) break;

        fwrite(buf, ret, 1, fp);
    }
}

void tardir(const char* root, const char* dirpath, FILE* fp)
{
    DIR* dir = opendir(dirpath);
    struct dirent* entry = NULL;
    while(1)
    {
        entry = readdir(dir);
        if(entry == NULL)
            return;

        if(strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
            continue;

        char path[1024];
        sprintf(path, "%s/%s", dirpath, entry->d_name);

        if(entry->d_type == DT_DIR)
        {
            tardir(root, path, fp);
        }
        else if(entry->d_type == DT_REG)
        {
            tarfile(root, path, fp);
        }
    }

    closedir(dir);
}

// ./a.out ./test aaa
void tar(const char* dir, const char* dst)
{
    // 打包前的准备工作：将目标路径转成FILE*
    FILE* fp = fopen(dst, "w");
    if(fp == NULL)
    {
        printf("open dst file error\n");
        return;
    }

    // 将根目录的名字写入打包文件
    const char* name = rindex(dir, '/');
    if(name == NULL) name = dir;
    fprintf(fp, "%s\n", name);

    // 打包目录
    tardir(dir, dir, fp);

    fclose(fp);
}

void untar(const char* file)
{
    // 获取第一行，创建目录


}

int main(int argc, char* argv[])
{
    if(argc == 3) // 打包
    {
        tar(argv[1], argv[2]);
    }
    else if(argc == 2) // 解包
    {
        untar(argv[1]);
    }
    else
    {
        printf("argument error\n");
    }

    return 0;
}
