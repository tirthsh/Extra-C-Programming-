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

int showprompt = 1;
int laststatus = 0;  /* set by anything which runs a command */


int main()
{
    char buf[1000];
    struct parsed_line *p;
    extern void execute(struct parsed_line *p);

    while (1) {
	if (showprompt)
	    printf("$ ");
	if (fgets(buf, sizeof buf, stdin) == NULL)
	    break;
	if ((p = parse(buf))) {
	    execute(p);
	    freeparse(p);
        }
    }

    return(laststatus);
}


void execute(struct parsed_line *p)
{
    int status;
    extern void execute_one_subcommand(struct parsed_line *p);

    fflush(stdout);
    switch (fork()) {
    case -1:
	perror("fork");
	laststatus = 127;
	break;
    case 0:
	/* child */
	execute_one_subcommand(p);
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
void execute_one_subcommand(struct parsed_line *p)
{
    if (p->inputfile) {
	close(0);
	if (open(p->inputfile, O_RDONLY, 0) < 0) {
	    perror(p->inputfile);
	    exit(1);
	}
    }
    if (p->outputfile) {
	close(1);
	if (open(p->outputfile, O_WRONLY|O_CREAT|O_TRUNC, 0666) < 0) {
	    perror(p->outputfile);
	    exit(1);
	}
    }
    if (p->pl)
	printf("execute %s\n", p->pl->argv[0]);
    exit(0);
}
