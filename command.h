#define MAX_ARGS 10
#define MAX_CMDS 10

/******************************* Structs *********************************/
typedef struct SimpleCommand
	{
		int num_args;
		char *arguments[MAX_ARGS];
	}SimpleCommand;

typedef struct Command
	{
		int num_simple_commands;
		SimpleCommand *simple_commands[MAX_CMDS];
		char *out_file;
		char *in_file;
		char *err_file;
		int background;	
	}Command;

/******************************* Globals ************************************/

Command *current_command;
SimpleCommand *current_simple_command;

/********************************* Methods **********************************/

void insertSimpleCommand(Command *command,SimpleCommand *simpleCommand);
void insertArgument(SimpleCommand *simpleCommand, char *argument);
void execute(Command *command);
