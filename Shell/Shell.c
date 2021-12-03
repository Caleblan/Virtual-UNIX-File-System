#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "Shell.h"

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

int makeFile()
{

}

int writeFile(char ***parsedCommandPtr)
{
    char **parsedCommand = *parsedCommandPtr;

    int counter = 1;

    if(parsedCommand[counter] == '\0')
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

int deleteFile(char ***parsedCommandPtr)
{

}

int makeFile(char ***parsedCommandPtr)
{

}

int createDirectory(char ***parsedCommandPtr)
{

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

}
