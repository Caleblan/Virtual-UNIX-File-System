#ifndef DISK_H
#define DISK_H

#define BLOCK_SIZE 128

#include <stddef.h>

void diskWrite(unsigned int diskLocation, char blockData[BLOCK_SIZE]);

char *diskRead(unsigned int diskLocation);

#endif