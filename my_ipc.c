
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
int         fGenerator              =   DISABLE;
int         fConsumer               =   DISABLE;
int         fGeneratorOptions       =   DISABLE;
int         fConsumerOptions        =   DISABLE;


// Buffer pointers for storing paths for each function
char *      generatorExecutablePath;
char *      consumerExecutablePath;

// Buffers for storing options
char *      generatorOptions;
char *      consumerOptions;


int         PipeExecutables     (char *, char *);
int         ProcessCommandLine  (char **, int);
int         PerformOperations   ();
int         Help                ();
char **     VectorizeString     (char *, char *);

// Main function
int 
main(int argc, char *argv[])
{
    if (argc == 1)
    {
        ec = Help(); //Help is called if no options are given
        if (ec != E_OK)
            return ec;
    }
    else
    {        
        ec = ProcessCommandLine(argv, argc);
        if (ec != E_OK)
            return ec;
        
        ec = PerformOperations();
        if (ec != E_OK)
            return ec;
    }
    return ec;       
}


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
            if(argno + 1 == argCount) //Ensures that path is provided
            {
                return E_GENERAL;
            }
            fGenerator = ENABLE;
            generatorExecutablePath = commandLineArguments[argno + 1];
            argno += 2;
            break;
        case 'a':
            if (argno + 1 == argCount) //Ensures that path is provided
                return E_GENERAL;
            fGeneratorOptions = ENABLE;
            generatorOptions = commandLineArguments[argno + 1];
            argno += 2;
            break;
        case 'o':
            if (argno + 1 == argCount) //Ensures that path is provided
                return E_GENERAL;
            fConsumerOptions = ENABLE;
            consumerOptions = commandLineArguments[argno + 1];
            argno += 2;
            break;
        case 'c':
            if(argno + 1 == argCount) //Ensures that path is provided
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
    if(!(fGenerator && fConsumer)) //If Consumer or Generator path not specified, return
    {
        char *buf = "\nNo generator or consumer executable path specified, please use -g/-c flag to provide path.\n";
        ec = write(STDOUT_FILENO, buf, strlen(buf));
        if (ec == E_GENERAL)
        {
            return errno;
        }
        return E_GENERAL;
    }
    return E_OK; 
}


//  function: PerformOperations
//      This function calls and executes appropriate functions based on
//      which flags were set after processing command line.
//  @param: None
//  @return: Integer Error Code
int
PerformOperations()
{
    if(fConsumerOptions)
    {
        if (fGeneratorOptions)
        {
            ec = PipeExecutables(consumerOptions, generatorOptions);
        }
        else
        {
            ec = PipeExecutables(consumerOptions, "none");
        }
    }
    else
    {
        if(fGeneratorOptions)
        {
            ec = PipeExecutables("none", generatorOptions);
        }
           
        else
        {
            ec = PipeExecutables("none", "none");
        }
    }
    return ec;
}


//  function: PipeExecutables
//      This function executes a given generator binary specified by a generator path and pipes
//      its standard output to the standard input of consumer binary specified by consumer path
//
//  @param: pointer to a character array containing options string for consumer executable
//  @param: pointer to a character array containing options string for generator executable
//  @return: integer error code
int
PipeExecutables(char *optionsConsumer, char *optionsGenerator)
{
    char **generatorArguments = VectorizeString(optionsGenerator, generatorExecutablePath);
    char **consumerArguments = VectorizeString(optionsConsumer, consumerExecutablePath);
    int fd[2];
    ec = pipe(fd);
    if (ec == E_GENERAL)
        return errno;

    int pidc = fork();
    if (pidc == E_GENERAL)
        return errno; 
    else if (pidc == 0)
    {
        ec = dup2(fd[0], STDIN_FILENO);
        if (ec == E_GENERAL)
            return errno;

        ec = close(fd[1]);
        if (ec == E_GENERAL)
            return errno;
        
        ec = execv(consumerExecutablePath, consumerArguments);
        return errno; //only returns if error occurred in execv() system call
    }
    else
    {
        int pidg = fork();
        if (pidg == -1)
            return errno;
        else if (pidg == 0)
        {
            ec = dup2(fd[1], STDOUT_FILENO);
            if (ec == E_GENERAL)
                return errno;
            
            ec = close(fd[0]);
            if (ec == E_GENERAL)
                return errno;

            ec = execv(generatorExecutablePath, generatorArguments);
            return errno;
        }
        else
        {
            int retVal = 0;
            char buffer[MAX_SIZE];
            int signalCode;
            int wstatusg;
            int w = waitpid(pidg, &wstatusg, WUNTRACED);
            if (w == E_GENERAL)
                return errno;

            if (&wstatusg != NULL)
            {
                if (WIFEXITED(wstatusg))
                {
                    retVal = WEXITSTATUS(wstatusg); //finds exit status of child if not 0
                    if (retVal == E_OK)
                        return E_OK;
                    snprintf(buffer, MAX_SIZE, "%s: %d %s - %d", "The process with pid", pidg, "exited with error code", retVal);
                    write(STDOUT_FILENO, buffer, strlen(buffer));
                    return E_OK; //Design choice to print child exit status and return parent exit code
                }
                else if (WIFSIGNALED(wstatusg))
                {
                    signalCode = WTERMSIG(wstatusg);
                    snprintf(buffer, MAX_SIZE,  "%s: %d %s - %d", "The process with pid", pidg, "signalled with signal code", signalCode);
                    write(STDOUT_FILENO, buffer, strlen(buffer));
                    return E_OK;
                }
                else if (__WCOREDUMP(wstatusg))
                {
                    snprintf(buffer, MAX_SIZE, "%s: %d %s", "The process with pid", pidg, "was core dumped");
                    return E_OK;
                }
            }
            return E_OK;
        }
    }
}



//  function: VectorizeString
//      This function takes a string of character options as well as executable path and
//      constructs a null terminated array of strings that can be passed to the execv() 
//      system call. The format of the array is similar to the command line options received
//      by processes as command line arguments. 
//  @param: pointer to a character array containing options string
//  @param: pointer to a character array containing an executable path
//  @return: pointer to the vectorized array of strings
char ** 
VectorizeString(char *optionsString, char *path)
{
    if (optionsString == "none")
    {
        char **options = (char **) calloc(2, sizeof(char *)); //gets a pointer to an array of pointers
        options[0] = path;
        options[1] = NULL;
        return options;
    }
    int i = 0;
    int elementCount = 1; //Set to one because an array without spaces still has one element
    int length = strlen(optionsString + 1); // 
    for (i = 0; i < length; i ++)
    {
        if(optionsString[i] == ' ')
            elementCount++; //checks for number of spaces in the array
    }

    char **options = (char **) calloc(elementCount + 2, sizeof(char *)); // Is + 2 because of the addition of the path at the start and the null character at the end
    options[0] = path;
    char * token = strtok(optionsString, " "); //splits the string on spaces and appends to argument array
    i = 1;
    while( token != NULL ) {
        options[i++] = token; 
        token = strtok(NULL, " "); 
    }
    options[i] = NULL; //terminates array with null character
    return options;
}

// function: Help
//      Provides a basic message on how to use the program.
//  @param: None
//  @return: Integer Error Code
int 
Help()
{
    char *helpMessage = "\n\tUsage:\n\t./my_ipc -g <GeneratorExecutablePath> -c <GeneratorExecutablePath> -a <GeneratorOptionsString> -o <ConsumerOptionsString>\n\tFor more info, please refer to README file\n";
    int length = strlen(helpMessage);
    int error = write(STDOUT_FILENO, helpMessage, length);
    if (error == E_GENERAL)
        return errno;
    return E_OK;
}
