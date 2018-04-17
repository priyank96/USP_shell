#include "command.h"
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>

#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

void insertArgument(SimpleCommand *simpleCommand, char *argument)
{
	if(argument !=NULL)
		simpleCommand->arguments[simpleCommand->num_args++] = strdup(argument);	
	else	
		simpleCommand->arguments[simpleCommand->num_args++] = NULL;

}

void insertSimpleCommand(Command *command,SimpleCommand *simple_command)
{
	command->simple_commands[command->num_simple_commands++] = simple_command;
}

void execute(Command *command)
{
	printf("executing! %d\n",command->num_simple_commands);
	int ret;
	int i = 0;
	int tmpin = dup(0); // should use constants
	int tmpout = dup(1);
    printf("executing! %d\n",command->num_simple_commands);

	int fdin;
	if(command->in_file){
	    fdin = open(command->in_file,O_RDONLY);
	}
	else{
		fdin = dup(tmpin);
	}
    printf("executing! %d\n",command->num_simple_commands);

	int fdout;

	for(i;i<command->num_simple_commands;i++)
	{
		
		insertArgument(command->simple_commands[i], NULL);
		dup2(fdin,0);
		close(fdin);
        printf("executing! %d\n",command->num_simple_commands);

		if(i == command->num_simple_commands-1){
			if(command->out_file){
				fdout = open(command->out_file,O_CREAT|O_WRONLY);
			}
			else{
				
				fdout = dup(tmpout);
							
			}
		}
		else{
			int fdpipe[2];
			pipe(fdpipe);
			fdout=fdpipe[1];
			fdin=fdpipe[0];
		}
        printf("executing! %d\n",command->num_simple_commands);

		dup2(fdout,1); // use constants
		close(fdout);		
		
		ret = fork();
		if(ret == 0)
		{
			if(strcmp(command->simple_commands[i]->arguments[0],"cd") == 0)
			{
				chdir(command->simple_commands[i]->arguments[1]);
			} 		
			else
			{			
				execvp(command->simple_commands[i]->arguments[0], command->simple_commands[i]->arguments);
			}
		}
		if(!command->background)
		{
			waitpid(ret,NULL,0);
		}
        printf("executing! %d\n",command->num_simple_commands);

	}
	dup2(tmpin,0); // use constants
	dup2(tmpout,1);
	close(tmpin);	
	close(tmpout);

    printf("executing! %d\n",command->num_simple_commands);


	char cwd[1024];
	char host[64];
	struct passwd *pass;

	getcwd(cwd, sizeof(cwd));
	gethostname(host, sizeof(host));
	pass = getpwuid(getuid());

	printf("%s@%s:" CYAN "%s" RESET "$ ", pass->pw_name, host, cwd);

}
