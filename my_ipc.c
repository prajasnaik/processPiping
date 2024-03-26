
// Include Statements
#include    <unistd.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <errno.h>
#include    <string.h>
#include    <stdlib.h>
#include    <stdio.h>


//Define Statements (Macros)
#define     E_GENERAL   -1
#define     E_OK        0
#define     ENABLE      1
#define     DISABLE     0
#define     MAX_SIZE    1024


// Global Variable for Error Code
int         ec          =   E_OK;


// Global Variable Flags for deciding which operation to perform
int         fGenerator            =   DISABLE;
int         fConsumer         =   DISABLE;


// Buffer pointers for storing paths for each function
char *      generatorExecutablePath;
char *      consumerExecutablePath;

// function: ProcessCommandLine
//      This function takes the command line arguments and appropriately sets 
//      flags for which operations need to be performed. It also extracts 
//      appropiate pointers
//  @param: commandLineArguments - Pointer to array containing command line 
//          arguments
//  @param: argCount - Integer count of total  number of command line arguments
//  @return: Integer error code
int 
ProcessCommandLine(char *commandLineArguments[], int argCount)
{
    int argno = 1;
    while(argno < argCount)
    {
        switch (commandLineArguments[argno][1])
        {
        case 'g':
            if(argno + 1 == argCount)
            {
                return E_GENERAL;
            }
            fGenerator = ENABLE;
            generatorExecutablePath = commandLineArguments[argno + 1];
            argno += 2;
            break;
        case 'c':
            if(argno + 1 == argCount)
            {
                return E_GENERAL;
            }
            fConsumer = ENABLE;
            consumerExecutablePath = commandLineArguments[argno + 1];
            argno += 2;
            break;
        default:
            return E_GENERAL;
            break;
        }
    } 
    if(!fGenerator || fConsumer)
    {
        char *buf = "\nNo generator or consumer executable path specified, please use -g/-c flag to provide path.\n";
        ec = write(STDOUT_FILENO, buf, strlen(buf));
        if (ec == E_GENERAL)
        {
            ec = PrintError(errno);
            return ec;
        }
        return E_GENERAL;
    }
    return E_OK; 
}

