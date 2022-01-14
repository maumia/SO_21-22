#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 5

/**
   This test creates a max number of 9 threads, opens up to 9 files, reads up to 9 files
    and write it on the same file while running in paralell.
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

typedef struct{
	int fhandle;
	char *buffer;
	size_t len;
	ssize_t return_to_read;
}type_tfs_read;

void* read( void* args){
	type_tfs_open *settings = (type_tfs_open*) args;
    settings->return_value = tfs_open(settings->name,settings->flags);

	type_tfs_read *settings_r = (type_tfs_read*) args;
	settings_r->fhandle = settings->return_value;
    settings_r->return_to_read = tfs_read(settings_r->fhandle,settings_r->buffer,settings_r->len);

    return NULL;
}

typedef struct{
	int fhandle;
	char *buffer;
	size_t to_write;
	ssize_t return_to_write;
}type_tfs_write;

void* write( void* args){
	type_tfs_open *settings = (type_tfs_open*) args;
    settings->return_value = tfs_open(settings->name,settings->flags);

	type_tfs_write *settings_w = (type_tfs_write*) args;
	settings_w->fhandle = settings->return_value;
    settings_w->return_to_write = tfs_write(settings_w->fhandle,settings_w->buffer,settings_w->to_write);

    return NULL;
}


int main() {

    pthread_t tid[NUM_THREADS];
    type_tfs_open new_open[NUM_THREADS];
	type_tfs_read new_read[NUM_THREADS];
	type_tfs_write new_write[NUM_THREADS];
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
		pthread_create(&tid[j], NULL, read, (void *)&new_read[j]);
	}

	for( int j = 0; j < NUM_THREADS; j++ ){
		pthread_join(tid[j], NULL);
	}

	for( int j = 0; j < NUM_THREADS; j++){
		assert(new_open[j].return_value >= 0 );
		assert( tfs_close( new_open[j].return_value ) != -1 );
	}

    assert(tfs_destroy() != -1);
  
	puts("Sucessful test read_diff_write_on_the_same");

    return 0;
}