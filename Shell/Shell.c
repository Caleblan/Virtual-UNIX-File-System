#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
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
        printf("Too few arguements. Command should follow form \'delete_file [unsigned int arguement] [String]\'.\n");
        return;
    }
    else if(parsedCommand[3] != NULL)
    {
        printf("Too many arguements. Command should follow form \'delete_file [unsigned int arguement]\'.\n");
        return;
    }

    //TODO dataBLock pointers for inode

    int inodeIndex = atoi(parsedCommand[1]);


    //Used only if the string is larger than the BLOCK_SIZE
    char *newString = parsedCommand[2];

    int counter = 0;

    
    //Check if inode is currently allocated or not.
    char *inodeBitmapBlock = diskRead(1);

    char inodeByte = inodeBitmapBlock[inodeIndex / 8];
    
    if(inodeByte & (0b10000000 >> inodeIndex % 7) == 0)
    {
        printf("Inode at index %d is not currently allocated as a file.\n", inodeIndex);
        free(inodeBitmapBlock);
        return;
    }

    free(inodeBitmapBlock);


    do
    {
        //If we run out of direct pointers, create and use an indirect pointer.
        if(counter < 4)
        {
            // char data

            //Get one blocks worth of data for direct pointers.
            for(int i = 0; i < BLOCK_SIZE; i++)
            {
                if(*newString != '\0')
                {
                    break;
                }

                newString++;
            }

            // disk
            
        }
        //If there is not enough pointers to hold data blocks, cut off the copy 
        else if(counter > + BLOCK_SIZE * counter)
        {
            printf("File exceeds maximum size so file has been clipped");
            return;
        }
        else
        {

        }

        counter++;
        
    } while (strlen(newString) > BLOCK_SIZE);
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



void deleteFile(char ***parsedCommandPtr)
{
    if(disk2 != NULL)
    {
        printf("Disk partition has already been created (DiskSize (in bytes): %d, DiskBlocks: %d).\n", diskSize, diskBlocks);
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

    //Finds inode in bitmap and sets it's value to 0.
    char* inodeBitmapBlock = diskRead(1);
    unsigned int charIndex = inodeBitmapBlock[inodeIndex / 8];
    char setBitMask = 0b01111111 >> (inodeIndex % 8);

    //Checks if the inode is currently allocated or not.
    char checkBitMask = 0b10000000;
    for(int i = 0; i < 7; i++)
    {
        //If the inode bitmap already says the node is free, return error message to the user.
        if((checkBitMask & charIndex) == 0)
        {
            printf("Inode at index \'%d\' has either not been created or has been deallocted already.\n", inodeIndex);
            //free(inodeBitmapBlock);
            return;
        }

        checkBitMask = checkBitMask >> 1;
    }

    //Set bit value using bitmask so inode is marked as used.
    inodeBitmapBlock[charIndex] ^= setBitMask;
    diskWrite(inodeIndex, &inodeBitmapBlock);
    //free(inodeBitmapBlock);

    //Set datablocks in datablock to null.
    //TODO use if so that if value is greater than, don't go through this.
    char* inodeBlock = diskRead(2 + inodeIndex);

    char* dataBlockGroup = diskRead(4);

 
    //TODO allow multiple string arguements if I get time.
    //diskWrite();
}

/**
 * Allocates a free block of data 
 */
void makeFile(char ***parsedCommandPtr)
{
    bool availableInode = false;

    char *inodeBitampBlock = diskRead(1);

    int index;
    int j;
    char bitMask;

    //Search through bitmap until there is an open position
    for(index = 0; index < BLOCK_SIZE; index++)
    {
        char bits = inodeBitampBlock[index];
        bitMask = 0b10000000;

        //Go through each bit of the char
        for(j = 7; j >= 0; j--)
        {
            printf("%d\n", (index * 8) + (7-j));

            //Use "and" operation on bitmap with bitmask.
            //If value is equal to 0, that position is empty and we can use that inode.
            if((bitMask & bits) == 0)
            {
                //Set bit value using bitmask so inode is marked as used.
                inodeBitampBlock[index] ^= bitMask;
                availableInode = true;
                break;
            }

            bitMask = bitMask >> 1;
        }

        if(availableInode)
        {
            break;
        }
    }

    unsigned int inodeIndex = (index * 8) + (7-j);

    char *metaData = diskRead(0);   
    
    //
    unsigned int inodeCount = (int) (metaData[8] << 24);
    inodeCount += (int) (metaData[9] << 16);
    inodeCount += (int) (metaData[10] << 8);
    inodeCount += (int) metaData[11];

    //If no inode is availble, notify user.
    if(!availableInode || inodeIndex >= inodeCount)
    {
        printf("No more inodes available. A file must be deleted before another is added.\n");
    }
    else
    {
        char data[BLOCK_SIZE];
        memcpy(&data, inodeBitampBlock, BLOCK_SIZE);
        diskWrite(1, data);
        printf("Inode at index %d has been created.\n", inodeIndex);
    }

    free(metaData);
    free(inodeBitampBlock);
}



/**
 * Formats disk so a file system
 */
void formatDisk()
{
    char metaData[128];

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

    unsigned inodeCount = (0.10 * diskBlocks);

    //Splits the inode count (int) into four bytes.
    metaData[8] = (inodeCount >> 24) & 0xFF;
    metaData[9] = (inodeCount >> 16) & 0xFF;
    metaData[10] = (inodeCount >> 8) & 0xFF;
    metaData[11] = inodeCount & 0xFF;



    //Set all remaining values to zero
    for(int i = 12; i < BLOCK_SIZE; i++)
    {
        metaData[i] = 0;
    }

    diskWrite(0, metaData);

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
