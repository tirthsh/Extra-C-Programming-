#include <stdio.h>
#include <unistd.h>

int main()
{
    char *x[3];
    extern char **environ;

    x[0] = "cat";
    x[1] = "a.c";
    x[2] = NULL;
    //does the cat command on x[1]
    //no such file called 'a wacky file name ...' exists 
    //so prints out error mssg
    execve("/bin/cat", x, environ);
    perror("/bin/cat");
    return(1);
}
