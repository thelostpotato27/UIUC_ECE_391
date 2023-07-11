#include "filesys.h"

/*
 * DESCRIPTION: Initializes segments of file system, such as boot block, inodes, etc.
 * See Appendix A of the ECE 391 MP3 writeup for the file organization.
 *
 * INPUTS: Starting address of file system.
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: Initializes file system structs.
 * 
 */
void filesys_init(uint32_t start_addr){
    boot_block = (boot_block_t*)start_addr;         //init datablock pointer to start address
    data_entry = *(dentry_t*)(start_addr + 64); // starts at first directory entry, boot block is 64B
    inode_start = (inode_t*)(start_addr + BLOCK_SIZE);      //init inode_t pointer to 1 after start address
    data_block_start = (datablock_t *)(start_addr + (boot_block->inode_nums)*BLOCK_SIZE + BLOCK_SIZE);       //init data block pointer to 1 after inodes
}

/*
 * DESCRIPTION: Opens a file to enable the data to be read.
 *
 * INPUTS: Name of the file.
 * 
 * OUTPUTS: 0 upon success, -1 on failure
 * 
 * SIDE EFFECTS: Points data_entry to file, allowing the currently opened file to be accessed.
 * 
 */
int32_t file_open(const uint8_t* fname){
    // if(read_dentry_by_name(fname, &data_entry) == 0) return 0;
    // else return -1;

    return 0;
}

/* Closes file. Will be further implemented in Checkpoint 3.3. */
// fd is file descriptor
int32_t file_close(int32_t fd){
    return 0;
}

/*
* DESCRIPTION: Reads data from file. Update position in open file.
*/
int32_t file_read(int32_t fd, void* buf, int32_t bytes)
{
    memset((uint8_t*) buf, NULL, bytes);
    
    if(buf == NULL)
        return -1;
    
    uint32_t bytes_read = read_data(terminals[disp_terminal].pcb->open_files[fd].inode_num, terminals[disp_terminal].pcb->open_files[fd].file_pos, buf, bytes);

    if (bytes_read < 0) return -1;
    // update file position
    terminals[disp_terminal].pcb->open_files[fd].file_pos += bytes_read;

    return bytes_read;
}

/*
 * DESCRIPTION: Reads data from file.
 *
 * INPUTS: fd -- currently used as file's inode, implementation will change once PCB used.
 * buf -- buffer to be written to, bytes -- the # of bytes to read, offset -- where in file
 * to start reading.
 * 
 * OUTPUTS: 0 upon success, -1 on failure
 * 
 * SIDE EFFECTS: Reads data from file into buffer.
 * 
 */
/* Reads 'bytes' bytes from 'buf.' */
int32_t data_read(int32_t fd, void* buf, uint32_t bytes, uint32_t offset) {
    if(fd >= boot_block->inode_nums) return -1;
    if (read_data(fd, offset, buf, bytes) == -1) return -1;
    else return 0;
}

/* Should just return (see MP3 writup Checkpoint 2). */
int32_t file_write(int32_t fd, const void* buf, int32_t bytes){
    return -1;
}

/*
 * DESCRIPTION: Opens directory for file access.
 *
 * INPUTS: file -- directory name
 * 
 * OUTPUTS: 0 upon success, -1 on failure
 * 
 * SIDE EFFECTS: Opens directory for access.
 * 
 */
int32_t dir_open(const uint8_t* file){
    // if(read_dentry_by_name(file, &data_entry) == 0) return 0; // check for existence
    // else if (data_entry.filetype != DIRECTORY_ACCESS) return -1; // checks if directory
    // return -1;
    return 0;
}

/* Should just return (see MP3 writup Checkpoint 2). */
int32_t dir_close(int32_t fd){
    return 0;
}

/*
 * DESCRIPTION: Reads next file name into buffer.
 *
 * INPUTS: fd - file descriptor (to be implemented), buf - to be written to,
 * bytes - unused
 * 
 * OUTPUTS: 0 upon success, -1 on failure
 * 
 * SIDE EFFECTS: Fills buf with filename.
 * 
 */

int32_t dir_read(int32_t fd, void* buf, int32_t bytes){

    dentry_t dentry;

    uint32_t index =  terminals[disp_terminal].pcb->open_files[fd].file_pos;
    int32_t ret = read_dentry_by_index(index, &dentry);

    if (ret == -1) return 0;

    memcpy((uint8_t*)buf, &(dentry.filename), MAX_NAME_LENGTH);

    // goes to next file
    index++;
    terminals[disp_terminal].pcb->open_files[fd].file_pos = index;

    if (index < 63) return bytes; // 63 directory entries

    return 0;
}

/* Should just return (see MP3 writup Checkpoint 2). */
int32_t dir_write(int32_t fd, const void* buf, int32_t bytes){
    return -1;
}

/*
 * DESCRIPTION: Given a string, reads data from directory entry with that name.
 *
 * INPUTS: fname -- name of file, dentry -- directory entry to write to
 * 
 * OUTPUTS: 0 upon success, -1 on failure
 * 
 * SIDE EFFECTS: Modifies dentry with dentry of the given file.
 * 
 */

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    if(!dentry) return -1;
    if(fname == NULL) return -1;
    if (strlen((int8_t *)fname) > MAX_NAME_LENGTH) return -1;

    int i;
    int dir_entries = boot_block->dir_entries;

    for(i = 0; i < dir_entries; i++){
        // dentry_t try_dentry = boot_block->d[i];
        // int cast to get compiler to stop complaining
        if(strncmp((int8_t*) fname, (int8_t*)boot_block->d[i].filename, MAX_NAME_LENGTH) == 0){
            strncpy((int8_t*)dentry->filename, (int8_t*)boot_block->d[i].filename, MAX_NAME_LENGTH); 
            dentry->filetype = boot_block->d[i].filetype;
            dentry->inode_num = boot_block->d[i].inode_num;
            return 0;
        }
    }
    return -1; // file not found
}

/*
 * DESCRIPTION: Given an index, reads data from directory entry with that index.
 *
 * INPUTS: index -- index of file, dentry -- directory entry to write to
 * 
 * OUTPUTS: 0 upon success, -1 on failure
 * 
 * SIDE EFFECTS: Modifies dentry with dentry of the given file.
 * 
 */

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    if (index >= boot_block->dir_entries) return -1;
    // dentry_t try_dentry = boot_block->d[index];
    strncpy((int8_t*)dentry->filename,(int8_t*)boot_block->d[index].filename, MAX_NAME_LENGTH);
    dentry->filetype = boot_block->d[index].filetype;
    dentry->inode_num = boot_block->d[index].inode_num;
    return 0;
}

/*
 * DESCRIPTION: Reads data from file into buffer.
 *
 * INPUTS: inode -- inode of file, offset -- offset in bytes from file start,
 * buf -- buffer to write to, length -- number of bytes to write
 * 
 * OUTPUTS: 0 upon success, -1 on failure
 * 
 * SIDE EFFECTS: Fills buffer with chars.
 * 
 */

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){

    // FIX TO ACCOUNT FOR OFFSET

    int i = 0;
    uint32_t block_index = offset / BLOCK_SIZE; // offset in block
    uint32_t offset_in_block = offset % BLOCK_SIZE;

    if (inode >= boot_block->inode_nums) return -1;
    inode_t* try_inode = (inode_t *)(&inode_start[inode]);

    if (buf == NULL) return -1;

    // if length is over the remaining length of the file, then only copy up until end
    int32_t remain_length = (try_inode->data_length - offset) < length ? (try_inode->data_length - offset) : length; // data length in bytes
    int32_t ret = remain_length;
    if (remain_length < 0) return -1; // should not happen

    while(remain_length > 0) {
        // user wants to copy more than remaining data
        if (remain_length > BLOCK_SIZE) {
            if (i == 0) {
                memcpy(buf, &(data_block_start[try_inode->data_block[block_index + i]].data[offset_in_block]), BLOCK_SIZE - offset_in_block);
                i++;
                remain_length -= (BLOCK_SIZE - offset_in_block); // will copy from next block in next loop
            }
            else {
                memcpy(&(buf[(i - 1) * BLOCK_SIZE + (BLOCK_SIZE - offset_in_block)]), &(data_block_start[try_inode->data_block[block_index + i]].data[0]), BLOCK_SIZE);
                i++;
                remain_length -= BLOCK_SIZE; // will copy from next block in next loop
            }

        }
        else {
            if (i == 0) {
                memcpy(buf, &(data_block_start[try_inode->data_block[block_index + i]].data[offset_in_block]), remain_length);
                break;
            }
            else {
                memcpy(&(buf[(i - 1) * BLOCK_SIZE + (BLOCK_SIZE - offset_in_block)]), &(data_block_start[try_inode->data_block[block_index + i]].data[0]), remain_length);
                break;
            }

        }
    }

    // returns bytes read
    return ret;
}

int32_t get_file_size(int32_t inode_index)
{
    return ((inode_t*)(&(inode_start[inode_index])))->data_length;
}
