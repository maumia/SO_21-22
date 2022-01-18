#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 5

/**
   This test creates a max number of 9 threads and verifies if tecnico_fs can open and create multiple files in paralell.
 */

typedef struct{
    char *name;
    int flags;
    int return_value;
}type_tfs_open;

void* open( void* args){
    type_tfs_open *settings = (type_tfs_open*) args;
    settings->return_value = tfs_open(settings->name,settings->flags);
    return NULL;
}

int main() {

    pthread_t tid[NUM_THREADS];
    type_tfs_open new_open[NUM_THREADS];
	new_open[0].name = "/f0";
	new_open[1].name = "/f1";
	new_open[2].name = "/f2";
	new_open[3].name = "/f3";
	new_open[4].name = "/f4";
	/*new_open[5].name = "/f5"
	new_open[6].name = "/f6"
	new_open[7].name = "/f7"
	new_open[8].name = "/f8"*/


    for(int i = 0; i < NUM_THREADS; i++){
        new_open[i].flags = TFS_O_CREAT;
		new_open[i].return_value = -100;               /*Para identificar possivel erro*/
    }

	assert(tfs_init() != -1);
	for( int j = 0; j < NUM_THREADS; j++){
		pthread_create(&tid[j], NULL, open, (void *)&new_open[j]);
	}

	for( int j = 0; j < NUM_THREADS; j++ ){
		pthread_join(tid[j], NULL);
	}

	for( int j = 0; j < NUM_THREADS; j++){
		assert(new_open[j].return_value >= 0 );
		assert( tfs_close( new_open[j].return_value ) != -1 );
	}

    assert(tfs_destroy() != -1);
  
	puts("Sucessful test open _files");

    return 0;
}