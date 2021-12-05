#include <stdlib.h>
#include <stdio.h>
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

    printf("%s\n", data);

    int BLOCK_END = (diskLocation * BLOCK_SIZE) + BLOCK_SIZE;

    int counter = 0;
    
    //Copy data from disk to a buffer
    for(int i = diskLocation * BLOCK_SIZE; i < BLOCK_END; i++)
    {
        //Don't keep going in the loop if more characters
        if(disk2[i] == '\0')
        {
            data[counter] = disk2[i];
            break;
        }

        data[counter++] = disk2[i];
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
        if(disk2[i] == '\0')
        {
            readData[counter] = disk2[i];
            break;
        }

        readData[counter++] = disk2[i];
    }

    return readData;
}


