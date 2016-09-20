
/*
 * CS252: Shell project
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
#include <list>
#include <iterator>
#include "command.h"

//Declare extern global variable
extern char**environ;


SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
	
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	//TEST15 & 16: Environment variable expansion
	
//	printf("argument: %s\n",argument);


	char * p = argument;
	if(strstr(p,"${") != NULL){
		char * env_var = (char*)malloc(1024);
		char * new_argument = (char*)malloc(1024);
		new_argument[0] = '\0';
		int index = 0;
		while(*p){
			if(*p == '$' && *(++p) == '{'){
				while(*(++p) != '}'){
					env_var[index++] = *p;
				}
				p++;
				env_var[index] = '\0';
				strcat(new_argument,getenv(env_var));
				index = 0;
			}else{
				env_var[0] = *p;
				env_var[1] = '\0';
				strcat(new_argument,env_var);
				p++;
			}
		}
		argument = strdup(new_argument);
		free(new_argument);
		free(env_var);
		p = argument;
	}

	//Tilde Expansion
	if(*p == '~'){
		if(strlen(p) == 1){
		//	printf("~");
			argument = strdup(getenv("HOME"));
		}else{
		//	printf("begin\n");
		//	char * home = "/homes/";
			char * dir = (char*)malloc(1024);
			strcpy(dir,"/homes/");
			p++;
		//	printf("%s\n",dir);
			
			strcat(dir,strdup(p));
		//	printf("dir: %s\n",dir);
			argument = strdup(dir);
			free(dir);
		//	printf("%s\n",argument);
		}
		p = argument;	
	}

	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	//Check for double free
	int flag = 0;
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		if(_outFile == _errFile){
			flag = 1;
		}
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile && flag != 1) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		
		prompt();
		return;
	}
	
	//IGNORE CTRL-C
	if(!strcmp(_simpleCommands[0]->_arguments[0],"exit")){
	//	printf("Bye bye\n");
		exit(2);
	}
	


	// Print contents of Command data structure
////////////////////*******	print();
	
	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	
	//Save default input output error
	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );
	//Input, Output, Error Files.
	int inputfd;
	int outfd;
	int errfd;
	//variables for fork()
	int pid;
	int flag = 0;
	int flag2 = 0;

	///////////////Redirecte input file, output file and error file///////////
/**/	if(_inputFile){
/**/		inputfd = open(_inputFile,O_RDONLY|O_CREAT,0666);
/**/		if(inputfd < 0){
/**/			perror("ERROR:***Creat inputfile\n");
			exit(2);
		}
	}else{
		inputfd = dup(defaultin);
	}


	if(_errFile){
		if(_append == 1){
			errfd = open(_errFile,O_RDWR|O_APPEND,0666);
			if(errfd < 0){
				perror("ERROR:***Append errfd\n");
				exit(2);
			}
		}else{
			//Not append errFile
			errfd = open(_errFile,O_CREAT|O_RDWR|O_TRUNC,0666);
			if(errfd < 0){
				perror("ERROR:***Creat errfd\n");
				exit(2);
			}
		}
	}else{
				//If there is no _errFile
		errfd = dup(defaulterr);			
	}

	for(int i = 0; i < _numberOfSimpleCommands;i++){
		dup2(inputfd, 0);
		close(inputfd);
		if(i == _numberOfSimpleCommands - 1){

			if(_outFile){
			//APPEND ">>"
				if(_append == 1){
		
				//	printf("APPEND=1\n");
					outfd = open(_outFile,O_RDWR|O_APPEND,0666);
					if(outfd < 0){
						perror("ERROR:***Append outfile\n");
						exit(2);
					}
				//">" output
				}else{
					outfd = open(_outFile,O_CREAT|O_RDWR|O_TRUNC,0666);
					if(outfd < 0){
						perror("ERROR:***Creat outfile\n");
						exit(2);
					}
				}
			}else{
				//If there is no _outFIle
				outfd = dup(defaultout);
			}
		}else{
			int fdpipe[2];
			pipe(fdpipe);
			outfd = fdpipe[1];
			inputfd = fdpipe[0];
		//	close(fdpipe[0]);
		//	close(fdpipe[1]);
		}


		//TEST102-106: CD
		if(!strcmp(_simpleCommands[i]->_arguments[0],"cd")){
			int dir;
			if(_simpleCommands[i]->_arguments[1] == NULL){
				dir = chdir(getenv("HOME"));
				if(dir < 0){
			//		perror("ERROR:***chdir cd\n");
				}
				prompt();
				clear();
				return;
			}else{
				dir = chdir(_simpleCommands[i]->_arguments[1]);
				if(dir < 0){
					dup2(outfd,2);
					perror("");
					dup2(errfd,2);
				}
				prompt();
				clear();
				return;
			}
		
		}
		//TEST11: Enviroment: set variable
		if(!strcmp(_simpleCommands[i]->_arguments[0],"setenv")){
			//set environment variable
			pid = setenv(_simpleCommands[i]->_arguments[1],_simpleCommands[i]->_arguments[2],1);
			if(pid < 0){
				perror("ERROR: ***Failed to setenv\n");
				exit(2);
			}
			
			prompt();
			clear();
			return;	
		}
		
		//TEST14: Delete environment variable
		if(!strcmp(_simpleCommands[i]->_arguments[0],"unsetenv")){
			int i = 0;
			char ** p = environ;
			char * search;
			char * cmd;
			cmd = strdup(_simpleCommands[i]->_arguments[1]);
			while(*p != NULL){
				search = strdup(*p);
				if(strstr(search,cmd) != NULL){
					*p = NULL;
				}
				p++;
				i++;
			}


			prompt();
			clear();
			return;
		}



		dup2(errfd,2);
		dup2(outfd,1);
		close(outfd);
		//Strat fork() a process
		pid = fork();

		if(pid < 0){
			//error, failed to fork()
			perror("ERROR: ***Failed to fork a process\n");
			exit(2);
		}else if (pid == 0){
			//CHILD process
			//code for printenv
			if(!strcmp(_simpleCommands[i]->_arguments[0],"printenv")){
				char **p = environ;
				//redirect output
				dup2(outfd,1);
				while(*p != NULL){
					printf("%s\n",*p);
					p++;
				}
			}
			execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);
			perror("ERROR: ***Failed to execute\n");
			exit(2);
		}
	}
	dup2(defaultin,0);
	dup2(defaultout,1);
	dup2(defaulterr,2);
	close(errfd);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	if(!_background){
		waitpid(pid, NULL,0);
	}


	

	// Clear to prepare for next command
	//printf("End before clear: append = %d\n",_append);
	clear();
	//printf("After clear: append = %d\n",_append);
	// Print new prompt
	
	prompt();
	
}

// Shell implementation

void
Command::prompt()
{
	if(isatty(0)){
		printf("myshell>");
		fflush(stdout);
	}
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

//TEST10: Kill Zombie Process
extern "C" void killzombie(int sig)
{
	while(waitpid(-1,NULL,WNOHANG) > 0);
}

//ignore ctrl-c
extern "C" void disp(int sig)
{
//	fprintf(stderr, "\n    Ctrl-C not working anymore!\n");	
	fprintf(stderr,"\n");
	Command::_currentCommand.prompt();
	
}


main()
{	

	//Ctrl-C signal(works, uncommit before test)

	struct sigaction signalAction_C;
	signalAction_C.sa_handler = disp;
	sigemptyset(&signalAction_C.sa_mask);
	signalAction_C.sa_flags = SA_RESTART;

	int error_C = sigaction(SIGINT,&signalAction_C,NULL);
	if(error_C){
		perror("sigaction_C");
		exit(-1);
	}


	Command::_currentCommand.prompt();
	
	


	//ADD signal here
	struct sigaction signalAction;
	signalAction.sa_handler = killzombie;
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_flags = SA_RESTART;

	int error = sigaction(SIGCHLD,&signalAction,NULL);
	if(error){
		perror("sigaction");
		exit(-1);
	}
	//
	yyparse();
}

