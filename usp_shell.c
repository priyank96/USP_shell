#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include<regex.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAX_CMD_LEN 100
#define NUM_TOKEN 10
#define NUM_CMDS 5
#define DELIMITERS " \t\r\n\a"

#define CYAN    "\x1b[36m"
#define RED    "\x1b[95m"
#define RESET   "\x1b[0m"


struct history {
    char cmd[MAX_CMD_LEN];
    struct tm* timeinfo;
    int bleh;
} hist[25];

int count = 0;


void put_history(char *cmd) {

    hist[count].bleh = 1;
    strcpy(&hist[count].cmd, cmd);
    hist[count].cmd[strlen(cmd)+1] = '\n';

    time_t rawtime;
    time(&rawtime);
    hist[count].timeinfo = localtime(&rawtime);

    count = (count+1)%25;
}

void get_history() {
    for(int i = 0; i < 25; i++) {
        if (hist[i].bleh) {
            char curr_time[32];
            //strftime(curr_time, 32, "_%Y_%m_%d_%H_%M", );
            printf("%s" RED "%s\n" RESET, asctime(hist[i].timeinfo), hist[i].cmd);
        }
    }
}

char *read_alias(char *cmd) {

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/home/vyam/.usp_rc", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char * ret = NULL;
    char * dub = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        //char **args = malloc(NUM_TOKEN * sizeof(char*));
        //printf("jhk %s\n", line);
        dub = strdup(line);
        char *token = strtok(line, "=");
        token = strtok(line, "=");

        if(strcmp(token, cmd) == 0) {
            ret = strdup(dub+strlen(token)+1);
            break;
        }
    }

    fclose(fp);

    return ret;
}

int exec_process(char **args, int num_tokens) {
    pid_t pid; // for pid of child

    int status; // for return of waitpid

	// arguments to pass to execvp
	char **exec_args[NUM_CMDS];
	exec_args[0] = malloc(NUM_TOKEN * sizeof(char*));
	int args_count = 0;

	// for i/o redirection
    int in_fd = dup(0);
    int out_fd = dup(1);
	int tmp_in = dup(0);
	int tmp_out = dup(1);
	int out_file_fd = -1;
	//for piping
	int num_cmds = 1;
	int pipe_fd[2];
	pipe(pipe_fd);

	int i = 0;
	while(i < num_tokens)
    {
		if(strcmp(args[i],"|")==0)
		{
			exec_args[num_cmds-1][args_count] = NULL;
			exec_args[num_cmds] = malloc(NUM_TOKEN * sizeof(char*));
			num_cmds++;
			args_count= 0;
			i++;
		}
        else if(strcmp(args[i],"<")==0)
        {
            in_fd = open(args[i+1],O_RDONLY);
            i+=2;
		}
        else if(strcmp(args[i],">")==0)
        {


            out_file_fd = open(args[i+1],O_CREAT|O_WRONLY,0777);
            i+=2;



		}
		else
		{
			exec_args[num_cmds-1][args_count++] = strdup(args[i++]);
		}
    }
	exec_args[num_cmds-1][args_count] = NULL;
	for(i=0;i<num_cmds;i++)
	{
		dup2(in_fd,0);
		close(in_fd);
		if( i+1 == num_cmds)
		{
			if(out_file_fd != -1)
				dup2(out_file_fd,1);
			else
				dup2(tmp_out,1);
		}
		else
		{
			int fdpipe[2];
			pipe(fdpipe);
			out_fd = fdpipe[1];
			in_fd = fdpipe[0];
		}
		dup2(out_fd,1);
		close(out_fd);
		if((pid=fork())==-1)
		   perror("Shell: can't fork: %s\n");

		else if(pid==0)
		{


			if (execvp(exec_args[i][0], exec_args[num_cmds-1]) == -1)
		    {
		        perror("Shell: can't exec \n");
		        exit(1);
		    }
		}
		if((pid=waitpid(pid,&status,0))<0)
		    perror("Shell: waitpid error: %s\n");
	}
	dup2(tmp_in,0);
	dup2(tmp_out,1);

	for(i=0;i<num_cmds;i++)
		free(exec_args[i]);
}


char *expand_stars(char *cmd)
{
	char *fil_names[100];
	int num_files = 0;
	char *new_cmd = strdup(" ");
	// get all the file names from the directory
		DIR *d;
		struct dirent *dir;
		d = opendir(".");
		if (d)
		{
				while ((dir = readdir(d)) != NULL)
				{
						fil_names[num_files++] = strdup(dir->d_name);
		//				printf("%s\n",fil_names[num_files-1]);
				}
				closedir(d);
		}

	//get the string to match with
	char *token = strtok(cmd, DELIMITERS);
	char *temp;
	while(token)
	{
			
			if(strstr(token,"*")!=NULL)
			{
				regex_t regex;
				int reti;
				/* Compile regular expression */
				token[strlen(token)-1] = '\0';				
				asprintf(&temp,"^[%s]+.*",token);
				reti = regcomp(&regex, temp, REG_EXTENDED);
				for(int i=0; i<num_files; i++)
					{
					/* Execute regular expression */
						reti = regexec(&regex, fil_names[i], 0, NULL, 0);
						if (reti != REG_NOMATCH) { // match
							asprintf(&temp,"%s %s",new_cmd,fil_names[i]);
							
							//break;
							new_cmd = temp;
							}

					}
			}
			else
			{

				asprintf(&temp,"%s %s",new_cmd,token);
				new_cmd = temp;
			}
		  token = strtok(NULL, DELIMITERS);
	}

	//printf("New command: %s\n",new_cmd);
	//free(cmd);
	return new_cmd;
}

int main(int argc, char **argv)
{

    while (1)
    {
        char cwd[1024];
        char host[64];
        struct passwd *pass;

        getcwd(cwd, sizeof(cwd));
        gethostname(host, sizeof(host));
        pass = getpwuid(getuid());

      // printf("%s@%s:" CYAN "%s" RESET "$ ", pass->pw_name, host, cwd);
		
        char *cmd = NULL;
        long bufsize = 0;
       //	 getline(&cmd, &bufsize, stdin); // no need to allocate memory getline will allocate on its own
		char *prompt;
		asprintf(&prompt,"%s@%s:" CYAN "%s" RESET "$ ", pass->pw_name, host, cwd);
		cmd = readline(prompt);
		put_history(cmd);
		add_history(cmd);
		cmd = expand_stars(cmd);
        char *dub = strdup(cmd);
        char **args = malloc(NUM_TOKEN * sizeof(char*));
        char *token = strtok(cmd, DELIMITERS);

        int i = 0;

        char *alias = NULL;
        //if((alias = read_alias(token)) != NULL) {
        //    token = strtok(alias, DELIMITERS);
        //}
        //else
            token = strtok(dub, DELIMITERS);


        while(token)
        {
            args[i] = token;
            token = strtok(NULL, DELIMITERS);
            i++;
        }

        args[i] = NULL;

        if(args) {
            if(*args[i - 1] == '\\') {
                while(*args[i - 1] == '\\') {
                    i--;
                    args[i] == NULL;
                    printf("> ");
                    cmd = NULL;
                    bufsize = 0;
                    getline(&cmd, &bufsize, stdin);

                    token = NULL;
                    token = strtok(cmd, DELIMITERS);

                    while(token)
                    {
                        args[i] = token;
                        token = strtok(NULL, DELIMITERS);
                        i++;
                    }
                    args[i] = NULL;
                }
            }

            if (strcmp(args[0], "cd") == 0) {
                if (chdir(args[1]) != 0) {
                    perror("cd: ");
                }
            }
            else if(strcmp(args[0], "hist") == 0)
                get_history();
            else if(strcmp(args[0], "exit") == 0)
                exit(0);
            else

                exec_process(args,i);
        }

        free(cmd);
        free(args);
    }
}
