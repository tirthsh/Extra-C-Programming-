#include <stdio.h>
#include <unistd.h>

int main()
{
    char *x[3];
    extern char **environ;

    x[0] = "s";
    x[1] = "sol";
    x[2] = NULL;
    execve("/bin/ls", x, environ);
    perror("/bin/cat");
    return(1);
}
