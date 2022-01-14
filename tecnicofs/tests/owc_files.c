#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 5

/**
   This test creates a max number of 9 threads and verifies if tecnico_fs can open,create and write multiple files in paralell.
 */

typedef struct{
    char *name;
    int flags;
    int return_value;
}type_tfs_open;

typedef struct{
    char *name;
    int flags;
    void const *buffer;
    size_t to_write;
    long int return_value;
}type_tfs_write;

void* open( void* args){
    type_tfs_open *settings = (type_tfs_open*) args;
    settings->return_value = tfs_open(settings->name,settings->flags);
    return NULL;
}

void* write(void* args){
    type_tfs_write *settings = (type_tfs_write*) args;
    settings ->return_value = tfs_write(settings->flags, settings->buffer,settings->to_write);
    return NULL;
}

int main() {

    pthread_t tid[NUM_THREADS];

    /*
    type_tfs_open new_open[NUM_THREADS];
	new_open[0].name = "/f0";
	new_open[1].name = "/f1";
	new_open[2].name = "/f2";
	new_open[3].name = "/f3";
	new_open[4].name = "/f4";
    */

    type_tfs_write new_write[NUM_THREADS];
    new_write[0].name = "/f0";
	new_write[1].name = "/f1";
	new_write[2].name = "/f2";
	new_write[3].name = "/f3";
	new_write[4].name = "/f4";
    new_write[0].to_write = 10;
    new_write[1].to_write = 10;
    new_write[2].to_write = 10;
    new_write[3].to_write = 10;
    new_write[4].to_write = 10;
    new_write[0].buffer = "ola";
    new_write[1].buffer = "ola";
    new_write[2].buffer = "ola";
    new_write[3].buffer = "ola";
    new_write[4].buffer = "ola";


    for(int i = 0; i < NUM_THREADS; i++){
                      
        new_write[i].flags = TFS_O_CREAT;
        new_write[i].return_value = -100;
    }

	assert(tfs_init() != -1);
	for( int j = 0; j < NUM_THREADS; j++){
		pthread_create(&tid[j], NULL, open, (void *)&new_write[j]);
	}

    for( int j = 0; j < NUM_THREADS; j++){
		pthread_create(&tid[j], NULL, open, (void *)&new_write[j]);
	}

	for( int j = 0; j < NUM_THREADS; j++ ){
		pthread_join(tid[j], NULL);
	}

	for( int j = 0; j < NUM_THREADS; j++){
		assert(new_write[j].return_value >= 0 );
		assert(tfs_close( new_write[j].return_value ) != -1 );
	}

    assert(tfs_destroy() != -1);
  
	puts("Sucessful test owc_files");

    return 0;
}