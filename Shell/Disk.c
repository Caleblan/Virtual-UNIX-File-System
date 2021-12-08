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
void diskWrite(unsigned int diskLocation, char** blockData)
{
    //If the addresses location exceeds the addressable blocks.
    if(diskLocation >= diskBlocks)
    {
        printf("Memory location exceeds disk size (MemoryLocation: %d, DiskSize: %d).\n", diskLocation, diskBlocks);
        return;
    }

    char* data = *blockData;

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
        //Don't keep going in the loop if more characters
        if(data[counter] == '\0')
        {
            printf("NULL : %d\n", data[counter]);
            disk2[i] = 0;
        }
        else
        {
            printf("NOT%d\n", data[counter]);
            disk2[i] = data[counter++];
        }
    }

}

char *diskRead(unsigned int diskLocation)
{
    //If the addresses location exceeds the addressable blocks.
    if(diskLocation >= diskBlocks)
    {
        printf("Memory location exceeds disk size (MemoryLocation: %d, DiskSize: %d).\n", diskLocation, diskBlocks);
        return NULL;
    }

    char *readData = malloc(BLOCK_SIZE * sizeof(char));

    int BLOCK_END = (diskLocation * BLOCK_SIZE) + BLOCK_SIZE;

    int counter = 0;
    
    //Copy data from disk to a buffer
    for(int i = diskLocation * BLOCK_SIZE; i < BLOCK_END; i++)
    {
        readData[counter++] = disk2[i];
    }

    return readData;
}


