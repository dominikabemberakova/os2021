#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

 
void find(char* path,char *name) {
    char buf[512], *p=0;
    int fd;
    struct dirent de;
    struct stat st;
 
    if((fd = open(path, 0)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }
 //checking the file type 
    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch(st.type){
    case T_FILE:
        if(strcmp((path),name)==0)
            printf("%s\n",path);
        break;
 
    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            //skipping "." and ".." to avoid looping forever 
            if(strcmp(de.name,".")==0||strcmp(de.name,"..")==0){
                continue;
            }
            if(strcmp(de.name,name)==0){
                printf("%s/%s\n",path,name);
            }
            //recursion to go deeper inside
            find(buf,name);
        }
        break;
    }
    close(fd);
 
}
int main(int argc,char* argv[]){
    if(argc<3){
        exit(-1);
    }
    find(argv[1],argv[2]);
    exit(0);
}