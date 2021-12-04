#include <stdlib.h>
#include <stdio.h>
#include "Disk.h"

/**
 * Write to a block located somewhere on a disk.
 */
void diskWrite(unsigned int diskLocation, char** blockData)
{
    //If the addresses location exceeds the addressable blocks.
    if(diskLocation > diskBlocks)
    {
        printf("Memory location exceeds disk size (MemoryLocation: %d, DiskSize: %d).\n", diskLocation, diskBlocks);
        return;
    }

    char* data = *blockData;

    int BLOCK_END = (diskLocation * BLOCK_SIZE) + BLOCK_SIZE;

    int counter = 0;
    
    //Copy data from disk to a buffer
    for(int i = diskLocation * BLOCK_SIZE; i < BLOCK_END; i++)
    {
        disk2[counter++] = data[i];
    }

}

char *diskRead(unsigned int diskLocation)
{
    //If the addresses location exceeds the addressable blocks.
    if(diskLocation > diskBlocks)
    {
        printf("Memory location exceeds disk size (MemoryLocation: %d, DiskSize: %d).\n", diskLocation, diskBlocks);
        return;
    }

    char *readData = malloc(BLOCK_SIZE * sizeof(char));

    int BLOCK_END = (diskLocation * BLOCK_SIZE) + BLOCK_SIZE;

    int counter = 0;
    
    //Copy data from disk to a buffer
    for(int i = readLocation * BLOCK_SIZE; i < BLOCK_END; i++)
    {
        readData[counter++] = disk2[i];
    }

    return readData;
}


