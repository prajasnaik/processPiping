# Process Piping

## How to use the program

#### Compiling and Execution:

```Bash
        make
        ./my_ipc -g <GeneratorExecutable> -c <ConsumerExecutable> -a <GeneratorOptionsString> -o <ConsumerOptionsString>
``` 
Here the -g flag is used to specify the generator executable path and the -c flag is used to specify the consumer executable path. The -a and -o flags are used to provide the options for the generator and the consumer respectively.

## Design Decisions

This process will only execute if the generator and consumer process paths are provided. We have chosen to make the options for both the processes as optional.

If the number of arguments passed to the process is insufficient, it will terminate with error code -1. This was also a design decision to allow the process to indicate that the issue was with the arguments and not any other specific error. Similarly, any other error related to arguments will return -1

In the program, we have chosen to print exit code of child only if the value is not 0, otherwise no error code is printed. This is to remove redundancy. Apart from this, we have also chosen to detect signals and core dumps. In case these occur, we will return the codes associated with them. Also, to ensure smooth execution, we have chosen to only detect the generator process and its return value, the consumer process is ignored. The consumer runs in the background while the generator in the foreground. 

In the program if no arguments are given, the help message will be displayed with information of how to use the program. This decision was made to meet assignment requirements while retaining the default help functionality.