#include <stdlib.h>
#include "Disk.h"
#include "CFFS.h"


int main(int argsc, char** argsv)
{
    char* command;

    //If there are no command arguements, then ask person for what size of file system they want.
    if(argsc == 1)
    {
        printf("How big would you like to make your disk (in bytes)?\n");

        char input;

        int length = 0;

        //
        while ((input = getchar()) != '\n')
        {
            command = realloc(command, length + 1);
            command[length++] = input;
        }

        //Parse integer and 
        diskSize = atoi(command);
        free(command);
    }
    else
    {
        //TODO add error checking here.
        diskSize = atoi(argsv[1]);
    }

    char disk[diskSize] = {};
    disk2 = disk;

    diskBlocks = diskSize / diskBlocks;

    //Used to store an array of structs.
    disk = calloc(diskBlocks, sizeof(char));
}

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

char[] diskRead(unsigned int diskLocation)
{
    //If the addresses location exceeds the addressable blocks.
    if(diskLocation > diskBlocks)
    {
        printf("Memory location exceeds disk size (MemoryLocation: %d, DiskSize: %d).\n", diskLocation, diskBlocks);
        return;
    }

    char readData[BLOCK_SIZE];

    int BLOCK_END = (diskLocation * BLOCK_SIZE) + BLOCK_SIZE;

    int counter = 0;
    
    //Copy data from disk to a buffer
    for(int i = readLocation * BLOCK_SIZE; i < BLOCK_END; i++)
    {
        readData[counter++] = disk2[i];
    }

    return readData;
}


