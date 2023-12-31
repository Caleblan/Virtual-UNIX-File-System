#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Disk.h"

unsigned int diskSize = 0;
unsigned int diskBlocks = 0;

char *disk2 = NULL;

/**
 * Write to a block located somewhere on a disk.
 */
void diskWrite(unsigned int diskLocation, char data[BLOCK_SIZE])
{
    //If the addresses location exceeds the addressable blocks.
    if(diskLocation >= diskBlocks)
    {
        printf("Memory location exceeds disk size (MemoryLocation: %d, DiskSize: %d).\n", diskLocation, diskBlocks);
        return;
    }

    //Method requires that the block size is only a single block size.
    if(strlen(data) > BLOCK_SIZE)
    {
        printf("Data length excceeds the block size of the disk (BlockSize: %d)", BLOCK_SIZE);
        return;
    }

    int BLOCK_END = (diskLocation * BLOCK_SIZE) + BLOCK_SIZE;

    int counter = 0;
    
    //Copy data from disk to a buffer
    for(int i = diskLocation * BLOCK_SIZE; i < BLOCK_END; i++)
    {
        disk2[i] = data[counter++];
        //printf("Index: %d , [%c, %d]\n", i, disk2[i], disk2[i]);
    }

    //printf("\n");

    return;

}

char *diskRead(unsigned int diskLocation)
{
    //If the addresses location exceeds the addressable blocks.
    if(diskLocation >= diskBlocks)
    {
        printf("Memory location exceeds disk size (MemoryLocation: %d, DiskSize: %d).\n", diskLocation, diskBlocks);
        return NULL;
    }

    char *readData = calloc(BLOCK_SIZE, sizeof(char));

    int BLOCK_END = (diskLocation * BLOCK_SIZE) + BLOCK_SIZE;

    int counter = 0;
    
    //Copy data from disk to a buffer
    for(int i = diskLocation * BLOCK_SIZE; i < BLOCK_END; i++)
    {
        readData[counter++] = disk2[i];
        //printf("READ [%d]: %d\n", i ,readData[counter-1]);
    }

    //printf("\n");  
    
    return readData;
}


