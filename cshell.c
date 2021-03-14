#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

typedef struct Command {

	char *name;
	char *time;
	int code;

	struct Command *next;

} Command;

typedef struct EnvVar {

	char *name;
	char *value;

	struct EnvVar *next;

} EnvVar;

void cshell(int script, char **argv);
char *cshell_read();
char *cshell_script();
char **cshell_parse(char *line);
int cshell_execute(char **tokens);
void cshell_print(char **tokens);
void cshell_child(char **tokens);
void cshell_log(char **tokens);
void cshell_var(char **tokens);
void print_log();
int change_dir(char *path);

Command *head = NULL;
Command *current = NULL;

EnvVar *headvar = NULL;
EnvVar *currentvar = NULL;

char *color;
int token_size;

int main(int argc, char **argv) {

	int script;

	if (argc > 1) {

		// Script mode
		script = 1;

	} else {

		// Interactive mode
		script = 0;

	}

	// Run cshell
	cshell(script, argv);

	return 0;

}

void cshell(int script, char **argv) {

	char *line;
	char **tokens;
	int running;
	int index = 0;

	color = malloc(8 * sizeof(char));

	if (script == 1) {

		// Script mode

		// Open .txt file to be read
		FILE *file = fopen(argv[1], "r");

		if (file == NULL) {

			printf("File does not exist.\n");

		} else {

			char readline[100];

			printf("Script mode\n");

			while (fgets(readline, sizeof(readline), file) != NULL) {

				char *newline;

				if ((newline = strchr(readline, '\n')) != NULL) {

					// Change the '\n' in the line into a '\0'
					*newline = '\0';

				}

				// Read line from .txt file
				line = readline;

				// Split command line into multiple tokens
				tokens = cshell_parse(line);

				if (tokens[0] == NULL) {

					// If there are no commands, nothing happens

				} else {

					// If there are commands, execute command
					printf("%s", color);
					cshell_log(tokens);
					running = cshell_execute(tokens);

				}

				free(tokens);

			}

		}

	} else {

		// Interactive mode

		do {

			printf("cshell$ ");
			printf("\e[0;37m");

			// Read command line arguments
			line = cshell_read();

			// Split command line into multiple tokens
			tokens = cshell_parse(line);

			if (tokens[0] == NULL) {

				// If there are no commands, nothing happens

			} else {

				// If there are commands, execute command
				printf("%s", color);
				cshell_log(tokens);
				running = cshell_execute(tokens);

			}

			free(line);
			free(tokens);

		} while (running);

	}

}

char *cshell_read() {

	int buffer_size = 100;
	int buffer_index = 0;
	int c;

	// Create a buffer and allocate memory
	char *buffer = malloc(buffer_size);

	while (1) {

		// Read a character from stdin stream
		c = getchar();

		// If c has not reached a new line, put character into buffer
		if (c != '\n') {

			buffer[buffer_index] = c;
			buffer[buffer_index + 1] = '\0';

		} else {

			// Else return buffer
			return buffer;

		}

		// Increment buffer index
		buffer_index++;

		// If buffer index is greater than the buffer size, reallocate memory
		if (buffer_index + 1 >= buffer_size) {

			buffer_size += 1000;
			buffer = realloc(buffer, buffer_size);

		}

	}


}

char **cshell_parse(char *line) {

	int buffer_size = 100;
	int buffer_index = 0;
	token_size = 0;

	// Create a buffer and allocate memory
	char **buffer = malloc(buffer_size * sizeof(char*));
	char *token;

	// Split command line into multiple tokens
	token = strtok(line, " ");

	while (token != NULL) {

		// Put the token into buffer and increment index
		buffer[buffer_index] = token;
		buffer_index++;

		// If buffer index is greater than the buffer size, reallocate memory
		if (buffer_index >= buffer_size) {

			buffer_size += 1000;
			buffer = realloc(buffer, buffer_size * sizeof(char*));

		}

		// Continue tokenizing the command line
		token = strtok(NULL, " ");
		token_size++;

	}

	// Return the buffer
	return buffer;

}

int cshell_execute(char **tokens) {

	if (strcmp(tokens[0], "exit") == 0) {

		// Exit command
		return 0;

	} else if (strcmp(tokens[0], "print") == 0) {

		// Print command
		cshell_print(tokens);

	} else if (strcmp(tokens[0], "theme") == 0) {

		// Theme command

		if (strcmp(tokens[1], "red") == 0) {

			color = "\e[0;31m";
			printf("%s", color);

		} else if (strcmp(tokens[1], "blue") == 0) {

			color = "\e[0;34m";
			printf("%s", color);

		} else if (strcmp(tokens[1], "green") == 0) {

			color = "\e[0;32m";
			printf("%s", color);

		} else if (strcmp(tokens[1], "yellow") == 0) {

			color = "\e[0;33m";
			printf("%s", color);

		} else if (strcmp(tokens[1], "cyan") == 0) {

			color = "\e[0;36m";
			printf("%s", color);

		}

	} else if (strcmp(tokens[0], "log") == 0) {

		// Log command
		print_log();

	} else if (tokens[0][0] == '$') { 

		// Setting environment variables
		cshell_var(tokens);

	} else if (strcmp(tokens[0], "cd") == 0) {

		// Change directory
		change_dir(tokens[1]);

	} else {

		// Non-built in command
		cshell_child(tokens);

	}

	return 1;

}

void cshell_print(char **tokens) {

	for (int i = 1; i < token_size; i++) {

		// Check each token

		if (tokens[i][0] == '$') {

			// If token is an EnvVar

			// Start from the beginning of the EnvVar linked list
			EnvVar *ptr = headvar;

			char *arg;
			char *var_name;

			// Split the token where the "=" is
			arg = strtok(tokens[i], "=");
			var_name = arg + 1;

			// While ptr has not reached the end of the linked list
			while (ptr != NULL) {

				// Check if the environment variable already exists
				if (strcmp(ptr->name, var_name) == 0) {

					// Print the value of the EnvVar if it exists
					printf("%s ", ptr->value);

				}

				// Go to the next EnvVar
				ptr = ptr->next;

			}

		} else {

			// Else print the token
			printf("%s ", tokens[i]);

		}

	}

	printf("\n");

}

void cshell_child(char **tokens) {

	pid_t child;
	int status;

	child = fork();

	if (child == 0) {

		// Child process

		// Directory path
		char dir[100] = "/bin/";

		// Execute non-built in command
		strcat(dir, tokens[0]);
		status = execvp(dir, tokens);

		if (status == -1) {

			printf("Missing command.\n");

		}

		exit(0);

	} else {

		// Parent process
		waitpid(child, &status, WUNTRACED);

	}

	// Set current command status code to -1
	current->code = -1;

}

void cshell_log(char **tokens) {

	// Create a new Command
	Command *newCommand = (Command *) malloc(sizeof(Command));

	time_t timer;
	struct tm *time_info;
	char *string;

	// Get the current time information
	time(&timer);
	time_info = localtime(&timer);
	string = asctime(time_info);

	// Copy the name of the command into Command struct
	newCommand->name = malloc(strlen(tokens[0]) + 1);
	strcpy(newCommand->name, tokens[0]);

	// Copy the time of the command into Command struct
	newCommand->time = malloc(strlen(string) + 1);
	strcpy(newCommand->time, string);

	newCommand->code = 0;

	if (head == NULL) {

		// If the Command struct linked list does not exist yet, create one
		head = newCommand;
		current = head;

	} else {

		// Else, point next node to the new Command being added to Command struct
		current->next = newCommand;
		current = current->next;

	}

}

void print_log() {

	Command *ptr = head;

	// While Command linked list has not reached the end
	while (ptr->next != NULL) {

		// Print the log
		printf("%s", ptr->time);
		printf("%s %d\n", ptr->name, ptr->code);
		ptr = ptr->next;

	}

}

void cshell_var(char **tokens) {

	char *arg;
	char *var_name;
	char *var_value;

	for (int i = 0; i < token_size; i++) {

		if (strchr(tokens[i], '=') == NULL || tokens[i][strlen(tokens[i]) - 1] == '=') {

			// If no value was given to the variable
			printf("Please enter a value for: %s\n", tokens[i]);
			current->code = -1;

		} else if (tokens[i][0] == '$') {

			// Create a new EnvVar
			EnvVar *newVar = (EnvVar *) malloc(sizeof(EnvVar));

			// Split token by "=", and set the argument before "=" as the variable name
			arg = strtok(tokens[i], "=");
			var_name = arg + 1;

			// Split token by "=", and set the argument after "=" as the variable value
			arg = strtok(NULL, "=");
			var_value = arg;

			// Allocate memory and copy the name into the new EnvVar
			newVar->name = malloc(strlen(var_name));
			strcpy(newVar->name, var_name);

			// Allocate memory and copy the value into the new EnvVar
			newVar->value = malloc(strlen(var_value));
			strcpy(newVar->value, var_value);

			if (headvar == NULL) {

				// If the EnvVar struct linked list does not exist yet, create one
				headvar = newVar;
				currentvar = headvar;

			} else {

				// Else, point next node to the new EnvVar being added to EnvVar struct
				currentvar->next = newVar;
				currentvar = currentvar->next;

			}

		}

	}

}

int change_dir(char *path) {

	return chdir(path);

}