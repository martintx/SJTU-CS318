/**********8*************************************
   > File Name:    yujia_shell.c
   > Author:       Yujia Bao
   > Mail:         sjtu@yujiabao2510.com
   > Created Time: 2015.4.4 01:02:33
************************************************/

#include<stdio.h>
#include<unistd.h> 
#include<stdlib.h> 
#include<sys/types.h> 
#include<string.h>
#include<pwd.h>
#include<sys/wait.h>
//#include<signal.h>
#define MAXLINE 100
#define STD_INPUT  0
#define STD_OUTPUT 1
void printpwd()
{
	char pwdbuf[100];
	char* delim = "/";
	getcwd(pwdbuf, sizeof(pwdbuf));
	char temp1[100];
	char* temp2;
	temp2 = strtok(pwdbuf, delim);
	while(temp2 != NULL)
	{
		strcpy(temp1, temp2);
		temp2 = strtok(NULL, delim);
	}
	printf("%s ~ ",temp1);
	return;
}
void iterate_pipe(char* commands[], int total_num, int current_num){
	if(current_num == total_num){
		char* argv_delim = " ";
		int j = 0;
		int num_argv = 0;
		while(j < strlen(commands[current_num - 1])){
			if (*(commands[current_num - 1]+j) == ' ')
				num_argv += 1;
			j++;
		}
		char* argv[num_argv + 2];
		argv[0] = strtok(commands[current_num - 1], argv_delim);
		j = 0;
		while(argv[j] != NULL){
			j += 1;
			argv[j] = strtok(NULL, argv_delim);
		}
		int temp_pid;
		int result;
		result = execvp(argv[0], argv);
		if (result == -1){
			fprintf(stderr,"Yujia Shell: command not found:%s\n",commands[current_num - 1]);
			exit(1);
		}
	}else{
		int pipe_fd[2];
		int child_pid;
		if(pipe(pipe_fd) == -1){
			fprintf(stderr,"Create pipe error.\n");
			exit(1);
		}
		if((child_pid = fork()) != 0){
			int status;
			waitpid(child_pid, &status, 0);
			close(pipe_fd[1]);
			close(STD_INPUT);
			dup(pipe_fd[0]);
			close(pipe_fd[0]);
			iterate_pipe(commands, total_num, current_num+1);
		}else{	
			close(pipe_fd[0]);
			close(STD_OUTPUT);
			dup(pipe_fd[1]);
			close(pipe_fd[1]);
			char* argv_delim = " ";
			int j = 0;
			int num_argv = 0;
			while(j < strlen(commands[current_num - 1])){
				if (*(commands[current_num - 1]+j) == ' ')
					num_argv += 1;
				j++;
			}
			char* argv[num_argv + 2];
			argv[0] = strtok(commands[current_num - 1], argv_delim);
			j = 0;
			while(argv[j] != NULL){
				j += 1;
				argv[j] = strtok(NULL, argv_delim);
			}
			int result;
			result = execvp(argv[0],argv);
			if (result == -1){
				fprintf(stderr,"Yujia Shell: command not found:%s\n",commands[current_num - 1]);
				exit(1);
			}
		}
	}
}

void process_command(char* buf)
{
	pid_t pid;
	buf[strlen(buf) - 1] = 0;
	if ((pid = fork()) < 0){
		fprintf(stderr, "fork error");
	}else if (pid == 0){
		//split the command and arguments
		char* pipe_delim = "|";
		int i = 0;
		int num_pipe = 0;
		while(i < strlen(buf)){
			if (*(buf+i) == '|')
				num_pipe += 1;
			i ++;
		}
		char* commands[num_pipe + 2];
		commands[0] = strtok(buf, pipe_delim);
		i = 0;
		while(commands[i] != NULL){
			i += 1;
			commands[i] = strtok(NULL ,pipe_delim);
		}
		iterate_pipe(commands,num_pipe + 1, 1);
	}else{
		int status;
		waitpid(pid, &status, 0);
	}
	return;
}

int main()
{
	char buf[MAXLINE];
	int status;
	printf("Welcome to Yujia Shell\n");
	printpwd();	
	while (fgets(buf, MAXLINE, stdin) != NULL){
		if (strlen(buf) == 1)
			continue;
		if (strlen(buf) == MAXLINE - 1)
			printf("Command too long\n");
		else if (strlen(buf) == 5 && buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't')
			exit(0);
		else if (strlen(buf) >= 3 && buf[0] == 'c' && buf[1] == 'd'&& ((buf[2] == ' ') || (buf[2] == '\n'))){
			buf[strlen(buf)-1] = 0;
			char* delim = " ";
			int i = 0;
			int num_argv = 0;
			while(i < strlen(buf)){
				if (*(buf+i) == ' ')
					num_argv += 1;
				i++;
			}			
			char* argv[num_argv + 2];
			argv[0] = strtok(buf,delim);
			i  = 0;
			while(argv[i] != NULL){
				i += 1;
				argv[i] = strtok(NULL, delim);
			}
			if (i == 1){
				struct passwd *user;
				user = getpwuid(getuid());
				chdir(user->pw_dir);
			}else if(i == 2){
				int result = chdir(argv[1]);
				if (result == -1)
					fprintf(stderr, "Yujia Shell: illegial address.\n");
			}else
				fprintf(stderr, "Yujia Shell: illegial address.\n");
			printpwd();
		}else{

			process_command(buf);
			printpwd();
		}
	}
	exit(0);
}
