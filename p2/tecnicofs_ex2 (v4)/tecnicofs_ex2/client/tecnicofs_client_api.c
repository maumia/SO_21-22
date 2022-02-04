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


int id;             //client id
int client_pipe;    //client pipe
int server_pipe;    //server pipe
char cl_path[40];   //client path



int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    
    unlink(client_pipe_path);

    if( mkfifo(client_pipe_path, 0777) == -1 ) {
        printf("Error creating pipe%s\n", strerror(errno));
    }
    memcpy(cl_path, client_pipe_path, 40);
    printf("Client created\n");
        
    char messg[41];
    messg[0] = TFS_OP_CODE_MOUNT;
    memcpy(messg + 1, client_pipe_path, MAX_INPUT);
    server_pipe = open(server_pipe_path, O_WRONLY);
    printf("Server opened\n");
    if(server_pipe == -1){
        printf("Error opening server pipe");
        return -1;
    }
    write(server_pipe, messg, 41);
    printf("Wrote to sv pipe\n");
    fflush(stdout);
    client_pipe = open(client_pipe_path, O_RDONLY);
    printf("Client opened\n");
    fflush(stdout);
    if(client_pipe <= 0){
        printf("Error opening client pipe");
        return -1;
    }
   
    
    if(read(client_pipe, &id , sizeof(int)) < 0){
        printf("Error reading pipe%s\n", strerror(errno));
    };

    printf("Mounted\n");
    return 0;
   
}

int tfs_unmount() {
    
    int res;
    char messg[sizeof(char) + sizeof(int)];
    messg[0] = TFS_OP_CODE_UNMOUNT;
    memcpy(messg + 1, &id, sizeof(char));
    write(server_pipe, messg, 1 + sizeof(int));
    read(client_pipe, &res , sizeof(int));
    unlink(client_pipe);

    printf("Unmounted\n");
    return 0;
}

int tfs_open(char const *name, int flags) {
    char messg[sizeof(char) + sizeof(char) * MAX_INPUT + sizeof(int) * 2]; 
    messg[0] = TFS_OP_CODE_OPEN;
    memcpy(messg + sizeof(char), &id, sizeof(int));
    memcpy(messg + sizeof(int) + sizeof(char), name, MAX_INPUT);
    memcpy(messg + sizeof(char) + sizeof(int) + sizeof(char) * MAX_INPUT, &flags, sizeof(int));
    
    if(write(server_pipe, messg, sizeof(char) + sizeof(char) * MAX_INPUT + sizeof(int) * 3) < 0)
        printf("Error writting to sv: %s", strerror(errno));
    

    int ret = 0;
    read(client_pipe, &ret, sizeof(int));
    printf("Opened\n");
    return ret;
}

int tfs_close(int fhandle) {

    char messg[sizeof(int) * 3]; 
    messg[0] = TFS_OP_CODE_CLOSE;
    memcpy(messg + sizeof(char), &id, sizeof(int));
    memcpy(messg + sizeof(int) * 2, &fhandle, sizeof(int));
    
    write(server_pipe, messg, sizeof(int) * 3);
    read(client_pipe, &id , sizeof(int));
    printf("Closed\n");
    return 0;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    
    char messg[sizeof(int) * 2 + sizeof(size_t) + sizeof(char) + len];
    messg[0] = TFS_OP_CODE_WRITE;
    memcpy(messg + sizeof(char), &id, sizeof(int));
    memcpy(messg + sizeof(int) + sizeof(char), &fhandle, sizeof(int));
    memcpy(messg + 9, &len, sizeof(sizeof(size_t)));
    memcpy(messg + 9 + sizeof(size_t), buffer, len);

    ssize_t ret = 0;
    write(server_pipe, messg, 9 + sizeof(size_t) + len);
    read(client_pipe, &ret , sizeof(ssize_t));
    printf("Wrote\n");
    return ret; 
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    
    char messg[sizeof(char) + sizeof(int) * 2 + sizeof(size_t)];
    messg[0] = TFS_OP_CODE_READ;
    memcpy(messg + sizeof(char), &id, sizeof(int));
    memcpy(messg + sizeof(int) + sizeof(char), &fhandle, sizeof(int));
    memcpy(messg + 9, &len, sizeof(sizeof(size_t)));
    
    write(server_pipe,messg,sizeof(char) + sizeof(int) * 2 + sizeof(size_t) );
    ssize_t ret = 0;   
    if(read(client_pipe, &ret , sizeof(ssize_t)) < 0)
        printf("Erro no read: %s", strerror(errno)); //Está a funcionar
    printf("Read1\n");
    // read(client_pipe, buffer , len);
    printf("Read\n");
    return ret;
}

int tfs_shutdown_after_all_closed() {
    char messg[sizeof(char) + sizeof(int)];
    messg[0] = TFS_OP_CODE_READ;
    memcpy(messg + 1, &id, sizeof(int));





    return -1;
}
