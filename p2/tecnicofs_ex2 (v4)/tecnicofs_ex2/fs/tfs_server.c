#include "operations.h"
#include "common/common.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#define SES_ID 1


int svfileopen;                 //servidor
int ses_id[SES_ID];             //array clientes
int fcl;                        //cliente
pthread_mutex_t mtx[SES_ID];

int ses(){
    for(int i= 0; i < SES_ID; i++){
        if (ses_id[i] == -1){ 
        return i;
        }
    }
    printf("Session not found");
    return -2;
}


void tfs_sv_mount(){

    char mbuffer[41];
    read(svfileopen, mbuffer, sizeof(char) * 40);
    mbuffer[40] = 0;
    fcl = open(mbuffer, O_WRONLY);
    
    if (fcl < 0){
         printf("Error opening %s : %s\n", mbuffer, strerror(errno));
         return; 

    }

    int id = ses();
    pthread_mutex_init(&(mtx[id]), NULL);
    pthread_mutex_lock(&(mtx[id]));
    if(id == -2){
        if(write(fcl, &id, sizeof(int) < 0)){
            printf("Error write client : %s\n", strerror(errno));
            pthread_mutex_unlock(&(mtx[id]));
            return;
        }

    }

    else{
        
        ses_id[id] = fcl;
        if(write(fcl, &id, sizeof(int)) < 0){
            printf("Error write client : %s\n", strerror(errno));
            pthread_mutex_unlock(&(mtx[id]));
            return;
        }
        pthread_mutex_unlock(&(mtx[id]));        
    }
}

void tfs_sv_unmount(){
    
    int id;
    read(svfileopen, &id,sizeof(int));
    pthread_mutex_lock(&(mtx[id]));
    ses_id[id] = -2;
    close(fcl);
    pthread_mutex_unlock(&(mtx[id]));
    pthread_mutex_destroy(&(mtx[id]));
    
}

void tfs_sv_open(){

    int id;
    char name[40];
    int flags;

    read(svfileopen, &id, sizeof(int));
    pthread_mutex_lock(&(mtx[id]));
    read(svfileopen, &name, sizeof(char)*40);
    read(svfileopen, &flags, sizeof(int));
    int ret;
    ret = tfs_open(name , flags);
    if(write(fcl, &ret, sizeof(int)) < 0){
        printf("Error write client : %s\n", strerror(errno));
        pthread_mutex_unlock(&(mtx[id]));
        return;
    }
    pthread_mutex_unlock(&(mtx[id]));
}

void tfs_sv_close(){

    int id;
    int fhandle;
    read(svfileopen, &id, sizeof(int));
    pthread_mutex_lock(&(mtx[id]));
    read(svfileopen, &fhandle, sizeof(int));
    int ret ;
    ret = tfs_close(fhandle);
    if(write(fcl, &ret, sizeof(int)) < 0){
        printf("Error write client : %s\n", strerror(errno));
        pthread_mutex_unlock(&(mtx[id]));
        return;
    }
    pthread_mutex_unlock(&(mtx[id]));
}

void tfs_sv_write(){

    int id;
    int fhandle;
    size_t len;
    
    read(svfileopen , &id, sizeof(int));
    pthread_mutex_lock(&(mtx[id]));
    read(svfileopen , &fhandle, sizeof(int));
    read(svfileopen , &len, sizeof(size_t));
    char buff_cont[len];
    read(svfileopen , buff_cont, len);


    ssize_t ret = tfs_write(fhandle, buff_cont, len);
    if(write(fcl, &ret, sizeof(ssize_t)) < 0){
        printf("Error write client : %s\n", strerror(errno));
        pthread_mutex_unlock(&(mtx[id]));
        return;
    }
    pthread_mutex_unlock(&(mtx[id]));   
}

void tfs_sv_read(){
    int id;
    int fhandle;
    size_t len;
    ssize_t ret = 0;
        
    read(svfileopen , &id, sizeof(int));
    pthread_mutex_lock(&(mtx[id]));
    read(svfileopen , &fhandle, sizeof(int));
    read(svfileopen , &len, sizeof(size_t));
    char buff_cont[len];
    ret = tfs_read(fhandle, buff_cont, len);
    if(write(fcl, &ret, sizeof(ssize_t)) < 0){
        printf("Error write client : %s\n", strerror(errno));
        pthread_mutex_unlock(&(mtx[id]));
        return;
    }
    write(fcl, buff_cont, len);
    pthread_mutex_unlock(&(mtx[id]));
}

void tfs_sv_shutdownafterclose(){
    int id;
    int ret;

    read(svfileopen, &id, sizeof(int));
    pthread_mutex_lock(&(mtx[id]));
    ret = tfs_destroy_after_all_closed();
    write(id, &ret , sizeof(int));
    pthread_mutex_unlock(&(mtx[id]));
}


int main(int argc, char **argv) {

    ses_id[0] = -1; 

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);
    unlink(pipename);
    int create = mkfifo(pipename, 0777);
    if( create == -1 ) {
        if (errno != EEXIST){                         //verifica que  o erro n??o ?? o named pipe j?? existir
            return -1;
        }
    }
    svfileopen = open(pipename, O_RDONLY);
    char opcode = 0;
    tfs_init();
    ssize_t bufread; 

    
    while(1){
        bufread = read(svfileopen, &opcode, sizeof(char));
        if(bufread != 1){
            exit(2);
        }
        switch (opcode)
        {
        case TFS_OP_CODE_MOUNT :
            tfs_sv_mount();
            break;
        
        case TFS_OP_CODE_UNMOUNT :
            tfs_sv_unmount();
            break;

        case TFS_OP_CODE_OPEN : 
            tfs_sv_open();
            break;
        
        case TFS_OP_CODE_CLOSE :
            tfs_sv_close();
            break;

        case TFS_OP_CODE_WRITE : 
            tfs_sv_write();
            break;
        
        case TFS_OP_CODE_READ :
            tfs_sv_read();
            break;

        case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED :
            tfs_sv_shutdownafterclose();
            break;

        default:
            break;
        }
    } 
}