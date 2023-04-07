#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

/*Main shell functions*/
void smsh_body();
char* smsh_read_line(); 
char** smsh_split_line(char*);
int smsh_execute(char**);
void smsh_createProcess(char**, int, int, char*, int, char*);
int RedirectInput(char **, char **);
int RedirectOutput(char**, char**);
int CheckAmpersand(char**);
int CheckFileExists(char *);
void CheckProcessID(char **);
void Replace$$(char*, char *, char*);
void sigHandler(int);

/*Functions for builtin shell commands*/
int smsh_cd(char**);
int smsh_exit();
int smsh_status();
int num_builtins();

char *builtin_strings[] = {
	"cd", "exit", "status"
};

/*Global variables*/
int currStatus = 0;
int numArgs = 0;
pid_t parentPid;
char message[200] = {0};
int fgOnlyMode = 0;

// Function Name: smsh_cd
// Description: Implements the built-in shell command cd. 
// Is capable of changing directory to 'HOME' and to whatever directory is in the first argument.
// Also checks and returns whether if the chdir was successful.
int smsh_cd(char** args){
	int i;
	if(numArgs == 1){
		i = chdir(getenv("HOME"));
	}else{
		i = chdir(args[1]);
	}
	if(i != 0){
		currStatus = 1;
	}
	return 1;
}

// Function Name: smsh_exit
// Description: Used for exiting the smallsh.
int smsh_exit(){
	exit(0);
}

// Function Name: smsh_status
// Description: Prints the status of the previously run command. 
int smsh_status(){
	printf("exit value %d\n", currStatus);
	return 1;
}

// Function Name: num_builtins
// Description: Returns the size of the builtin_strings array for use in executing the built-in commands.
int num_builtins(){
	return sizeof(builtin_strings)/sizeof(char*);
}

// Function Name: smsh_body
// Description: The main body of the program. Loops infinitely until exit is called. 
void smsh_body(){
	char *line;
	char **args;
	int exitStatus;
	while(1){
		line = smsh_read_line();
		args = smsh_split_line(line);
		exitStatus = smsh_execute(args);
		free(line);
		free(args);
	}

}

// Function Name: smsh_read_line
// Description: Handles the the command line newline symbol and reading input from the user.
// Returns the read line.
char* smsh_read_line(){

	write(STDOUT_FILENO, message, sizeof(message));
	memset(message, 0, sizeof(message));
	char *line = NULL;
	ssize_t bufferSize = 2048;
	int numCharsEntered = -5;
	while(1){
		printf(": ");
		numCharsEntered = getline(&line, &bufferSize, stdin);
		if(numCharsEntered == -1){
			clearerr(stdin);
		}else{
			break;
		}
	}
	line[strcspn(line, "\n")] = '\0';
	return line;
}

// Function Name: smsh_split_line
// Description: Does the parsing of characters in the inputted command. Tokenizes each argument/element in the line based on the delimiter.
// Returns the tokenized line.
char** smsh_split_line(char* line){
	int bufferSize = 512;
	int pos = 0;
	char **tokens = malloc(bufferSize * sizeof(char*));
	char *token;
	char delimiter[] = " \t\r\n\a";	

	token = strtok(line, delimiter);
	while(token != NULL){
		tokens[pos] = token;
		pos++;
		token = strtok(NULL, delimiter);
	}
	tokens[pos] = NULL;
	return tokens;
}

// Function Name: smsh_execute
// Description: Executes the tokenized arguments from the command. Checks whether the command is a valid, a comment, a built-in command, or exec command
// and runs the appropriate function for handling the command. Also checks for $$, blocking &, and file redirection symbols. 
int smsh_execute(char **args){	
	int i;
	int argsCounter = 0;
	if(args[0] == NULL){
		return 1;
	}else if(strstr(args[0], "#") != 0){
		return 1;
	}
	
	for(i = 0; args[i] != '\0'; i++){
		argsCounter++;
	}
	numArgs = argsCounter;
		
	CheckProcessID(args);

	for(i = 0; i < num_builtins(); i++){
		if(strcmp(args[0], "cd") == 0){
			return smsh_cd(args);
		}else if(strcmp(args[0], "exit") == 0){
			return smsh_exit();
		}else if(strcmp(args[0], "status") == 0){
			return smsh_status();
		}
	}
	int input, output, block;
	char *outputFileName = NULL;
	char *inputFileName = NULL;
	
	block = CheckAmpersand(args);
	
	input = RedirectInput(args, &inputFileName);
		
	output = RedirectOutput(args, &outputFileName);
	
	smsh_createProcess(args, block, input, inputFileName, output, outputFileName);
	return 1;
}

// Function Name: CheckAmpersand
// Description: Checks if there is an & in the tokenized arguments. 
int CheckAmpersand(char **args){
	int j;
	for(j = 1; args[j] != NULL; j++){
		if(strcmp(args[j],"&") == 0){
			args[j] = NULL;
			return 0;
		}
	}
	return 1;
} 

// Function Name: RedirectInput
// Description: Checks if there is a < symbol in the tokenized arguments. Also stores the file name of the input file
// in inputFileName if < is found. 
int RedirectInput(char **args, char **inputFileName){
	int i;
	int j;
	for(i = 0; args[i] != NULL; i++){
		if(strcmp(args[i], "<") == 0){

			if(args[i+1] != NULL){
				*inputFileName = args[i+1];
			}else{
				return -1;
			}	
			for(j = i; args[j-1] != NULL; j++){
				args[j] = args[j+2];
			}
			return 1;
		}
	}
}

// Function Name: RedirectOutput
// Description: Checks if there is a > symbol in the tokenized arguments. Also stores the file name of the output file
// in outputFileName if > is found. 
int RedirectOutput(char **args, char **outputFileName){
	int i;
	int j;
	for(i = 0; args[i] != NULL; i++){
		if(strcmp(args[i], ">") == 0){

			if(args[i+1] != NULL){
				*outputFileName = args[i+1];
			}else{
				return -1;
			}	
			for(j = i; args[j-1] != NULL; j++){
				args[j] = args[j+2];
			}
			return 1;
		}
	}
}

// Function Name: CheckFileExists
// Description: Checks if a file exists using stat.
int CheckFileExists(char *fileName){
		if(fileName != NULL){
			struct stat buffer;
			int exist = stat(fileName, &buffer);
			if(exist == 0){
				return 1;
			}else{
				return 0;
			}
		}
		return 1;
}

// Function Name: CheckProcessID
// Description: Checks if && is a tokenized argument. Replaces the every $$ symbol with the shell's process ID. 
void CheckProcessID(char **args){
	int j;
	int pid = getpid();
	char processID[20];
	sprintf(processID, "%d", pid);
	char *searchString = "$$";

	for(j = 1; args[j] != NULL; j++){
			Replace$$(args[j], searchString, processID);		
	}
} 

// Function Name: Replace$$
// Description: Called by CheckProcessID. Does the replacing of the $$ symbol with the shell's process ID. 
void Replace$$(char* string, char *searchString, char* replaceString){
	char buffer[100];
	char *ch;

	if(!(ch = strstr(string, searchString))){
		return;
	}
	strncpy(buffer, string, ch-string);
	buffer[ch-string] = 0;
	sprintf(buffer+(ch-string), "%s%s", replaceString, ch + strlen(searchString));
	string[0] = 0;
	strcpy(string, buffer);
	return Replace$$(string, searchString, replaceString);
}

// Function Name: sigHandler
// Description: The signal handler for handling SIGCHLD, SIGINT, and SIGTSTP signals. Reaps terminated child zombies, makes sure the shell
// does not terminate from SIGINT, and detects if the user wants the shell to go into foreground mode. 
void sigHandler(int sig){
	
	pid_t pid;
	int status;

	switch(sig){
		case SIGCHLD:
			while((pid = waitpid(-1,&status,WNOHANG)) != -1){
				if(pid != 0){	
					if(WIFSIGNALED(status)){
						sprintf(message, "background process %d is done: terminated by signal %d\n", pid, status);
						return;
					}else{
						sprintf(message, "background process %d is done: exit value %d\n", pid, status);
						return;
					}
				}else{
					break;
				}
			}
			break;

		case SIGINT:
			signal(SIGINT, sigHandler);	
			return;
		break;
		
		case SIGTSTP:
			signal(SIGTSTP, sigHandler);
			if(fgOnlyMode == 0){
				sprintf(message, "Entering foreground-only mode (& is now ignored)\n");
				fgOnlyMode = 1;
			}else{
				sprintf(message, "Exiting foreground-only mode\n");
				fgOnlyMode = 0;
			}
			return;
		break;
	}
	
}

// Function Name: smsh_createProcess
// Description: Handles everything dealing with processes. Handles entering and exiting foreground mode, handles file redirection, 
// calls signal handlers, forks child processes, and waits for terminated child processes.
void smsh_createProcess(char **args, int block, int input, char *inputFileName, int output, char *outputFileName){	
	pid_t pid;
	int fdIn, fdOut;
	int status;
	int execResult;
	int result;
	int exitStatus;	

	parentPid = getpid();

	struct sigaction SIG_action = {0};
	SIG_action.sa_handler = sigHandler;
	sigfillset(&SIG_action.sa_mask);
	SIG_action.sa_flags = SA_RESTART;	
	sigaction(SIGCHLD, &SIG_action, NULL);	
	sigaction(SIGINT, &SIG_action, NULL);	
	sigaction(SIGTSTP, &SIG_action, NULL);
	
	if(CheckFileExists(inputFileName) == 0){
		printf("cannot open %s for input\n", inputFileName);
		currStatus = 1;
		return;
	}
	
	if(CheckFileExists(outputFileName) == 0){
		printf("cannot open %s for output\n", inputFileName);
		currStatus = 1;
		return;
	}	

	pid = fork();

	if(pid == 0){
			
		if(input){
			fdIn = open(inputFileName, O_RDONLY);	
			dup2(fdIn, STDIN_FILENO);
			close(fdIn);
		}
			
		if(output){
			fdOut = open(outputFileName, O_CREAT | O_WRONLY | O_TRUNC, 0644);
			dup2(fdOut, STDOUT_FILENO);
			close(fdOut);
		}
				
		execResult = execvp(args[0], args);
		if(execResult == -1){
			exit(-1);
		}
		exit(0);
	}
	if(fgOnlyMode == 1){
		block = 1;
	}
	if(block){
		result = waitpid(pid, &status, 0);	
		if(WIFEXITED(status)){
			exitStatus = WEXITSTATUS(status);
		}else if(WIFSIGNALED(status)){
			printf("terminated by signal %d\n", WTERMSIG(status));
		}
		if(execResult < 0){
			currStatus = 1;
		}else{
			if(exitStatus == 255){
				printf("%s: no such file or directory\n", args[0]);
			}
			currStatus = exitStatus;
		}
	}else{
		printf("background pid is %d\n", pid);
		fflush(stdout);
	}	
}

// Function Name: main
// Description: the main function for the small shell. 
int main(int argv, char** argc){
	smsh_body();
	return 0;
}
