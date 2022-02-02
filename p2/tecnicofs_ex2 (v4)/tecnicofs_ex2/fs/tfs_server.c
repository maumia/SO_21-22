#include "operations.h"
#include <fcntl.h>
#include "common/common.h"
#include <errno.h>’
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
        printf("Session not found");
    return -2;

}


void tfs_sv_mount(){
    int id;
    read(svfileopen, &id,sizeof(int));
    void* mbuffer = (void*) malloc(sizeof(char)*40);
    read(svfileopen, mbuffer, 40);
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        printf("Error opening");
        exit(2);

    }
    
    if((id = ses()) == -2){

        printf("Session not found");
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
    //ver este mbuffer que nao esta a fazer nada
    int id;
    void* mbuffer = (void*) malloc(sizeof(char)*40);
    read(svfileopen, &id,sizeof(int));
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        printf("Error opening");
        exit(2);

    }
    // int fcl = ses_id[id];
    close(fcl);
}

void tfs_sv_open(){

    int id;
    char const* name;
    int flags;
    
    void* mbuffer = (void*) malloc(sizeof(char)*40);
    read(svfileopen, &id, sizeof(int));
    read(svfileopen + sizeof(int), &name, sizeof(char)*40);
    read(svfileopen + sizeof(char) * 40 + sizeof(int), &flags, sizeof(int));
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        printf("Error opening");
        exit(2);

    }
    // int fcl = ses_id[id];
    
    tfs_open(&name , flags);

}

void tfs_sv_close(){

    int id;
    int fhandle;

    void* mbuffer = (void*) malloc(sizeof(char)*40);
    read(svfileopen, &id, sizeof(int));
    read(svfileopen + sizeof(int), &fhandle, sizeof(int));
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        printf("Error opening");
        exit(2);

    }
    // int fcl = ses_id[id];

    tfs_close(fhandle);


}

void tfs_sv_write(){

    int id;
    int fhandle;
    size_t len;
    // char[len] buff_cont;

    read(svfileopen, &id, sizeof(int));
    read(svfileopen + sizeof(int), &fhandle, sizeof(int));
    read(svfileopen + (sizeof(int) * 2), &len, sizeof(int));
    // read(svfileopen + (sizeof(int) * 3), &buff_cont, sizeof(int));





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
    printf("Input received\n");
    if(mkfifo(pipename,0777)<0)
        return -1;
    /*
    if( create < 0 ) {
        if (errno != EEXIST){                         //verifica que  o erro não é o named pipe já existir
            return -1;
        }
    }
    */
    printf("Pipe created");
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
            break;

        case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED :
            break;
        
        
        
        
        
        
        
        
        }
        


        
        



    }

    
}