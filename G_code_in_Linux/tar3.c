
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

// 硬链接问题
// 如果一个文件被打包过了
// 后面的硬链接就不需要再打包了，如何判断？？？？
//


// 在全局变量中，保存已经被打包的文件信息
// inode <--> 文件名
typedef struct file_info
{
    ino_t inode;
    char filename[256] ; // 是否全路径？？？
} file_info;

// 最多支持1024个文件的打包，多了就挂了
file_info* infos;
int info_index = 0;

void tarfile(const char* root, const char* filepath, FILE* fp)
{
    struct stat stat_buf;
    stat(filepath, &stat_buf);

    int i;
    for(i=0; i<info_index; ++i)
    {
        // 表示该文件已经保存过了
        if(infos[i].inode == stat_buf.st_ino)
        {
            // 这个是硬链接
            fprintf(fp, "%s\n", "h");
            fprintf(fp, "%s\n", filepath+strlen(root));
            fprintf(fp, "%s\n", infos[i].filename);  // 链接对象
            return;
        }
    }

    // 1. 获取filepath的inode
    // 2. 判断这个inode是不是已经被打包了
    // 3. 如果不是，那么按照之前做法，将文件打包到打包文件中，并且把它的信息保存到全局变量
    // 4. 如果是已经打包了，写入硬链接信息
    fprintf(fp, "%s\n", "-");
    // 将文件名写入，将文件的尺寸写入
    fprintf(fp, "%s\n", filepath+strlen(root));

    // 将文件尺寸写入
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

    infos[info_index].inode = stat_buf.st_ino;
    strcpy(infos[info_index].filename, filepath+strlen(root));
    info_index++;
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
            // 打印时间戳
            struct tms buf1;
            times(&buf1);
            tarfile(root, path, fp);
            // 打印时间戳
            struct tms buf2;
            times(&buf2);
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
    else if(strcmp(type, "h") == 0)
    {
        // 获取即将要创建的文件的名字
        char* line = get_line(fp);
        char newfilepath[1024];
        sprintf(newfilepath, "%s/%s", root, line);

        // 硬链接中，已经打包过的文件的名字
        line = get_line(fp);
        char oldfilepath[1024];
        sprintf(oldfilepath, "%s/%s", root, line);

        // 进行链接
        link(oldfilepath, newfilepath);

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
    infos = malloc(sizeof(*infos) * 1024);
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
