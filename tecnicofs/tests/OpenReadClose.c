#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 5

/**
   This test creates a max number of 9 threads and verifies if tecnico_fs can open,read and close multiple files in paralell.
 */

typedef struct{
    char *name;
    int flags;
    void const *buffer;
    size_t to_read;
    int return_value;
}type_tfs_read;

void* read(void* args){
    type_tfs_read *settings = (type_tfs_read*) args;
    settings ->return_value =(int)tfs_read(settings->flags, settings->buffer,settings->to_read);
    return NULL;
}

int main() {

    pthread_t tid[NUM_THREADS];
    type_tfs_read new_read[NUM_THREADS];

    new_read[0].name  =  "/f0";
	new_read[1].name  =  "/f1";
	new_read[2].name  =  "/f2";
	new_read[3].name  =  "/f3";
	new_read[4].name  =  "/f4";
    new_read[0].to_read  = 10;
    new_read[1].to_read  = 10;
    new_read[2].to_read  = 10;
    new_read[3].to_read  = 10;
    new_read[4].to_read  = 10;
    /*
    new_read[0].buffer = "ola";
    new_read[1].buffer = "ola";
    new_read[2].buffer = "ola";
    new_read[3].buffer = "ola";
    new_read[4].buffer = "ola";
    */

    for(int i = 0; i < NUM_THREADS; i++){
                      
        new_read[i].flags = TFS_O_CREAT;
        new_read[i].return_value = -100;
    }

	assert(tfs_init() != -1);
	for( int j = 0; j < NUM_THREADS; j++){
		pthread_create(&tid[j], NULL, read, (void *)&new_read[j]);
	}



	for( int j = 0; j < NUM_THREADS; j++ ){
		pthread_join(tid[j], NULL);
	}

	for( int j = 0; j < NUM_THREADS; j++){
		assert(new_read[j].return_value >= 0 );
		assert(tfs_close(new_read[j].return_value) != -1 );
	}

    assert(tfs_destroy() != -1);
  
	puts("Sucessful test");

    return 0;
}