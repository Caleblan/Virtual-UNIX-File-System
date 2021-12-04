#ifndef DISK_H
#define DISK_H

#define BLOCK_SIZE 128

#include <stddef.h>

unsigned int diskSize = 0;
unsigned int diskBlocks = 0;

char *disk2 = NULL;

void diskWrite(unsigned int diskLocation, char** blockData);

char *diskRead(unsigned int diskLocation);

#endif