#include "tecnicofs_client_api.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT 40

static char bufferPipe[MAX_INPUT];
int id; //client id
int client_fd;
int server_fd;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    
    if( mkfifo(client_pipe_path, 0777) == -1 ) {
        if (errno != EEXIST){                         //verifica que  o erro não é o named pipe já existir
            return -1;
        }
    }
    client_fd = open(client_pipe_path, O_RDONLY);
    if(client_fd == -1){
        return -1;
    }
    server_fd = open(server_pipe_path, O_WRONLY);
    if( server_fd == -1){
        return -1;
    }
    char* mess = (char*) malloc(41);
    sprintf(mess, "%d", TFS_OP_CODE_MOUNT);
    memcpy(mess + 1, client_pipe_path, MAX_INPUT);
    write(server_fd, mess, 41);
    read(client_fd, &id , sizeof(int));
    return id;
}

int tfs_unmount() {
    
    int err;
    char* mess = (char*) malloc(1 + sizeof(int));
    sprintf(mess, "%d", TFS_OP_CODE_UNMOUNT);
    memcpy(mess + 1, &id, sizeof(int));
    write(server_fd, mess, 1 + sizeof(int));
    read(client_fd, &err , sizeof(int));

    return err;
}

int tfs_open(char const *name, int flags) {

    char* mess = (char*) malloc(MAX_INPUT + 1 + 1 + 1); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(mess, "%d", TFS_OP_CODE_OPEN);
    memcpy(mess + 1, &id, sizeof(int));
    memcpy(mess + 2, &name, MAX_INPUT);
    memcpy(mess + 2 + MAX_INPUT, &flags, sizeof(int));
    
    write(server_fd, mess, MAX_INPUT + 1 + 1 + 1);
    read(client_fd, &id , sizeof(int));

    return -1;
}

int tfs_close(int fhandle) {
    char* mess = (char*) malloc(MAX_INPUT + 1 + 1); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(mess, "%d", TFS_OP_CODE_CLOSE);
    memcpy(mess + 1, &id, sizeof(int));
    memcpy(mess + 2, &fhandle, MAX_INPUT);
    
    write(server_fd, mess, MAX_INPUT + 1 + 1);
    read(client_fd, &id , sizeof(int));

    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    
    char* mess = (char*) malloc(1 + 1 + 1 + 1 + len); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(mess, "%d", TFS_OP_CODE_WRITE);
    memcpy(mess + 1, &id, sizeof(int));
    memcpy(mess + 2, &fhandle, MAX_INPUT);
    memcpy(mess + 3, len, sizeof(int));
    memcpy(mess + len, &buffer, len);

    write(server_fd, mess, MAX_INPUT + 1 + 1 + 1 + len);
    read(client_fd, &id , sizeof(int));
    return -1;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    
    char* mess = (char*) malloc(1 + 1 + 1 + 1); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(mess, "%d", TFS_OP_CODE_READ);
    memcpy(mess + 1, &id, sizeof(int));
    memcpy(mess + 2, &fhandle, MAX_INPUT);
    memcpy(mess + 3, len, sizeof(int));
    
    write(server_fd, mess, MAX_INPUT + 1 + 1 + 1);
    read(client_fd, &id , sizeof(int));

    return -1;
}

int tfs_shutdown_after_all_closed() {
    char* mess = (char*) malloc(1 + 1 ; //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(mess, "%d", TFS_OP_CODE_READ);
    memcpy(mess + 1, &id, sizeof(int));





    return -1;
}
