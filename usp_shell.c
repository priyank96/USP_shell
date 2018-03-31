#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUM_TOKEN 10
#define DELIMITERS " \t\r\n\a"


int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}


int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}
int lsh_exit(char **args)
{
  return 0;
}

int lsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}




char **get_tokens(char *line)
{
	
	int position = 0;
	char **tokens = malloc(NUM_TOKENS * sizeof(char*));
	char *token = strtok(line, DELIMITERS);

	while (token != NULL)
	{
		tokens[position++] = token;
		token = strtok(NULL, DELIMITERS);
	}
	tokens[position] = NULL; // passing list to exec has to be null terminated
	return tokens;
}

int main(int argc, char **argv)
{

	char **args;
	int status = 1;
	
	char *line = NULL;
	long bufsize = 0; // getline allocates a buffer
  while (status)
	{
		printf("> ");
		getline(&line, &bufsize, stdin);
		args = get_tokens(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	} 

  return 0;
}

