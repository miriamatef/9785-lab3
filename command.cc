
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


void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    //currentCommand.execute();

    for (int i = 0; i < tokens.size(); i++)
    {
        const Token &token = tokens[i];
        
        if (token.type == TOKEN_EOF)
        {
            break;
        }

        switch (token.type)
        {
        case TOKEN_COMMAND:
            currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
            break;

        case TOKEN_ARGUMENT:
            currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
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
            if (currentCommand._numberOfAvailableSimpleCommands>0)
            {
                std::cerr << "Error: input redirection can only be specified for the first command\n";
                delete currentSimpleCommand;
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
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

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
        delete(_simpleCommands[i]);//CHECKKKKK THIIISSSSSSS
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

void Command::execute()
{
    print();

    int defaultInput = dup(STDIN_FILENO);
    int defaultOutput = dup(STDOUT_FILENO);
    int defaultError = dup(STDERR_FILENO);

    if (defaultInput <0 || defaultOutput <0 || defaultError <0)
    {
        perror("Error: dup failed");
        clear();
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        std::cerr << "Error: fork failed\n";
        clear();
        prompt();
        return;
    }
    else if (pid == 0) // Child process
    {
        execvp(_simpleCommands[0]->_arguments[0], _simpleCommands[0]->_arguments);
        perror("Error: execvp failed");
        exit(EXIT_FAILURE);
    }
    else // Parent process
    {
        if (!_background)
        {
            waitpid(pid, NULL, 0);
        }
    }

    dup2(defaultInput, STDIN_FILENO);
    dup2(defaultOutput, STDOUT_FILENO);
    dup2(defaultError, STDERR_FILENO);

    close(defaultInput);
    close(defaultOutput);
    close(defaultError);

    clear();
    prompt();
}

void Command::prompt() //modification: stop reading when EOF is reached
{
    std::string input;

    while (true)
    {
        printf("myshell>");
        fflush(stdout);
        std::getline(std::cin, input); 
        if (std::cin.eof()) {
            printf("\n");
            break;
        }
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt();
    return 0;
}
