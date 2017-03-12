/*
 * This program demonstrates the use of getopt().
 *
 * Please note:
 * Not only does it have some pedagogical comments, it has some pedagogical
 * printfs!  None of these should appear in any serious program.
 * That is, you don't print out what the command-line options are; you just
 * make use of them.
 *
 * Having global variables for command-line option values is a common
 * practice.  Also, I think it's a reasonable practice.  In this style, you
 * initialize them to their default values, so that if the getopt() loop
 * doesn't change their values, they have the default values.  You may be
 * able to make them "file static" (to be discussed in lecture shortly!) as
 * they are below, so that they are private to your one .c file which contains
 * the main().  This is sometimes feasible, sometimes not.
 *
 * The primary purpose of the "status" variable below is to avoid having
 * multiple usage-message-outputting code, in a way which isn't illustrated
 * here:  You can have a getopt error, in which case getopt() returns '?';
 * and you can experience errors in analyzing the arguments.  Furthermore,
 * the place which says "if (status)" also may contain an additional if clause
 * about the value of optind, if the program cannot take zero or more files.
 * For example, in writing "grep" we would be saying "if (status || optind ==
 * argc)", because optind==argc means that there are no further arguments,
 * and grep requires at least one non-option argument (the pattern to be
 * matched).
 *
 * The "status" variable also potentially becomes your program's process
 * exit status; that's the origin of its name, and it is indeed returned below.
 */

#include <stdio.h>
#include <stdlib.h>  /* for atoi() */
#include <unistd.h>  /* for getopt() */

static char *progname;
static int xflag = 0, count = 12;  /* i.e. default is no x, and -c 12 */


int main(int argc, char **argv)
{
    int c, status = 0;

    progname = argv[0];

    while ((c = getopt(argc, argv, "c:x")) != EOF) {
	switch (c) {
	case 'c':
	    count = atoi(optarg);
	    break;
	case 'x':
	    xflag = 1;
	    break;
	case '?':
	default:
	    status = 1;
	    break;
	}
    }

    if (status) {
	fprintf(stderr, "usage: %s [-c count] [-x] [name ...]\n", progname);
	return(status);
    }

    printf("Command-line options:\n");
    printf("    count is %d.\n", count);
    printf("    'x' flag is %s.\n", xflag ? "on" : "off");

    if (optind == argc) {
	printf("and there are no further arguments.\n");
    } else {
	for (; optind < argc; optind++) {
	    printf("arg %s\n", argv[optind]);
	}
    }
    return(status);
}
