
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"
#include <errno.h>

void WildCards(SimpleCommand *simpleCommand, const std::string &arg)
{
    glob_t matching= {};
    int flags = glob (arg.c_str(), GLOB_NOCHECK, nullptr, &matching);
    if (flags == 0)
    {
        for (size_t i = 0; i < matching.gl_pathc; ++i)
        {
            simpleCommand->insertArgument(strdup(matching.gl_pathv[i]));
        }
    }
    else //if expansion fails, save the original argument
    {
        simpleCommand->insertArgument(strdup(arg.c_str()));
    }
    globfree(&matching);
}

//============================================================================================

void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    //currentCommand.execute();

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const Token &token = tokens[i];
        
        if (token.type == TOKEN_EOF)
        {
            break;
        }

        switch (token.type)
        {
        case TOKEN_COMMAND:
            WildCards(currentSimpleCommand, token.value);
            //currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
            break;

        case TOKEN_ARGUMENT:
            WildCards (currentSimpleCommand, token.value);
            //currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
            break;

        case TOKEN_REDIRECT:
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF ||
                tokens[i + 1].type == TOKEN_PIPE ||
                tokens[i + 1].type == TOKEN_BACKGROUND ||
                tokens[i + 1].type == TOKEN_REDIRECT ||
                tokens[i + 1].type == TOKEN_APPEND ||
                tokens[i + 1].type == TOKEN_INPUT ||
                tokens[i + 1].type == TOKEN_ERROR ||
                tokens[i + 1].type == TOKEN_REDIRECT_AND_ERROR)
            {
                std::cerr << "Error: missing redirection filename\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentCommand._outFile != nullptr)
            {
                std::cerr << "Error: multiple output redirections\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            currentCommand._outFile = strdup(tokens[++i].value.c_str());
            currentCommand._append = 0; 
            break;

        case TOKEN_APPEND:
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF ||
                tokens[i + 1].type == TOKEN_PIPE ||
                tokens[i + 1].type == TOKEN_BACKGROUND ||
                tokens[i + 1].type == TOKEN_REDIRECT ||
                tokens[i + 1].type == TOKEN_APPEND ||
                tokens[i + 1].type == TOKEN_INPUT ||
                tokens[i + 1].type == TOKEN_ERROR ||
                tokens[i + 1].type == TOKEN_REDIRECT_AND_ERROR)
            {
                std::cerr << "Error: missing redirection filename\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentCommand._outFile != nullptr)
            {
                std::cerr << "Error: multiple output redirections\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            currentCommand._append = 1;
            currentCommand._outFile = strdup(tokens[++i].value.c_str());
            break;

        case TOKEN_INPUT:
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF ||
                tokens[i + 1].type == TOKEN_PIPE ||
                tokens[i + 1].type == TOKEN_BACKGROUND ||
                tokens[i + 1].type == TOKEN_REDIRECT ||
                tokens[i + 1].type == TOKEN_APPEND ||
                tokens[i + 1].type == TOKEN_INPUT ||
                tokens[i + 1].type == TOKEN_ERROR ||
                tokens[i + 1].type == TOKEN_REDIRECT_AND_ERROR)
            {
                std::cerr << "Error: missing redirection filename\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentCommand._inputFile != nullptr)
            {
                std::cerr << "Error: multiple input redirections\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentCommand._numberOfSimpleCommands>0)
            {
                std::cerr << "Error: input redirection can only be specified for the first command\n";
                delete currentSimpleCommand;
                free(currentCommand._inputFile);
                currentCommand.clear();
                return;
            }
            currentCommand._inputFile = strdup(tokens[++i].value.c_str());
            break;

        case TOKEN_ERROR:
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF ||
                tokens[i + 1].type == TOKEN_PIPE ||
                tokens[i + 1].type == TOKEN_BACKGROUND ||
                tokens[i + 1].type == TOKEN_REDIRECT ||
                tokens[i + 1].type == TOKEN_APPEND ||
                tokens[i + 1].type == TOKEN_INPUT ||
                tokens[i + 1].type == TOKEN_ERROR ||
                tokens[i + 1].type == TOKEN_REDIRECT_AND_ERROR)
            {
                std::cerr << "Error: missing redirection filename\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentCommand._errFile != nullptr)
            {
                std::cerr << "Error: multiple error redirections\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            currentCommand._errFile = strdup(tokens[++i].value.c_str());
            break;

        case TOKEN_REDIRECT_AND_ERROR:
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF ||
                tokens[i + 1].type == TOKEN_PIPE ||
                tokens[i + 1].type == TOKEN_BACKGROUND ||
                tokens[i + 1].type == TOKEN_REDIRECT ||
                tokens[i + 1].type == TOKEN_APPEND ||
                tokens[i + 1].type == TOKEN_INPUT ||
                tokens[i + 1].type == TOKEN_ERROR ||
                tokens[i + 1].type == TOKEN_REDIRECT_AND_ERROR)
            {
                std::cerr << "Error: missing redirection filename\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentCommand._out_error == 1)
            {
                std::cerr << "Error: multiple error redirections\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            currentCommand._out_error = 1;
            currentCommand._append =1;
            currentCommand._errFile = strdup(tokens[++i].value.c_str());
            currentCommand._outFile = strdup(tokens[i].value.c_str());
            break;

        case TOKEN_PIPE:
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF ||
                tokens[i + 1].type == TOKEN_PIPE ||
                tokens[i + 1].type == TOKEN_BACKGROUND ||
                tokens[i + 1].type == TOKEN_REDIRECT ||
                tokens[i + 1].type == TOKEN_APPEND ||
                tokens[i + 1].type == TOKEN_INPUT ||
                tokens[i + 1].type == TOKEN_ERROR ||
                tokens[i + 1].type == TOKEN_REDIRECT_AND_ERROR)
            {
                std::cerr << "Error: missing command after pipe\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentSimpleCommand->_numberOfArguments == 0)
            {
                std::cerr << "Error: missing command before pipe\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentCommand._outFile != nullptr)
            {
                std::cerr << "Error: output redirection is only allowed on the last command\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            currentCommand.insertSimpleCommand(currentSimpleCommand);
            currentSimpleCommand = new SimpleCommand();
            break;

        case TOKEN_BACKGROUND:
            if (i + 1 < tokens.size() && tokens[i + 1].type != TOKEN_EOF)
            {
                std::cerr << "Error: background operator '&' must be at the end of the command\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            if (currentSimpleCommand->_numberOfArguments == 0 && currentCommand._numberOfSimpleCommands == 0)
            {
                std::cerr << "Error: missing command before background operator '&'\n";
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }
            currentCommand._background = 1;
            break;
        
        default:
            std::cerr << "Error: unexpected token type\n";
            delete currentSimpleCommand;
            currentCommand.clear();
            return;
        }
    }

    if (currentSimpleCommand->_numberOfArguments > 0)
    {
        currentCommand.insertSimpleCommand(currentSimpleCommand);
        currentSimpleCommand = nullptr; // Prevent deletion of the current simple command since it's now part of the command
    }
    else
    {
        delete currentSimpleCommand;
        currentSimpleCommand = nullptr; // Prevent double deletion
    }

    if (currentCommand._numberOfSimpleCommands == 0)
    {
        //std::cerr << "Error: no command to execute\n" //shell stays silent if no command is provided
        currentCommand.clear();
        return;
    }
    
    currentCommand.execute();
    
}

//=================================================================================================================================================

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,_numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

//================================================================================================================

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;
    _out_error = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;
    _out_error = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

//===========================================================================================================================================================================

void Command::execute()
{
    print();

    if (_numberOfSimpleCommands == 0) //checking for empty command
    {
        clear();
        return;
    }

    char **simpleArgs = _simpleCommands[0]->_arguments; //get arguments fo the simple command
    if (simpleArgs ==nullptr || simpleArgs[0] == nullptr)
    {
        clear();
        return;
    }

    //HANDLING THE EXIT:
    if (_numberOfSimpleCommands ==1 && strcmp(simpleArgs[0],"exit")==0)
    {
        printf("Good bye!!\n");
        fflush(stdout);
        clear();
        exit(0);
    }

    //cd IMPLEMENTATION:
    if (_numberOfSimpleCommands ==1 && strcmp(simpleArgs[0],"cd")==0 && !_background) 
    {
        char *directory = simpleArgs[1];
        if (directory == nullptr) 
        {
            directory = getenv("HOME");
        }
        if (directory == nullptr || chdir(directory) ==-1)
        {
            perror("Error: cd failed");
        }
        clear();
        return;
    }

    //CTRL+C:
    struct sigaction ctrlc;
    memset(&ctrlc, 0, sizeof(ctrlc));
    ctrlc.sa_handler = SIG_DFL;
    //fel main() hakhaly el shell y-ignore SIGINT
    sigemptyset(&ctrlc.sa_mask);

    //file default descriptors: 
    int defaultInput = dup(0);
    int defaultOutput = dup(1);
    int defaultError = dup(2);

    if (defaultInput <0 || defaultOutput <0 || defaultError <0)
    {
        perror("Error: dup failed");
        if (defaultInput >= 0) 
            close(defaultInput);

        if (defaultOutput >= 0) 
            close(defaultOutput);

        if (defaultError >= 0) 
            close(defaultError);
        
        clear();
        return;
    }

    //child and parent
    int prev=-1;
    std::vector<pid_t> childPids;

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        if (_simpleCommands[i] == nullptr || _simpleCommands[i]->_arguments == nullptr || _simpleCommands[i]->_arguments[0] == nullptr || _simpleCommands[i]->_numberOfArguments == 0)
        {
            std::cerr << "Error: invalid command\n";
            break;
        }

        //PIPE
        int pipe_file[2]={-1,-1}; //[0]=read, [1]=write
        if (i < _numberOfSimpleCommands - 1)
        {
            if (pipe(pipe_file) < 0)
            {
                perror("Error: pipe failed");
                break;
            }
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error: fork failed");
            if (pipe_file[0] != -1)
            {
                close(pipe_file[0]);
            }
            if (pipe_file[1] != -1)
            {
                close(pipe_file[1]);
            }
            break;
        }

        else if (pid == 0) // Child process
        {
            sigaction(SIGINT, &ctrlc, nullptr);
            signal (SIGCHLD, SIG_DFL);
            
            //Connect input to the previous command's pipe
            if (prev != -1)
            {
                if(dup2(prev, 0) < 0) //hayethat fel stdin (0)
                {
                    perror("Error: dup2 failed");
                    _exit(EXIT_FAILURE);
                }
            }

            //Connect output to the next command's pipe
            if (pipe_file[1] != -1)
            {
                if(dup2(pipe_file[1], 1) < 0) //hayethat fel stdout (1)
                {
                    perror("Error: dup2 failed");
                    _exit(EXIT_FAILURE);
                }
            }

            if (prev != -1)
            {
                close(prev);
            }

            if (pipe_file[0] != -1)
            {
                close(pipe_file[0]);
            }

            if (pipe_file[1] != -1)
            {
                close(pipe_file[1]);
            }

            //Input redirection:
            if (i==0 && _inputFile != nullptr)
            {
                int inputFileDir = open(_inputFile, O_RDONLY);
                if (inputFileDir < 0)
                {
                    perror("Error: failed to open input file");
                    _exit(EXIT_FAILURE);
                }
                if (dup2(inputFileDir, 0) < 0) //put the input file instead of stdin (0)
                {
                    perror("Error: dup2 failed for input redirection");
                    close(inputFileDir);
                    _exit(EXIT_FAILURE);
                }
                close(inputFileDir);
            }

            //Output and append redirections:
            if (i==_numberOfSimpleCommands -1 && _outFile != nullptr)
            {
                int flags = O_WRONLY | O_CREAT;
                if (_append)
                {
                    flags |= O_APPEND; //adds to the end of the file
                }
                else
                {
                    flags |= O_TRUNC; //overwrites 
                }

                int outputFd = open(_outFile, flags, 0644);
                if (outputFd < 0)
                {
                    perror("Error: failed to open output file");
                    _exit(EXIT_FAILURE);
                }
                if (dup2(outputFd, 1) < 0)
                {
                    perror("Error: dup2 failed for output redirection");
                    close(outputFd);
                    _exit(EXIT_FAILURE);
                }
                close(outputFd);
            }

            //duplicating teh std o/p to the std error
            if (i == _numberOfSimpleCommands - 1 && _out_error)
            {
                if (dup2(1, 2) < 0)
                {
                    perror("dup2 combined output/error");
                    _exit(1);
                }
            }

            //Error redirection: 
            if (i== _numberOfSimpleCommands -1 && _errFile != nullptr && !_out_error)
            {
                int errorFd = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (errorFd < 0)
                {
                    perror("Error: failed to open error file");
                    _exit(EXIT_FAILURE);
                }
                if (dup2(errorFd, 2) < 0)
                {
                    perror("Error: dup2 failed for error redirection");
                    close(errorFd);
                    _exit(EXIT_FAILURE);
                }
                close(errorFd);
            }   
            close(defaultInput);
            close(defaultOutput);
            close(defaultError);

            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror("Error: execvp failed");
            _exit(EXIT_FAILURE);
        }

        else // Parent process
        {
            childPids.push_back(pid);

            //since parent doesn't need the pipe descriptors, close them
            if (prev != -1)
            {
                close(prev);
            }
            if (pipe_file[1] != -1)
            {
                close(pipe_file[1]);
            }

            //Keep the read end of the pipe for the next command
            prev = pipe_file[0];
        }
    }

    //Close the final pipe read
    if (prev != -1)
    {
        close(prev);
    }

    //Restore shell's original descriptors
    if (dup2(defaultInput, 0)<0)
    {
        perror("Error: dup2 failed");
    }
    if (dup2(defaultOutput, 1)<0)
    {
        perror("Error: dup2 failed");
    }
    if (dup2(defaultError, 2)<0)
    {
        perror("Error: dup2 failed");
    }

    close(defaultInput);
    close(defaultOutput);
    close(defaultError);

    //wait for foreground processes to finish
    if (!_background)
    {
        for (pid_t childPid : childPids)
        {
            while (waitpid(childPid, NULL, 0) < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }

                if (errno == ECHILD)
                {
                    break;
                }
                perror("Error: waitpid failed");
                break;
            }
        }
    }

    clear();
}

void Command::prompt() //modification: stop reading when EOF is reached
{
    std::string input;

    while (true)
    {
        printf("myshell>");
        fflush(stdout);
        std::getline(std::cin, input);  //Reads an entire line from standard input into input.
        if (std::cin.eof()) {
            printf("\n");
            break;
        }
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

void childTerminated(int signalNumber)
{
    (void)signalNumber;

    int savedErrno = errno;
    int status;
    pid_t pid;

    // Reap all terminated child processes.
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        int logFd = open("myshell.log", O_WRONLY | O_CREAT | O_APPEND, 0644);

        if (logFd >= 0)
        {
            char message[100];

            int length = snprintf(message,sizeof(message),"Child process %ld terminated\n",(long)pid);

            if (length > 0 && length < (int)sizeof(message))
            {
                write(logFd, message, length);
            }

            close(logFd);
        }
    }

    errno = savedErrno;
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    signal (SIGINT, SIG_IGN); // Ignore SIGINT in the shell

    //Preparing the SIGCHILD handler:
    struct sigaction childAction;
    childAction.sa_handler = childTerminated;
    sigemptyset(&childAction.sa_mask);
    childAction.sa_flags = SA_RESTART | SA_NOCLDSTOP;
//SA_RESTART: requests auto restarting
//SA_NOCLDSTOP:prevents the handler from being invoked because the child stopped


    if (sigaction(SIGCHLD, &childAction, NULL) == -1)
    {
        perror("sigaction SIGCHLD");
        return 1;
    }

    //STARTTTTT:
    Command::_currentCommand.prompt();
    return 0;
}
