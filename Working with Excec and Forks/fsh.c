/*
 * fsh.c - the Feeble SHell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "fsh.h"
#include "parse.h"
#include "error.h"
#include <string.h>
#include <sys/stat.h>



int showprompt = 1;
int laststatus = 0;  /* set by anything which runs a command */
int forking = 0;

extern  char**environ;
extern int pathExists(char *path);
extern void printUsageMessage(char *path);
extern void printgetOptUsageMessage(char *arg);


int main(int argc, char **argv)
{
    char buf[1000];
    char *newCommand;
    struct parsed_line *p;
    extern void execute(struct parsed_line *p);
    int option;
    int vflag = 0;
    int iflag = 0;
    int cflag = 0;

    //use getopt to find if option entered is valid 
    option = getopt(argc,argv,"vic:");
    switch(option){
    	case 'v':
    		vflag = 1;
    		break;
    	case 'i':
    		iflag = 1;
    		break;
    	case 'c':
    		cflag = 1;
    		//printf("%d\n", optind);
    		if(optind == 2)
    			printgetOptUsageMessage(argv[0]);
    		else{
    			int count = 0;
    			for (; optind < argc; optind++){
    				//printf("arg %s\n", argv[optind]);
    				count++;
    			}
    			//printf("%d\n", count);
    			if(!(count == 0))
    				printgetOptUsageMessage(argv[0]);
    			else{
    				newCommand = argv[argc-1];
    				//printf("New command: %s\n", newCommand);
    			}
    		}
    		break;
    	case '?': 
    		//print usage messasge if option is not v, i or c 
    		printgetOptUsageMessage(argv[0]);
    }
    while (1) 
    {
		if (showprompt){
			if((isatty(fileno(stdin))) || (iflag == 1))
	    		printf("$ ");
		}
		if (fgets(buf, sizeof buf, stdin) == NULL)	
		    break;
		if(cflag){
			if((p = parse(newCommand))){
				execute(p);
				freeparse(p);
			}
		}
		else{
			if ((p = parse(buf))) {
				if(vflag)
					printf("%s", buf);
	    		execute(p);
	    		freeparse(p);
        	}
		}
    }
    return(laststatus);
}


void execute(struct parsed_line *p)
{
    int status;
    extern void execute_one_subcommand(struct parsed_line *p,int isTherePipe);
    //int pipe_double_redirection = p->pl->next->isdouble;



    fflush(stdout);
    switch (fork()) {
    case -1:
		perror("fork");
		laststatus = 127;
		break;
    case 0: 		
    	/* child */
    	//contains pipe
    	if(p->pl->next){
    		//printf("Has pipe\n");
    		int pid;
    		int pipefd[2];
    		if((pipe(pipefd)) == -1){
    			perror("Pipe failed.\n");
    			laststatus = 1; 

    		}
    		//pipefd[0] is now open for reading
    		//pipefd[1] is now open for writing
    		pid = fork();
    		if(pid == -1){
    			perror("Fork failed\n");
    			laststatus = 1; 

    		}
    		//child process
    		else if(pid == 0){
    			//printf("Child process.\n");
    			//do redirections and close the wrong side of the pipe
				close(pipefd[0]);  
    			dup2(pipefd[1],1); //automatically closes previous of fd 1
    			close(pipefd[1]);  //clean up 
    			execute_one_subcommand(p,0); //parameter 0 to notify if child or parent
    		}
    		else{ //parents process
    			//printf("Parent process\n");
    			close(pipefd[1]);
    			dup2(pipefd[0],0); //automatically closes preivous of fd 0
    			//if(pipe_double_redirection)
    			//	dup2(pipefd[1],2);
    			close(pipefd[0]); 
    			execute_one_subcommand(p,1);
    		}
    	}
    	else
    		//no pipes in the input
    		execute_one_subcommand(p,0);
		break;
    default:
		/* parent */
		wait(&status);
		laststatus = status >> 8;

    }
}

	
/*
 * execute_one_subcommand():
 * Do file redirections if applicable, then [you can fill this in...]
 * Does not return, so you want to fork() before calling me.
 */
void execute_one_subcommand(struct parsed_line *p,int isTherePipe)
{
	int double_redirection = p->output_is_double;

	char *input;

   	if(!(isTherePipe))
		input = p->pl->argv[0];
	else
		input = p->pl->next->argv[0];



    if (p->inputfile)
    {
		close(0);
		if (open(p->inputfile, O_RDONLY, 0) < 0) {
		    perror(p->inputfile);
		    exit(1);
		}
    }
    if (p->outputfile)
    {
    	
		close(1);
		//printf("After close: %d\n", double_redirection);

		if (open(p->outputfile, O_WRONLY|O_CREAT|O_TRUNC, 0666) < 0) {
			//printf("%d\n", double_redirection);
			perror(p->outputfile);
			exit(1);
		}
		if(double_redirection == 1)
			dup2(1,2);
		//printf("%d\n", double_redirection);


		//close(1);
    }
    //printf("%d\n", pipe_double_redirection);
   // if(pipe_double_redirection == 1)
	//	dup2(1,2);



    //make sure p->pl is not a null pointer
    if (p->pl)
    {
		//printf("execute %s\n", p->pl->argv[0]);
		char *file;
		char *path;

		//has no slashes
		if((strchr(input,'/')) == NULL)
		{

			//printf("Goes in 1\n");

			//do step 5, take command cat for example
		  	//create '/bin/cat' and check if it exists using stat()
		  	char *path = efilenamecons("/bin/",input);	
		  	//pathExists() returns 0 if it path exists, else 1
		  	if(!(pathExists(path))){
				file = path;
		  	}
		   	else
		   	{
		   		//printf("Expected: Not found in /bin/%s\n", input);
		   		//printf("Acual: Not found in %s\n", path);

		   		//check path '/usr/bin/cat' if '/bin/cat' DNE
				path = efilenamecons("/usr/bin",input); //     /usr/bin/cat doesnt work
				if(!(pathExists(path)))
					file = path;
				else
				{
		   			//printf("Expected: Not found in /usr/bin/%s\n", input);
		   			//printf("Acual: Not found in %s\n", path);

					//check path 'cat' if '/bin/cat' and '/usr/bin/cat' DNE
					path = input;
					if(!(pathExists(path)))
						file = path;
					else{ //if none of the exist then print usage message
						laststatus = 1; 
						printUsageMessage(path); 
					}
				}
		   	}
		} 
		//has slashes = abs plath
		else{
	   		path = input; //first argument to exec if abs path
	   		if(!(pathExists(path)))
	   			file = path;
	   		else{
	   			laststatus = 1;
	   			printUsageMessage(path);
	   			//printf("hi\n");
	   		}
		}
		if(laststatus != 1){
			//if parent then 2nd arg is not p->pl->argv
			//stdin is from pipe
			if(!(isTherePipe))
				execve(file,p->pl->argv,environ);
			else
				execve(file,p->pl->next->argv,environ);

		 	perror(input);
		 	laststatus = 1; 
		}
    }
    exit(0);
}


/*
Helper method which checks if a path exists or not using stat. Returns 0
on sucess, or else 1. Takes in a character pointer to a path as its only 
argument.
*/
int pathExists(char *path){
	struct stat statbuf;
	int status = 1;
	if(stat(path,&statbuf)){
		//printf("Error with stat.\n");
		status = 1;
	}
	else{
		if(S_ISREG(statbuf.st_mode)){
			//printf("Regular file passed\n");
			status = 0;
		}
	}
	return status;
	//return (stat (path,&statbuf) == 0);
}


/*
A void method which prints out an error messaging telling the user the 
command entered does not exist.
*/

void printUsageMessage(char *path){
	fprintf(stderr,"%s: command not found\n",path);
}


/*
A void method which prints out an usege messaging telling the user the 
options or the number of arugments are invalid.
*/
void printgetOptUsageMessage(char *arg){
	fprintf(stderr, "usage: %s [-i] [-v] [{file | -c command}]\n",arg);
    exit(1);
}

