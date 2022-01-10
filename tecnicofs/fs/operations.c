#include "operations.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a,b) ((a)<(b)? (a) : (b))
#define MAX_FILE_SIZE ((10+BLOCK_SIZE/sizeof(int))*BLOCK_SIZE)

int tfs_init() {
    state_init();

    /* create root inode */
    int root = inode_create(T_DIRECTORY);
    if (root != ROOT_DIR_INUM) {
        return -1;
    }

    return 0;
}

int tfs_destroy() {
    state_destroy();
    return 0;
}

static bool valid_pathname(char const *name) {
    return name != NULL && strlen(name) > 1 && name[0] == '/';
}


int tfs_lookup(char const *name) {
    if (!valid_pathname(name)) {
        return -1;
    }

    // skip the initial '/' character
    name++;

    return find_in_dir(ROOT_DIR_INUM, name);
}

int tfs_open(char const *name, int flags) {
    int inum;
    size_t offset;

    /* Checks if the path name is valid */
    if (!valid_pathname(name)) {
        return -1;
    }

    inum = tfs_lookup(name);
    if (inum >= 0) {
        /* The file already exists */
        inode_t *inode = inode_get(inum);
        if (inode == NULL) {
            return -1;
        }

        /* Trucate (if requested) */
        if (flags & TFS_O_TRUNC) {
            if (inode->i_size > 0) {
                if ( inode->i_data_blocks[10] != -1){
                    int* indirect_block = data_block_get(inode->i_data_blocks[10]);
                    for(int i = 0; i< BLOCK_SIZE/sizeof(int); i++){
                        if (data_block_free(indirect_block[i]) == -1) {
                            return -1;
                        }
                    }
                }
                for(int i = 0; i<11; i++){
                    if (data_block_free(inode->i_data_blocks[i]) == -1) {
                        return -1;
                    }
                }
                inode->i_size = 0;
            }
        }
        /* Determine initial offset */
        if (flags & TFS_O_APPEND) {
            offset = inode->i_size;
        } else {
            offset = 0;
        }
    } else if (flags & TFS_O_CREAT) {
        /* The file doesn't exist; the flags specify that it should be created*/
        /* Create inode */
        inum = inode_create(T_FILE);
        if (inum == -1) {
            return -1;
        }
        /* Add entry in the root directory */
        if (add_dir_entry(ROOT_DIR_INUM, inum, name + 1) == -1) {
            inode_delete(inum);
            return -1;
        }
        offset = 0;
    } else {
        return -1;
    }

    /* Finally, add entry to the open file table and
     * return the corresponding handle */
    return add_to_open_file_table(inum, offset);

    /* Note: for simplification, if file was created with TFS_O_CREAT and there
     * is an error adding an entry to the open file table, the file is not
     * opened but it remains created */
}


int tfs_close(int fhandle) { return remove_from_open_file_table(fhandle); }

ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    /* From the open file table entry, we get the inode */
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }

    /* Determine how many bytes to write */
    if (to_write + file->of_offset > MAX_FILE_SIZE) {
        to_write = MAX_FILE_SIZE - file->of_offset;
    }

    if (to_write > 0) {
        /* If necessary, allocate blocks */
        size_t blocks = (inode->i_size + BLOCK_SIZE-1)/BLOCK_SIZE;                                    /*blocos no momento*/
        size_t new_blocks = (file->of_offset + to_write + BLOCK_SIZE-1)/BLOCK_SIZE;                   /*blocos necessarios*/
        int* indirect_block = (int*)data_block_get(inode->i_data_blocks[10]);
        for( size_t i = blocks; i < new_blocks; i++){
            if(i < 10){
                inode->i_data_blocks[i] = data_block_alloc();
                continue;
            }
            if (blocks <= 10){
                inode->i_data_blocks[10] = data_block_alloc();
                indirect_block = (int*)data_block_get(inode->i_data_blocks[10]);
                for (size_t j = new_blocks - 10; j < BLOCK_SIZE/sizeof(int); j++){
                    indirect_block[j] = -1;
                }
            }
            indirect_block[i-10] = data_block_alloc();
        }
        
        size_t block_offset = file->of_offset % BLOCK_SIZE;
        size_t i = file->of_offset / BLOCK_SIZE;                                                   /*primeiro bloco escrito*/
        size_t offset = 0;
        size_t remain = to_write;
        while(remain>0){
            size_t bytes = MIN(remain, BLOCK_SIZE - block_offset);                                 /*resta remain, cabe no bloco BLOCK_SIZE-block_offset*/
            void *block ;
            if (i<10){
                block = data_block_get(inode->i_data_blocks[i]);
            }
            else {
                block = data_block_get(indirect_block[i-10]);
            }
            if (block == NULL) {
                return -1;
            }
            /* Perform the actual write */
            memcpy(block + block_offset, buffer + offset, bytes);
            block_offset = 0;
            i++;
            offset += bytes;
            remain -= bytes;
        }

        /* The offset associated with the file handle is
         * incremented accordingly */
        file->of_offset += to_write;
        if (file->of_offset > inode->i_size) {
            inode->i_size = file->of_offset;
        }
    }

    return (ssize_t)to_write;
}


ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    /* From the open file table entry, we get the inode */
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }

    /* Determine how many bytes to read */
    size_t to_read = inode->i_size - file->of_offset;
    if (to_read > len) {
        to_read = len;
    }

    if (to_read > 0) {
        size_t block_offset = file->of_offset % BLOCK_SIZE;
        size_t i = file->of_offset / BLOCK_SIZE;                                                   /*primeiro bloco lido*/
        size_t offset = 0;
        size_t remain = to_read;
        int* indirect_block = data_block_get(inode->i_data_blocks[10]);
        while(remain>0){
            size_t bytes = MIN(remain, BLOCK_SIZE - block_offset);                                 /*resta remain, cabe no bloco BLOCK_SIZE-block_offset*/
            void *block ; 
            if(i<10){
                block = data_block_get(inode->i_data_blocks[i]);
            }
            else {
                block = data_block_get(indirect_block[i-10]);
            }
            if (block == NULL) {
                return -1;
            }
            /* Perform the actual read */
            memcpy(buffer + offset, block + block_offset, bytes);
            block_offset = 0;
            i++;
            offset += bytes;
            remain -= bytes;
        }
        /* The offset associated with the file handle is
         * incremented accordingly */
        file->of_offset += to_read;
    }

    return (ssize_t)to_read;
}

int tfs_copy_to_external_fs(char const *source_path, char const *dest_path){

    FILE *dest;                                                          
    char buffer[BLOCK_SIZE];

    int source = tfs_open(source_path, 0);
    if (source == -1){
        return -1;
    }
    dest = fopen(dest_path, "w");
    if (dest == NULL){
        tfs_close(source);
        return -1;
    }

    ssize_t bytes_read;
    while((bytes_read = tfs_read(source, buffer, sizeof(buffer))) != 0){
        if (bytes_read == -1){
            return -1;
        }
        fwrite(buffer, (size_t)bytes_read, sizeof(char), dest);                                      /*sizeof char é 1*/
    }

    tfs_close(source);
    fclose(dest);

    return 0;

}
