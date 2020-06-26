
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include "ext2_fs.h"
#define SUPERBLOCK_BYTE_OFFSET 1024

/*---------------------------------*/
struct ext2_super_block super_block;
struct ext2_inode       inode;
struct ext2_group_desc  group_desc;
/*---------------------------------*/
int Mount_FD;
/*---------------------------------*/
uint32_t Num_Blocks;
uint32_t Num_Inodes;
uint32_t Block_Size;
uint32_t Inode_Size;
uint32_t Blocks_Per_Group;
uint32_t INodes_Per_Group;
uint32_t Num_Indir_Blocks;
uint32_t groups;
/*---------------------------------*/
void check(ssize_t temp){
    if(temp < 0){
        fprintf(stderr, "ERROR : pread\n");
        exit(1);
    }
}

void Superblock_Summary(){
    Num_Blocks = super_block.s_blocks_count;
    Num_Inodes = super_block.s_inodes_count;
    Block_Size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    Inode_Size = super_block.s_inode_size;
    Blocks_Per_Group = super_block.s_blocks_per_group;
    INodes_Per_Group = super_block.s_inodes_per_group;
    uint32_t Num_First_Empty_Inode = super_block.s_first_ino;

    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", Num_Blocks, Num_Inodes, 
           Block_Size, Inode_Size, Blocks_Per_Group, INodes_Per_Group, 
           Num_First_Empty_Inode); 
}


void Free_Block_Entries(){ 
    uint32_t i, j, index, num_blocks;
    uint8_t k;
    char bytes[Block_Size];
    for(i=0; i < groups; i++){
        ssize_t temp = pread(Mount_FD, bytes, Block_Size,  group_desc.bg_block_bitmap* Block_Size);
        check(temp);
        index = super_block.s_first_data_block + i * Blocks_Per_Group;
        num_blocks = (SUPERBLOCK_BYTE_OFFSET << super_block.s_log_block_size);
        for (j = 0; j < num_blocks; ++j) {
            k = 0;
            while(k < 8){
                if (!(bytes[j] & 1))
                  printf("BFREE,%u\n", index );
                bytes[j] = bytes[j] >> 1;
                k++; index++;
            }  
        }
    }
}

void DIR_ENT(uint32_t block, uint32_t num_I_Nodes){
    struct ext2_dir_entry temp_dir;
    uint32_t total = Block_Size * block, i = 0;
    while (i < Block_Size){
        ssize_t temp = pread(Mount_FD, &temp_dir, sizeof(struct ext2_dir_entry),  total + i);
        check(temp);
        if (temp_dir.inode) {
              printf("DIRENT,%u,%u,%u,%u,%u,'%s'\n",
                     num_I_Nodes, i, temp_dir.inode, temp_dir.rec_len,
                     temp_dir.name_len, temp_dir.name);
        }
        i = i + temp_dir.rec_len;
    }
}


void INDIR_B1(uint32_t num, uint32_t offset, uint32_t block) {
    uint32_t i, temp_block[Num_Indir_Blocks];
    ssize_t tt = pread(Mount_FD, temp_block, Block_Size, SUPERBLOCK_BYTE_OFFSET + (block - 1) * Block_Size);
    check(tt);
    for (i = 0; i < Num_Indir_Blocks; ++i) {
        if (temp_block[i]) {
            printf("INDIRECT,%u,%d,%u,%u,%u\n", num, 1, offset+i, block, temp_block[i]);
        }
    }
}

void INDIR_B2(uint32_t num, uint32_t offset, uint32_t block) {
    uint32_t i, temp_block[Num_Indir_Blocks];
    ssize_t tt = pread(Mount_FD, temp_block, Block_Size, SUPERBLOCK_BYTE_OFFSET + (block - 1) * Block_Size);
    check(tt);
    for (i = 0; i < Num_Indir_Blocks; i++) {
        if (temp_block[i]) {
            printf("INDIRECT,%u,%d,%u,%u,%u\n", num, 2, offset+i, block, temp_block[i]);
            uint32_t temp = temp_block[i];  
            uint32_t temp_block[Num_Indir_Blocks];
            tt = pread(Mount_FD, temp_block, Block_Size, SUPERBLOCK_BYTE_OFFSET + (temp - 1) * Block_Size);
            check(tt);
            uint32_t j;
            for (j = 0; j < Num_Indir_Blocks; j++) {
                if (temp_block[j]) {
                    printf("INDIRECT,%u,%d,%u,%u,%u\n", num, 1, offset+j, temp, temp_block[j]);
                }
            }
        }
    }
}


void INDIR_B3(uint32_t num, uint32_t offset, uint32_t block) {
    uint32_t i,j,k,temp_block[Num_Indir_Blocks];
    ssize_t tt = pread(Mount_FD, temp_block, Block_Size, SUPERBLOCK_BYTE_OFFSET + (block - 1) * Block_Size);
    check(tt);
    for (i = 0; i < Num_Indir_Blocks; i++) {
        if (temp_block[i]) {
            printf("INDIRECT,%u,%d,%u,%u,%u\n", num, 3, offset+i, block, temp_block[i]);
            uint32_t temp = temp_block[i]; 
            uint32_t temp_block[Num_Indir_Blocks];
            tt = pread(Mount_FD, temp_block, Block_Size, SUPERBLOCK_BYTE_OFFSET + (temp - 1) * Block_Size);
            check(tt);
            for (j = 0; j < Num_Indir_Blocks; j++) {
                if (temp_block[j]) {
                    printf("INDIRECT,%u,%d,%u,%u,%u\n", num, 2, offset+j, temp, temp_block[j]);
                    uint32_t temp2 = temp_block[j];   
                    uint32_t temp_block[Num_Indir_Blocks];
                    tt = pread(Mount_FD, temp_block, Block_Size, SUPERBLOCK_BYTE_OFFSET + (temp2 - 1) * Block_Size);
                    check(tt);
                    for (k = 0; k < Num_Indir_Blocks; k++) {
                        if (temp_block[k]) {
                            printf("INDIRECT,%u,%d,%u,%u,%u\n", num, 1, offset+k, temp2, temp_block[k]);
                        }
                    }
                }
            }
        }
    }
}

void format_time(uint32_t sec, char *buff){
    time_t temp = sec;
    struct tm* time = gmtime(&temp);
    strftime(buff, 32, "%m/%d/%y %H:%M:%S", time);
}
  
void inode_summary(uint32_t diff, uint32_t num_I_Nodes){
    
    ssize_t tt = pread(Mount_FD, &inode, sizeof(struct ext2_inode), 
        group_desc.bg_inode_table * Block_Size + diff * sizeof(struct ext2_inode));
    check(tt);
  
    if (inode.i_mode && inode.i_links_count) {
        char file_type;
        if ((inode.i_mode & 0xF000) == 0xA000)
            file_type = 's';
        else if ((inode.i_mode & 0xF000) == 0x8000)
            file_type = 'f';
        else if ((inode.i_mode & 0xF000) == 0x4000)
            file_type = 'd';
        else
            file_type = '?';
    
        char creation_time[32], modification_time[32], access_time[32];
        format_time(inode.i_ctime, creation_time);
        format_time(inode.i_mtime, modification_time);
        format_time(inode.i_atime, access_time);
        
        printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u",
               num_I_Nodes, file_type, inode.i_mode & 0x0FFF, inode.i_uid,
               inode.i_gid, inode.i_links_count, creation_time, modification_time, 
               access_time, inode.i_size, inode.i_blocks);
        int i;
        if (file_type != '?'){
            if (file_type == 's'){
                if (inode.i_size > 60) {
                    for (i = 0; i < EXT2_N_BLOCKS; i++)
                    printf(",%u",inode.i_block[i]);
                }
            }
            else{
                for (i = 0; i < EXT2_N_BLOCKS; i++)
                printf(",%u",inode.i_block[i]);
            }

        }
        printf("\n");
        Num_Indir_Blocks = Block_Size/sizeof(uint32_t);
        for (i = 0; i < EXT2_N_BLOCKS; i++) {
            if ( i < EXT2_NDIR_BLOCKS){
              if (inode.i_block[i] && file_type == 'd')
                DIR_ENT(inode.i_block[i], num_I_Nodes);
            }
            else{ // for indirect entries
                if(inode.i_block[i]){
                    int temp_val = 12;
                    if (i == EXT2_IND_BLOCK){
                        INDIR_B1(num_I_Nodes, temp_val, inode.i_block[i]);
                    }   
                    else if (i == EXT2_DIND_BLOCK)  {
                        temp_val = temp_val + 256; 
                        INDIR_B2(num_I_Nodes, temp_val, inode.i_block[i]);
                    }
                    else {
                        temp_val = temp_val +  256 + 256*256;
                        INDIR_B3(num_I_Nodes, temp_val, inode.i_block[i]);
                    }
                }
            }  
        }
    }
}

void Free_Inode_Entries(){ 
    uint32_t i,j, first, curr;
    uint32_t size = super_block.s_inodes_per_group / 8;
    uint8_t count;
    char bytes[size];
    for(i=0; i < groups; i++){
        ssize_t tt = pread(Mount_FD, bytes, size, group_desc.bg_inode_bitmap * Block_Size);
        check(tt);
        first = curr  = i * size * 8 + 1;
        for (j = 0; j < size; j++) {
            count = 0;
            while (count < 8){
                uint32_t diff = curr - first;
                if (bytes[j] & 1)
                    inode_summary(diff, curr);
                else
                    printf("IFREE,%u\n", curr);
                bytes[j] = bytes[j] >> 1;
                curr++;
                count++;
            }
        }
    }
}


void group_summary(){
    
    ssize_t tt = pread(Mount_FD, &group_desc, sizeof(struct ext2_group_desc), 
        (SUPERBLOCK_BYTE_OFFSET + sizeof(struct ext2_super_block)));
    check(tt);
    groups = ceil((double) Num_Blocks/Blocks_Per_Group);
    uint32_t i;
    for(i=0; i < groups; i++){
      printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", i, super_block.s_blocks_count, super_block.s_inodes_count,
             group_desc.bg_free_blocks_count, group_desc.bg_free_inodes_count, group_desc.bg_block_bitmap, 
             group_desc.bg_inode_bitmap, group_desc.bg_inode_table); 
    }

      Free_Block_Entries();
      Free_Inode_Entries();       

}
  
int main(int argc, char **argv){
    if (argc != 2) {
        fprintf(stderr, "ERROR : Ivalid arguments\n");
        exit(1);
    }
    if ((Mount_FD = open(argv[1], O_RDONLY)) == -1) {
        fprintf(stderr, "ERROR : No mounted\n");
        exit(1);
    }
    ssize_t temp = pread(Mount_FD, &super_block, 
        sizeof(struct ext2_super_block), SUPERBLOCK_BYTE_OFFSET);
    check(temp);
    Superblock_Summary();
    group_summary();
    
    exit(0);
    return 1;
}