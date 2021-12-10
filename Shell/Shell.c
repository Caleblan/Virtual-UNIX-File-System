#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "Shell.h"
#include "Disk.h"

bool errorFound = false;

//Used to continuously prompt user until stated otherwise.
bool active = true;

/**
 * Reads input from stdin one character at a time and dynamically
 * allocates enough memory to store command. 
 * 
 * Note: This function does not make null terminated strings at this point
 *      as due to realloc, the address may change so we need to store the 
 *      whole string initially before altering it.
 * 
 * @return Complete command entered by user. 
 */
char *getUserInput(void)
{
    char *command = NULL;

    char input;

    unsigned int length = 0;

    //Gets all characters before newline from stdin.
    while ((input = getchar()) != '\n')
    {
        command = realloc(command, length + 1);
        command[length++] = input;
    }

    //Marks end of string.
    command = realloc(command, length + 1);
    command[length] = '\0';

    return command;
}

/**
 * Takes the inputed string by user and creates a list of null terminated strings.
 * 
 * Note: This done in place of the original commands allocated memory area, therefore
 *       altering the original input string.
 * 
 * @parsedCommandPtr Pointer to all the address' containing each word in
 * user input.
 * 
 * @return An array of addresses pointing to the beginning of each now-null-terminated word in
 * the user inputed comamand. 
 */
char **tokenize(char **commandPtr)
{
    char *command = *commandPtr;

    char **parsedCommand = NULL;

    /*Gets first input value from user inputed string. Note that there will always be atleast 
    a single element at this point since we check for null input in main program loop. */
    parsedCommand = malloc(sizeof(char *));
    unsigned int parseCommandIndex = 0;
    parsedCommand[parseCommandIndex] = strtok(command, " ");

    //Continues to get additional input.
    while (parsedCommand[parseCommandIndex] != NULL)
    {
        parsedCommand = realloc(parsedCommand, (parseCommandIndex + 2) * sizeof(char *));
        parsedCommand[++parseCommandIndex] = strtok(NULL, " ");

        if (parsedCommand[parseCommandIndex] != NULL)
        {
            //Used if "" characters are found to make the string a single arguement.
            if (*parsedCommand[parseCommandIndex] == '\"')
            {

                parsedCommand[parseCommandIndex]++;
                char *temp = strtok(NULL, "\"");

                //Leaves the loop and says there is an error since there is not closing \"
                if (temp == NULL)
                {
                    errorFound = true;
                    printf("No closing \" character has been found.\n");
                    break;
                }
                //Turns the last character back into space to keep string going.
                else
                {
                    temp -= 1;
                    *temp = ' ';
                }
            }
        }
    }
    return parsedCommand;
}

/**
 * Parses user command to a command.
 * 
 * @parsedCommandPtr Pointer to all the addresses containing each word in
 * user input.
 */
void parseCommand(char ***parsedCommandPtr)
{
    char **parsedCommand = *parsedCommandPtr;

    if (strcmp(parsedCommand[0], "exit") == 0)
    {
        active = false;
    }
    else if(strcmp(parsedCommand[0], "disk_write") == 0)
    {
        diskWriteCommand(parsedCommandPtr);
    }
    else if(strcmp(parsedCommand[0], "create_partition") == 0)
    {
        createDisk(parsedCommandPtr);
    }
    else if(strcmp(parsedCommand[0], "format_disk") == 0)
    {
        formatDisk();
    }
    else if(strcmp(parsedCommand[0], "disk_read") == 0)
    {
        diskReadCommand(parsedCommandPtr);
    }
    else if(strcmp(parsedCommand[0], "make_file") == 0)
    {
        makeFile(parsedCommandPtr);
    }
    else if(strcmp(parsedCommand[0], "create_directory") == 0)
    {
        createDirectory(parsedCommandPtr);
    }
    else if(strcmp(parsedCommand[0], "write_file") == 0)
    {
        writeFile(parsedCommandPtr);
    }
    else if(strcmp(parsedCommand[0], "delete_file") == 0)
    {
        deleteFile(parsedCommandPtr);
    }
    //Temporary
      else if(strcmp(parsedCommand[0], "format") == 0)
    {
        formatDisk();
    }
    else
    {
        executeFile(parsedCommandPtr);
    }
}

/**
 * Creates a child process by system call to run executable,
 * which the parent program will wait until the childs completion.
 * 
 * @parsedCommandPtr Pointer to all the address' containing each word in
 * user input.
 */
void executeFile(char ***parsedCommandPtr)
{
    char **parsedCommand = *parsedCommandPtr;

    int pid = fork();

    //If child process, change image of process and execute it.
    if (pid == 0)
    {
        int creationVal = execvp(parsedCommand[0], parsedCommand);

        //If there is not command like this, terminate child process and give user feedback.
        if (creationVal == -1)
        {
            printf("\'%s\' does not match any known command.\n", parsedCommand[0]);
            freeMemory(&parsedCommand);
            exit(1);
        }
    }
    //Wait till child process is done.
    else
    {
        waitpid(pid, NULL, 0);
    }
}

/**
 * Free's both the user input command as well as as the pointer containing all the
 * addresses to the beginning of each words first character.
 * 
 * Note: When calling free(ptr) for actual input string, we only need to 
 *       give the method the first arguement. This is because when we tokenize
 *       the user string, it is done in place of orignal allocated memory location,
 *       and since technically it is only a single allocation (with reallocations), 
 *       we only have to free once.
 * 
 * @parsedCommandPtr Pointer to all the addresses containing each word in
 * user input.
 */
void freeMemory(char ***parsedCommandPtr)
{
    char **parsedCommand = *parsedCommandPtr;

    free(parsedCommand[0]);

    free(parsedCommand);
}

/**
 * 
 * @parsedCommandPtr 
 */
void diskWriteCommand(char ***parsedCommandPtr)
{
    if(disk2 == NULL)
    {
        printf("No disk partition has been created. Try creating one with \'create_partition [unsigned int arguement]\'.\n");
        return;
    }
  
    char** parsedCommand = *parsedCommandPtr;  

    if(parsedCommand[2] == NULL)
    {
        printf("Too few arguements. Command should follow form \'disk_write [unsigned int arguement] [char[128]]\'.\n");
        return;
    }
    else if(parsedCommand[3] != NULL)
    {
        printf("Too many arguements. Command should follow form \'disk_write [unsigned int arguement] [char[128]]\'.\n");
        return;
    }

    unsigned int blockAddress;

    //Checks if data is greater than the block size.
    if(strlen(parsedCommand[1]) > BLOCK_SIZE)
    {
        printf("Inputted data exceeds disk block size (BlockSize: %d).\n", BLOCK_SIZE);
        return;
    }

    //TODO check for greater than input.
    sscanf(parsedCommand[1], "%u", &blockAddress);

    diskWrite(blockAddress, &parsedCommand[2]);

    printf("'%s' written to disk (Block Location: %d).\n", parsedCommand[2], blockAddress);

    //TODO if args > 3 then return.
}


void diskReadCommand(char ***parsedCommandPtr)
{
 //TODO allow multiple string arguements if I get time.

    if(disk2 == NULL)
    {
        printf("No disk partition has been created. Try creating one with \'create_partition [unsigned int arguement]\'.\n");
        return;
    }

    char** parsedCommand = *parsedCommandPtr;

    unsigned int blockAddress;

    sscanf(parsedCommand[1], "%u", &blockAddress);
    char *block = diskRead(blockAddress);

    if(block != NULL)
    {
        printf("'%s' read from disk (Block Location: %d).\n", block, blockAddress);
    }

    //free(block);
}

/**
 * Creates 
 */
void createDisk(char ***parsedCommandPtr)
{
    if(disk2 != NULL)
    {
        printf("Disk partition has already been created (DiskSize (in bytes): %d, DiskBlocks: %d).\n", diskSize, diskBlocks);
        return;
    }
    
    char **parsedCommand = *parsedCommandPtr;

    if(parsedCommand[1] == NULL)
    {
        printf("Too few arguements. Command should follow form \'create_partition [unsigned int arguement]\'.\n");
        return;
    }
    else if(parsedCommand[2] != NULL)
    {
        printf("Too many arguements. Command should follow form \'create_partition [unsigned int arguement]\'.\n");
        return;
    }


    //TODO add error checking here.
    diskSize = atoi(parsedCommand[1]);

    diskBlocks = diskSize / BLOCK_SIZE;

    //Used to store an array of structs.
    disk2 = calloc(diskSize, sizeof(char));

    printf("Disk partition of size %d bytes has been created (BlockSize: %d, BlockCount: %d).\n", diskSize, BLOCK_SIZE, diskBlocks);
}



//TODO ASK IF WRITE TAKES IN AN INODE NUMBER.
void writeFile(char ***parsedCommandPtr)
{
    if(disk2 == NULL)
    {
        printf("No disk partition has been created. Try creating one with \'create_partition [unsigned int arguement]\'.\n");
        return;
    }

    char **parsedCommand = *parsedCommandPtr;

    if(parsedCommand[1] == NULL || parsedCommand[2] == NULL)
    {
        printf("Too few arguements. Command should follow form \'write_file [unsigned int arguement] [String]\'.\n");
        return;
    }
    else if(parsedCommand[3] != NULL)
    {
        printf("Too many arguements. Command should follow form \'write_file [unsigned int arguement] [String]\'.\n");
        return;
    }

    int inodeIndex = atoi(parsedCommand[1]);

    unsigned int inodeCount = getInodeCount();

    //Print error message tries to user if inodeIndex is greater than alotted time
    if(inodeIndex >= inodeCount)
    {
        printf("Inode index %d exceeds \'%d\' inode count.\n", inodeIndex, inodeCount);
        return;
    }
    //If node is currently unallocated, give error message to user.
    else if(!existingInode(inodeIndex))
    {
        printf("Inode at index %d is not currently allocated to a file.\n", inodeIndex);
        return;
    }
    // //TODO allow rewrites
    // else
    // {

    // }

    //Used only if the string is larger than the BLOCK_SIZE
    char *newString = parsedCommand[2];

    //Gets the block index of first dataBlocKBitmap
    int dataBitmapIndex = (2 + inodeCount);

    unsigned int fileBlockCount = 0;
    unsigned int stringLenth = 0;

    char *inode = diskRead(2 + inodeIndex);

    //Assign to direct pointers
    for(int i = 0; i < 4; i++)
    {
        char *dataBitmapBlock = diskRead(dataBitmapIndex);
        
        //TODO allow for multiple datablocksBitmaps
        int dataBitmapIndex = bitmapSearch(&dataBitmapBlock);
        int dataBlockIndex = (3 + inodeCount);

        stringLenth = strlen(newString);
        if(stringLenth != 0)
        {
            if(dataBlockIndex == -1)
            {
                printf("No more data blocks are available.\n");
                free(dataBitmapBlock);
                return;
                //TODO check if there is another datablock group
            }
            //Write new dataGroupBitmap to disk.
            char dataBitmap[BLOCK_SIZE] = {0};
            printf("%d\n", dataBitmapBlock[dataBitmapIndex / 8]);
            memcpy(&dataBitmap, dataBitmapBlock, BLOCK_SIZE);
            diskWrite(2 + inodeCount, dataBitmap);
            free(dataBitmapBlock);

            //Write to part of string data to allocated data block.
            char blockData[BLOCK_SIZE] = {0};
            if(strlen(newString) > BLOCK_SIZE)
            {
                memcpy(&blockData, newString, BLOCK_SIZE);
                diskWrite(dataBlockIndex, blockData);
                newString += BLOCK_SIZE;
            }
            else
            {
                memcpy(&blockData, newString, stringLenth);
                diskWrite(dataBlockIndex, blockData);
                newString += stringLenth;
            }

            //Split data block counter to 4 bytes.
            inode[((i + 1) * 4)] = (dataBlockIndex >> 24) & 0xFF;
            inode[((i + 1) * 4) + 1] = (dataBlockIndex >> 16) & 0xFF;
            inode[((i + 1) * 4) + 2] = (dataBlockIndex >> 8) & 0xFF;
            inode[((i + 1) * 4) + 3] = dataBlockIndex & 0xFF;

            fileBlockCount++;
        }
        //Set inode pointer to null
        else
        {
            printf("Here index %d\n", i);

            //Split inode count into four chars.
            inode[((i + 1) * 4)] = 0;
            inode[((i + 1) * 4) + 1] = 0;
            inode[((i + 1) * 4) + 2] = 0;
            inode[((i + 1) * 4) + 3] = 0;
        }
    }

    unsigned int dataBlockIndex;

    //If there is still more characters in the string, allocate a datablock of addresses.
    if(strlen(newString) > 0)
    {
        char *dataBitmapBlock = diskRead(dataBitmapIndex);
        int dataBitmapIndex = bitmapSearch(&dataBitmapBlock);
        //dataBlockIndex = (3 + inodeCount) + dataBitmapIndex;

        //Write new dataGroupBitmap to disk.
        char dataBitmap[BLOCK_SIZE] = {0};
        printf("%d\n", dataBitmapBlock[dataBitmapIndex / 8]);
        memcpy(&dataBitmap, dataBitmapBlock, BLOCK_SIZE);
        diskWrite(2 + inodeCount, dataBitmap);
        free(dataBitmapBlock);

        //If no open position on first
        if(dataBitmapIndex == -1)
        {
            printf("No more data blocks are available.\n");
            free(dataBitmapBlock);
            return;
        }

        //Sets indirect pointer for inode. Note: Indexs are realtive, so you have to add a base to it when reading.
        inode[16] = (inodeIndex >> 24) & 0xFF;
        inode[17] = (inodeIndex >> 16) & 0xFF;
        inode[18] = (inodeIndex >> 8) & 0xFF;
        inode[19] = inodeIndex & 0xFF;
    }
    else
    {

        //TODO temporary fix, need to still adjust file size properly.
        printf("New file has been created with size %d blocks.\n", fileBlockCount);
        return;
    }

    // char *dataBitmapBlock = diskRead(dataBitmapIndex);
    // int dataBitmapIndex = bitmapSearch(&dataBitmapBlock);

    char *indirectPointer = diskRead(dataBlockIndex);

    //Keep assigning blocks until either blocks run out or string runs out.
    for(int i = 0; i < BLOCK_SIZE / 4; i++)
    {
        stringLenth = strlen(newString);

        //If string has run out before going through all blocks, leave loop.
        if(strlen(newString) == 0)
        {
            break;
        }

        //Set datablock pointer.
        char *dataBitmapBlock = diskRead(dataBitmapIndex);
        int dataBitmapIndex = bitmapSearch(&dataBitmapBlock);
        free(dataBitmapBlock);
        
        //If no open position on first
        if(dataBitmapIndex == -1)
        {
            printf("No more data blocks are available.\n");
            free(dataBitmapBlock);
            return;
        }

        //Split inode count into four chars.
        indirectPointer[((i + 1) * 4)] = (inodeIndex >> 24) & 0xFF;
        indirectPointer[((i + 1) * 4) + 1] = (inodeIndex >> 16) & 0xFF;
        indirectPointer[((i + 1) * 4) + 2] = (inodeIndex >> 8) & 0xFF;
        indirectPointer[((i + 1) * 4) + 3] = inodeIndex & 0xFF;

        fileBlockCount++;

        if(stringLenth > BLOCK_SIZE)
        {
            newString += BLOCK_SIZE;
        }
        else
        {
            newString += stringLenth;
        }
    }

    //Write indirect pointer block onto disk.
    char indirectPtr[BLOCK_SIZE] = {0};
    memcpy(&indirectPtr, indirectPointer, BLOCK_SIZE);
    diskWrite(dataBlockIndex , indirectPtr);
    free(indirectPointer);

    //Split fileSize into four chars for inode.
    inode[0] = (fileBlockCount >> 24) & 0xFF;
    inode[1] = (fileBlockCount >> 16) & 0xFF;
    inode[2] = (fileBlockCount >> 8) & 0xFF;
    inode[3] = fileBlockCount & 0xFF;

    //Write inode to disk.
    char inodeArr[BLOCK_SIZE] = {0};
    memcpy(&inodeArr, inode, BLOCK_SIZE);
    diskWrite(2 + inodeIndex, inodeArr);
    free(inode);

    //If the string length is still greater, clip file as there are no more data blocks to use.
    if(strlen(newString) > 0)
    {
        printf("File exceeds maximum block size so file has been clipped.\n");
    }
    
    printf("New file has been created with size %d blocks.\n", fileBlockCount);
}




void deleteFile(char ***parsedCommandPtr)
{
    if(disk2 == NULL)
    {
        printf("No disk partition has been created. Try creating one with \'create_partition [unsigned int arguement]\'.\n");
        return;
    }
    
    char **parsedCommand = *parsedCommandPtr;

    if(parsedCommand[1] == NULL)
    {
        printf("Too few arguements. Command should follow form \'delete_file [unsigned int arguement]\'.\n");
        return;
    }
    else if(parsedCommand[2] != NULL)
    {
        printf("Too many arguements. Command should follow form \'delete_file [unsigned int arguement]\'.\n");
        return;
    }
 
    //TODO add error checking here.
    int inodeIndex = atoi(parsedCommand[1]);

    unsigned int inodeCount = getInodeCount();

    //Print error message tries to user if inodeIndex is greater than alotted time
    if(inodeIndex >= inodeCount)
    {
        printf("Inode index %d exceeds \'%d\' inode count.\n", inodeIndex, inodeCount);
        return;
    }

    if(!existingInode(inodeIndex))
    {
        printf("Inode at index %d is not currently allocated to a file.\n", inodeIndex);
        return;
    }

    //Used to deallocate inode in inodeBitmap.
    char *inodeBitmapBlock = diskRead(1);
    inodeBitmapBlock[inodeIndex / 8] ^= 0b10000000 >> (inodeIndex % 7);
    //Write indirect pointer block onto disk.
    char indirectPtr[BLOCK_SIZE] = {0};
    memcpy(&indirectPtr, inodeBitmapBlock, BLOCK_SIZE);
    diskWrite(1, indirectPtr);
    free(inodeBitmapBlock);


    char *inode = diskRead(2 + inodeIndex);
    
    //Clear all the data pointers (4 direct and indirect)
    for(int i = 1; i <= 4; i++)
    {

        //TODO change counter to clear indirect pointer.

        //Get data block pointer
        unsigned int pointer = (int) (inode[(i * 4)] << 24);
        pointer += (int) (inode[(i * 4) + 1] << 16);
        pointer += (int) (inode[(i * 4) + 2] << 8);
        pointer += (int) inode[(i * 4) + 3];

        //If address has been set, go clear that location 
        if(pointer > 0)
        {



            //Unallocate datablock corresponding to pointer.
            char *dataBitmapBlock = diskRead(2 + getInodeCount);
            dataBitmapBlock[pointer / 8] ^= (0b10000000 >> (pointer % 7)) ^ 0b01111111;
            diskWrite(2 + getInodeCount, inodeBitmapBlock);
            free(dataBitmapBlock);
        }
    }

    printf("File with inode %d has been deleted.\n", inodeIndex);
}

/**
 * Allocates a free inode to a file.
 */
void makeFile(char ***parsedCommandPtr)
{
    //Get inodeBitmapBlock and check if spot is open
    char *inodeBitampBlock = diskRead(1);
    int inodeIndex = bitmapSearch(&inodeBitampBlock);

    unsigned int inodeCount = getInodeCount();

    //If no inode is availble, notify user.
    if(inodeIndex == -1 || inodeIndex >= inodeCount)
    {
        printf("No more inodes available. A file must be deleted before another is added.\n");
    }
    //Unallocated inode index is found.
    else
    {
        char data[BLOCK_SIZE];
        memcpy(&data, inodeBitampBlock, BLOCK_SIZE);
        diskWrite(1, data);
        printf("Inode at index %d has been created.\n", inodeIndex);
    }

    free(inodeBitampBlock);
}

unsigned int getInodeCount()
{
    //Get inode count from superblock.
    char *metaData = diskRead(0);   
    unsigned int inodeCount = (int) (metaData[8] << 24);
    inodeCount += (int) (metaData[9] << 16);
    inodeCount += (int) (metaData[10] << 8);
    inodeCount += (int) metaData[11];
    free(metaData);

    return inodeCount;
}

/**
 * Returns a value determining if 
 */
bool existingInode(unsigned int inodeIndex)
{
    //This code block checks if an inode is currently alloacted ot not 
    char *inodeBitmapBlock = diskRead(1);
    char inodeByte = inodeBitmapBlock[inodeIndex / 8];
    free(inodeBitmapBlock);
    //If node is currently unallocated, give error message to user.
    if((inodeByte & (0b10000000 >> inodeIndex % 7)) == 0)
    {
        return false;
    }

    return true;
}

int bitmapSearch(char **bitmapBlock)
{
    char *inodeBitmapBlock = *bitmapBlock;

    bool availableSpot = false;

    int index;
    int j;
    char bitMask;

    char finalIndex = -1;

    //Search through bitmap until there is an open position
    for(index = 0; index < BLOCK_SIZE; index++)
    {
        char bits = inodeBitmapBlock[index];
        bitMask = 0b10000000;

        //Go through each bit of the char
        for(j = 7; j >= 0; j--)
        {
            //Use "and" operation on bitmap with bitmask.
            //If value is equal to 0, that position is empty and we can use that inode.
            if((bitMask & bits) == 0)
            {
                //Set bit value using bitmask so inode is marked as used.
                inodeBitmapBlock[index] ^= bitMask;
                availableSpot = true;
                break;
            }

            bitMask = bitMask >> 1;
        }

        if(availableSpot)
        {
            finalIndex = ((index * 8) + (7-j));
            break;
        }
    }

    return finalIndex;
}



/**
 * Formats disk so a file system
 */
void formatDisk()
{
    char metaData[128] = {0};

    //Split Magic Number (int) into four bytes.
    metaData[0] = (MAGIC_NUMBER >> 24) & 0xFF;
    metaData[1] = (MAGIC_NUMBER >> 16) & 0xFF;
    metaData[2] = (MAGIC_NUMBER >> 8) & 0xFF;
    metaData[3] = MAGIC_NUMBER & 0xFF;
 
    //Splits the block count (int) into four bytes.
    metaData[4] = (diskBlocks >> 24) & 0xFF;
    metaData[5] = (diskBlocks >> 16) & 0xFF;
    metaData[6] = (diskBlocks >> 8) & 0xFF;
    metaData[7] = diskBlocks & 0xFF;

    unsigned int inodeCount;

    //If calculated inode count is larger than can fit on bitmap, set it to max value (BLOCK_SIZE * 8).
    if(((unsigned int)(0.10 * diskBlocks)) > BLOCK_SIZE * 8)
    {
        inodeCount = (BLOCK_SIZE * 8);
    }
    else
    {
        inodeCount = (unsigned int)(0.10 * diskBlocks);
    }

    //Splits the inode count (int) into four bytes.
    metaData[8] = (inodeCount >> 24) & 0xFF;
    metaData[9] = (inodeCount >> 16) & 0xFF;
    metaData[10] = (inodeCount >> 8) & 0xFF;
    metaData[11] = inodeCount & 0xFF;

    printf("%d, %c\n", metaData[0], metaData[0]);
    printf("%d, %c\n", metaData[1], metaData[1]);
    printf("%d, %c\n", metaData[2], metaData[2]);
    printf("%d, %c\n", metaData[3], metaData[3]);
    printf("%d, %c\n", metaData[4], metaData[4]);
    printf("%d, %c\n", metaData[5], metaData[5]);
    printf("%d, %c\n", metaData[6], metaData[6]);
    printf("%d, %c\n", metaData[7], metaData[7]);
    printf("%d, %c\n", metaData[8], metaData[8]);
    printf("%d, %c\n", metaData[9], metaData[9]);
    printf("%d, %c\n", metaData[10], metaData[10]);
    printf("%d, %c\n", metaData[11], metaData[11]);


    diskWrite(0, metaData);



    //Calculate how many datablockBitmaps we need.
    int datablockBitampCount = diskBlocks - (2 + inodeCount) / (BLOCK_SIZE * 8);

    //Since we use calloc, everything is initilized to zero so we don't need to worry about setting those values initially.

    printf("Disk has been initialized with file system (Inodes: %d).\n", inodeCount);
}

void createDirectory(char ***parsedCommandPtr)
{
 //TODO allow multiple string arguements if I get time.

    
}



int main(void)
{

    char *command;
    char **parsedCommand;

    //Runs until user decides to exit.
    while (active)
    {
        errorFound = false;

        //User prompt.
        printf(">> ");

        command = getUserInput();

        //If input is found, parse/execute the command.
        if (*command != '\0')
        {
            parsedCommand = tokenize(&command);

            if (!errorFound)
            {
                parseCommand(&parsedCommand);
            }
            freeMemory(&parsedCommand);
        }
        //If no input if found (in this case a single \n), just free memory.
        else
        {
            free(command);
        }
    }

    //Free the emulated disk
    if(disk2 != NULL){
        free(disk2);
    }

}
