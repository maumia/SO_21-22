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
int id;             //client id
int client_pipe;    //client pipe
int server_pipe;    //server pipe
char cl_path[40];



int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    
    unlink(client_pipe_path);

    if( mkfifo(client_pipe_path, 0777) == -1 ) {
        printf("Error creating pipe");
        exit(-1);
    }
    memcpy(cl_path, client_pipe_path, 40);
    printf("Client created\n");
    server_pipe = open(server_pipe_path, O_WRONLY);
    printf("Server opened\n");
    if(server_pipe == -1){
        printf("Error opening server pipe");
        return -1;
    }

    

    
    char* messg = (char*) malloc(41);
    sprintf(messg, "%c", TFS_OP_CODE_MOUNT + 48);
    memcpy(messg + 1, client_pipe_path, MAX_INPUT);
    
    write(server_pipe, messg, 41);
    printf("Wrote to sv pipe");
    client_pipe = open(client_pipe_path, O_RDONLY);
    if(client_pipe == -1){
        printf("Error opening client pipe");
        return -1;
    }
    printf("Client opened\n");
    fflush(stdout);
    if(client_pipe == -1){
        printf("Error creating pipe");
        return -1;
    }
    read(client_pipe, &id , sizeof(int));
    
    return 0;
}

int tfs_unmount() {
    
    int res;
    char* messg = (char*) malloc(1 + sizeof(int));
    sprintf(messg, "%c", TFS_OP_CODE_UNMOUNT);
    memcpy(messg + 1, &id, sizeof(int));
    write(server_pipe, messg, 1 + sizeof(int));
    read(client_pipe, &res , sizeof(int));
    unlink(cl_path);

    return res;
}

int tfs_open(char const *name, int flags) {

    char* messg = (char*) malloc(MAX_INPUT + 1 + 1 + 1); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(messg, "%c", TFS_OP_CODE_OPEN);
    memcpy(messg + 1, &id, sizeof(int));
    memcpy(messg + 2, &name, MAX_INPUT);
    memcpy(messg + 2 + MAX_INPUT, &flags, sizeof(int));
    
    write(server_pipe, messg, MAX_INPUT + 1 + 1 + 1);
    read(client_pipe, &id , sizeof(int));

    return -1;
}

int tfs_close(int fhandle) {
    
    char* messg = (char*) malloc(MAX_INPUT + 1 + 1); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(messg, "%c", TFS_OP_CODE_CLOSE);
    memcpy(messg + 1, &id, sizeof(int));
    memcpy(messg + 2, &fhandle, MAX_INPUT);
    
    write(server_pipe, messg, MAX_INPUT + 1 + 1);
    read(client_pipe, &id , sizeof(int));

    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    
    char* messg = (char*) malloc(1 + 1 + 1 + 1 + len); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(messg, "%c", TFS_OP_CODE_WRITE);
    memcpy(messg + 1, &id, sizeof(int));
    memcpy(messg + 2, &fhandle, MAX_INPUT);
    memcpy(messg + 3, len, sizeof(int));
    memcpy(messg + len, &buffer, len);

    write(server_pipe, messg, MAX_INPUT + 1 + 1 + 1 + len);
    read(client_pipe, &id , sizeof(int));
    return -1;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    
    char* messg = (char*) malloc(1 + 1 + 1 + 1); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(messg, "%c", TFS_OP_CODE_READ);
    memcpy(messg + 1, &id, sizeof(int));
    memcpy(messg + 2, &fhandle, MAX_INPUT);
    memcpy(messg + 3, len, sizeof(int));
    
    write(server_pipe, messg, MAX_INPUT + 1 + 1 + 1);
    read(client_pipe, &id , sizeof(int));

    return -1;
}

int tfs_shutdown_after_all_closed() {
    char* messg = (char*) malloc(1 + 1); //40 DO CHAR + 1 DO OP_CODE + 1 DO ID + 1 DA FLAG
    sprintf(messg, "%c", TFS_OP_CODE_READ);
    memcpy(messg + 1, &id, sizeof(int));





    return -1;
}
