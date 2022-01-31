#include "operations.h"

#define S 1

typedef struct{
    int session_id;
    char client_pipe[40];
}client_t;

int main(int argc, char **argv) {

    client_t client[S];
    int actual_session;

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);

    /* TO DO */
    /*int client_fd = open(client_pipe_path, O_WRONLY);
    if(client_fd == -1){
        return -1;
    }*/

    int server_fd = open(pipename, O_RDONLY);
    if( server_fd == -1){
        return -1;
    }
    for(int i = 0; i < S; i++){
        if(read(server_fd, &client[i].client_pipe, sizeof(char)*40) == -1){
            return -1;
        }
        client[i].session_id = actual_session;
        actual_session += 1;
    }

    int client_fd = open(client[actual_session-1].client_pipe, O_WRONLY);
    if( client_fd == -1){
        return -1;
    }

    while(true){
        
    }
    return 0;
}