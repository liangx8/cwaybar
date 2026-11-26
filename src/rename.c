#define _POSIX_C_SOURCE 200809L


#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <wchar.h>
#include "log.h"
#include "strlist.h"
#include "cfg_data.h"
static void join_path(char *out, const char *dirname,const char *name)
{
    size_t dirname_length = strlen(dirname);
    size_t name_length=strlen(name)+1;
    int needs_separator = dirname[dirname_length] != '/';
    if(512<(dirname_length+name_length+needs_separator)){
        *out='\0';
        return;
    }
    memcpy(out, dirname, dirname_length);
    size_t offset = dirname_length;
    if (needs_separator) {
        out[offset++] = '/';
    }
    memcpy(out+offset,name,name_length);
    return ;
}
const char *const sufs[]={".jpg",".png"};

int walk_directory(const char *dirname, strlist *list)
{
    if (!dirname || !list) {
        return -1;
    }

    DIR *directory = opendir(dirname);
    if (!directory) {
        return -1;
    }

    int result = 0;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (!entry) {
            if (errno != 0) {
                result = -1;
            }
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        size_t len=strlen(entry->d_name);
        if(len<4) continue;
        int ignore=1;
        for(int ix=0;ix<2;ix++){
            size_t slen=strlen(sufs[ix]);
            if(strcmp(entry->d_name+len-slen,sufs[ix])==0){
                ignore=0;
                break;
            }
        }
        if(ignore){
            continue;
        }
        if (strlist_add(list, entry->d_name) != 0) {
            result = -1;
            break;
        }
    }

    closedir(directory);
    return result;
}
const char rm1[]="【哲风壁纸】";
const char repl[]="girls";
#define OUT_MAX 512
void concat_out(char *out,const char *lead,const char *file,size_t seq)
{
    const size_t rm1_length=sizeof(rm1)-1;
    const size_t repl_length=sizeof(repl)-1;
    if(strncmp(rm1,file,rm1_length)==0){
        snprintf(out,OUT_MAX,"%s%04ld_%s",lead,seq,file+rm1_length);
        return;
    }
    if(strncmp(repl,file,repl_length)==0){
        size_t fs=strlen(file);
        snprintf(out,OUT_MAX,"%s%04ld_girls%s",lead,seq,file+fs-4);
        return;
    }
    snprintf(out,OUT_MAX,"%s%04ld_%s",lead,seq,file);
    
} 
int rename_action(int argc, char *argv[],void *obj)
{
    struct CFG_DATA *cfg=(struct CFG_DATA *)obj;
    char out[OUT_MAX];
    char outs[OUT_MAX];

    if(cfg->rename_dir){
        wprintf(L"rename dir:%s\n",cfg->rename_dir);
    }
    strlist *list = strlist_create();
    if (!list) {
        return -1;
    }
    const size_t dir_length=strlen(cfg->rename_dir);
    memcpy(out,cfg->rename_dir,dir_length);
    const int needs_separator=out[dir_length-1]!='/';
    const int offset=dir_length + needs_separator;
    if(needs_separator){
        out[dir_length]='/';
    }
    int result = walk_directory(cfg->rename_dir, list);
    if(result){
        log_info(L"walk through dir %s error",cfg->rename_dir);
        strlist_destroy(list);
        return result;
    }
    size_t size=strlist_size(list);
    for(size_t ix=0;ix<size;ix++){
        join_path(outs,cfg->rename_dir,strlist_get(list,ix));
        concat_out(out+offset,cfg->rename_lead,strlist_get(list,ix),ix);
        wprintf(L"%s\n  \033[34m%s\033[0m\n",outs,out);
    }
    wprintf(L"总共有%d\n",size);
    strlist_destroy(list);
    return result;
}