#include "operations.h"
#include <fcntl.h>
#include "common/common.h"
#define SES_ID 1
#define S 1

typedef struct{
    int session_id;
    char client_pipe[40];
}client_t;


int svfileopen;
int ses_id[SES_ID]; 

int ses(){
    for(int i= 0; i < SES_ID; i++){
        if (ses_id[i] == -1){ return i;};
    }
    printf("Erro ao encontrar ses_id");
    return -2;

}


void tfs_sv_mount(){
    int id;
    void* mbuffer = (void*) malloc(sizeof(char)*40);
    read(svfileopen, mbuffer, 40);
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        exit(2);

    }
    
    if((id = ses()) == -2){

        write(fcl, id, sizeof(int));
        free(mbuffer);

    }

    else{
        
        ses_id[id] = fcl;
        write(fcl, id, sizeof(int));
        free(mbuffer);
        
    }

}

void tfs_sv_unmount(){

    int id;
    read(svfileopen, &id,sizeof(int));
    int fcl = ses_id[id];
    close(fcl);
}

void tfs_sv_open(char const *name, int flags){

    int id;
    read(svfileopen, &id, sizeof(int));
    int fcl = ses_id[id];
    tfs_open(name,flags);



}


int main(int argc, char **argv) {

    client_t client[S];
    int actual_session;

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);
    unlink(pipename);
    if(mkfifo(pipename, 0777) == -1){
        exit(-1);
    }

    svfileopen = open(pipename, O_RDONLY);
    char buffer[2] = "\0";
    tfs_init();
    int bufread; 

    
    while(1){
        bufread = read(svfileopen, buffer, 1);
        if(bufread != 1){
            exit(2);
        }
        switch (atoi(buffer))
        {
        case TFS_OP_CODE_MOUNT :
            tfs_sv_mount();
            break;
        
        case TFS_OP_CODE_UNMOUNT :

            break;

        case TFS_OP_CODE_OPEN : 
            break;
        
        case TFS_OP_CODE_CLOSE :
            break;

        case TFS_OP_CODE_WRITE : 
            break;
        
        case TFS_OP_CODE_READ :
            break;

        case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED :
            break;
        
        
        
        
        
        
        
        
        }
        


        
        



    }

    
}