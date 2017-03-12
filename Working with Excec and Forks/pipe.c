#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	pid_t pid;
	int mypipefd[2];
	int ret;
	char buf[20];

	ret = pipe(mypipefd);

	if(ret == -1){
		perror("pipe fail");
		exit(1);
	}

	pid = fork();

	if(pid == 0){
		//child process
		printf("Child process\n");
		close(mypipefd[0]);
		write(mypipefd[1],"hello there", 12);
	}
	else{
		//parent process
		printf("Parent process\n");
		close(mypipefd[1]);
		read(mypipefd[0],buf,15);
		printf("buff: %s\n", buf);

	}

	return 0;
}
