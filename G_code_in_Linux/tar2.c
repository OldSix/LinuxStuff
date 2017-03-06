
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
// 打包
// ./tar dir dstfile
// 解包
// ./tar dstfile 
//
// /home/xueguoliang/aaa
// /home/xueguoliang/aaa/bbb/ccc.c
void tarfile(const char* root, const char* filepath, FILE* fp)
{
    fprintf(fp, "%s\n", "-");
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

    fprintf(fp, "%s\n", "d");
    fprintf(fp, "%s\n", dirpath+strlen(root));

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

const char* endflag = "{==end==}";
const char* headerflag = "sz02's tar file";

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

    fprintf(fp, "%s\n", headerflag);

    // 将根目录的名字写入打包文件
    const char* name = rindex(dir, '/');
    if(name == NULL) name = dir;
    fprintf(fp, "%s\n", name);

    // 打包目录
    tardir(dir, dir, fp);


    // 打包结束的标记
    fprintf(fp, "%s\n", endflag);

    fclose(fp);
}

char* get_line(FILE*fp)
{
    static char line[4096];
    fgets(line, sizeof(line), fp);
    line[strlen(line)-1] = 0;
    printf("line is %s\n", line);
    return line;
}

// root = "test"
int untarfile(const char* root, FILE* fp)
{
    char* type = get_line(fp);
    if(strcmp(type, endflag) == 0)
        return -1;


    if(strcmp(type, "d") == 0)
    {
        char* filename = get_line(fp);
        char filepath[1024];
        sprintf(filepath, "%s/%s", root, filename);
        mkdir(filepath, 0777);
        return 0;
    }

    char* filename = get_line(fp);
    char filepath[1024];
    sprintf(filepath, "%s/%s", root, filename);

    int filelen = atoi(get_line(fp));

    // test/yyy
    // test/xxx/yyy
    FILE* dst = fopen(filepath, "w");
    if(dst == NULL)
    {
        // 说明有中间目录没有被创建出来，应该创建中间目录先

        printf("filepath is %s\n", filepath);
    }

    char buf[4096];
    while(filelen > 0)
    {
        int readlen = filelen < sizeof(buf)? filelen : sizeof(buf);

        int ret = fread(buf, 1, readlen, fp);
        if(ret <= 0) break;

        fwrite(buf, ret, 1, dst);

        filelen -= ret;
    }

    fclose(dst);
    return 0; 
}

void untar(const char* file)
{
    FILE* fp = fopen(file, "r");
    if(fp == NULL)
    {
        printf("can not open src file\n");
        return;
    }

    char* flag = get_line(fp);
    if(strcmp(flag, headerflag) != 0)
    {
        printf("format error\n");
        return;
    }

    // test
    // root --> static buf
    // strdup = {malloc strcpy}
    char* root = strdup(get_line(fp)); //已经去掉了\n
    mkdir(root, 0777);

    while(1)
    {
        if(untarfile(root, fp) == -1)
            break;
    }

    free(root);
    fclose(fp);
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
