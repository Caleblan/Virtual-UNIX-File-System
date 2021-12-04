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

    unsigned int blockAddress;

    //Checks if data is greater than the block size.
    if(strlen(parsedCommand[1]) > BLOCK_SIZE)
    {
        printf("Inputted data exceeds disk block size (BlockSize: %d).\n", BLOCK_SIZE);
        return;
    }
    //If value

    //TODO check for greater than input.
    sscanf(parsedCommand[2], "%u", &blockAddress);

    diskWrite(blockAddress, parsedCommand[2]);

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

    sscanf(parsedCommand[2], "%u", &blockAddress);
    char *block = diskRead(blockAddress);

    if(*block != NULL)
    {
        printf("'%s' read from disk.\n", block);
    }

    free(block);
}


void writeFile(char ***parsedCommandPtr)
{
    if(disk2 == NULL)
    {
        printf("No disk partition has been created. Try creating one with \'create_partition [unsigned int arguement]\'.\n");
        return;
    }


    char **parsedCommand = *parsedCommandPtr;

    int counter = 1;

    if(*parsedCommand[counter] == '\0')
    {
        printf("\'write_file\' requires .\n");
    }
    else
    {
        //TODO allow multiple string arguements if I get time.

        //TODO create new child process
        // while(parsedCommand[counter++])
        // {

        // }


    }
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

    //TODO add error checking here.
    diskSize = atoi(parsedCommand[1]);

    diskBlocks = diskSize / BLOCK_SIZE;

    //Used to store an array of structs.
    disk2 = calloc(diskSize, sizeof(char));
}



void deleteFile(char ***parsedCommandPtr)
{
 //TODO allow multiple string arguements if I get time.
}

void makeFile(char ***parsedCommandPtr)
{
    int BITMAP_START = BLOCK_SIZE * 2;
    int BITMAP_END = BLOCK_SIZE * 3;


    //Search through bitmap until there is an open position
    for(int i = BITMAP_START; i < BITMAP_END; i++)
    {
        char bitMask = 0b10000000;
        char bits = disk2[i];

        for(int j = sizeof(char); j >= 0; j--)
        {
            //Use "and" operation on bitmap with bitmask.
            //If value is equal to 0, that position is empty and we can use that inode.
            if((bitMask & bits) == 0)
            {
                //Set bit value using bitmask so inode is marked as used.
                disk2[i] ^= bitMask;
                break;
            }

            bitMask = bitMask >> 1;
        }
    }
}

/**
 * Formats disk so a file system
 */
void formatDisk()
{
    //Split Magic Number (int) into four bytes.
    disk2[0] = (MAGIC_NUMBER >> 24) & 0xFF;
    disk2[1] = (MAGIC_NUMBER >> 16) & 0xFF;
    disk2[2] = (MAGIC_NUMBER >> 8) & 0xFF;
    disk2[3] = MAGIC_NUMBER & 0xFF;
 
    //Splits the block count (int) into four bytes.
    disk2[4] = (diskSize >> 24) & 0xFF;
    disk2[5] = (diskSize >> 16) & 0xFF;
    disk2[6] = (diskSize >> 8) & 0xFF;
    disk2[7] = diskSize & 0xFF;

    //Splits the inode count (int) into four bytes.
    disk2[8] = (diskSize >> 24) & 0xFF;
    disk2[9] = (diskSize >> 16) & 0xFF;
    disk2[10] = (diskSize >> 8) & 0xFF;
    disk2[11] = diskSize & 0xFF;


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
