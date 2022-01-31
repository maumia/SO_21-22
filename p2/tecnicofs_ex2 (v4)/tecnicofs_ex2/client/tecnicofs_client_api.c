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

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */
    if( mkfifo(client_pipe_path, 0777) == -1 ) {
        if (errno != EEXIST){                         //verifica que  o erro não é o named pipe já existir
            return -1;
        }
    }
    int client_fd = open(client_pipe_path, O_RDONLY);
    if(client_fd == -1){
        return -1;
    }
    int server_fd = open(server_pipe_path, O_WRONLY);
    if( server_fd == -1){
        return -1;
    }
    char send_client_pipe[MAX_INPUT] = client_pipe_path;                                // possivel erro (passar para char)
    if(write(server_fd, &send_client_pipe, sizeof(send_client_pipe)) == -1){
        return -1;
    }


    return 0;
}

int tfs_unmount() {
    /* TODO: Implement this */
    return -1;
}

int tfs_open(char const *name, int flags) {
    /* TODO: Implement this */
    return -1;
}

int tfs_close(int fhandle) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

int tfs_shutdown_after_all_closed() {
    /* TODO: Implement this */
    return -1;
}
