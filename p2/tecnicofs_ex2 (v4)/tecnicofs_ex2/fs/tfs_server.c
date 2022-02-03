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
int svopen;
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
    printf("%d\n", id);
    char* mbuffer = (char*) malloc(sizeof(char)*40);
    read(svfileopen, mbuffer, sizeof(char) * 40);
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl < 0){

        printf("Error opening\n");

    }
    
    if((id = ses()) == -2){

        printf("User found, changing\n");
        write(fcl, id, sizeof(int));
        free(mbuffer);

    }

    else{
        
        printf("User not found, adding\n");
        ses_id[id] = fcl;
        write(fcl, id, sizeof(int));
        free(mbuffer);
        printf("Mounted\n");
            
    }

}
/*
void tfs_sv_unmount(){

    int id;
    read(svfileopen, &id,sizeof(int));
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        printf("Error opening");
        exit(2);

    }
    fcl = ses_id[id];
    close(fcl);
    ses_id[id] = -2;
}
*/
void tfs_sv_open(){

    int id;
    char const* name;
    int flags;
    
    void* mbuffer = (void*) malloc(sizeof(char)*40);
    read(svfileopen, &id, sizeof(int));
    read(svfileopen, &name, sizeof(char)*40);
    read(svfileopen, &flags, sizeof(int));
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        printf("Error opening");
        exit(2);

    }
  
    tfs_open(&name , flags);

}
/*
void tfs_sv_close(){

    int id;
    int fhandle;

    read(svfileopen, &id, sizeof(int));
    read(svfileopen + sizeof(int), &fhandle, sizeof(int));
    int fcl = open(mbuffer, O_WRONLY);
    if (fcl != 1){

        printf("Error opening");
        exit(2);

    }
    int fcl = ses_id[id];

    tfs_close(fhandle);


}
*/
void tfs_sv_write(){

    int id;
    int fhandle;
    size_t len;
    char buff_cont[len];

    printf("Starting tfs_write\n");
    read(svfileopen , &id, sizeof(int));
    read(svfileopen , &fhandle, sizeof(int));
    read(svfileopen , &len, sizeof(size_t));
    read(svfileopen , buff_cont, len);


    size_t res = tfs_write(fhandle, buff_cont, len);
    write(ses_id[id], res, sizeof(int));
    printf("tfs_write done\n");
    


}

int main(int argc, char **argv) {

    client_t client[S];
    int actual_session;

    ses_id[0] = -1; 
    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);
    unlink(pipename);
    printf("Input received\n");
    int create = mkfifo(pipename, 0777);
    if( create == -1 ) {
        if (errno != EEXIST){                         //verifica que  o erro não é o named pipe já existir
            return -1;
        }
    }
    printf("Pipe created\n");
    fflush(stdout);
    svfileopen = open(pipename, O_RDONLY);
    printf("Pipe opened\n");
    fflush(stdout);
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
            printf("tfs_sv_mount done\n");
            break;
        
        case TFS_OP_CODE_UNMOUNT :
            // tfs_sv_unmount();
            break;

        case TFS_OP_CODE_OPEN : 
            tfs_sv_open();
            printf("tfs_open done\n");
            break;
        
        case TFS_OP_CODE_CLOSE :
            // tfs_sv_close();
            break;

        case TFS_OP_CODE_WRITE : 
            tfs_sv_write();
            printf("tfs_write done\n");
            break;
        
        case TFS_OP_CODE_READ :
            break;

        case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED :
            break;
        
        
        
        
        
        
        
        
        }
        


        
        



    }

    
}