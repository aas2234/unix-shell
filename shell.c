#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "dir_stack.h"
#include "dir_list.h"

#define SHELL_PROMPT "abhi-sh:"
#define SPACE_DELIM " "
#define MAX_ARG_COUNT 100
#define MAX_DIR_SIZE 1000
#define INIT_LINE_SIZE 1000
#define CTRL_C 3

char *current_working_dir;
char *prev_working_dir;
struct dir_stack *dirs_stack;
struct dir_list *path_list;

/* trim leading and trailing whitespace in str
 * ref. http://stackoverflow.com/questions/122616
 */
char *trimwhitespace(char *str)
{
	char *end;

	while (isspace(*str))
		str++;

	if (*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		end--;
	*(end + 1) = 0;
	return str;
}

/* reads a line from stream and returns a pointer to the malloc'd string.
 * returns NULL if stream is empty */
char *read_line(FILE *stream)
{
	char *line = malloc(INIT_LINE_SIZE);
	int c;
	unsigned long i = 0;
	unsigned long line_size = INIT_LINE_SIZE;

	if (!line)
		return NULL;

	while (1) {
		if (i > line_size - 1) {
			line = realloc(line, line_size + INIT_LINE_SIZE);
			line_size += INIT_LINE_SIZE;
		}
		c = getc(stream);
		if (c == CTRL_C) {
			free(line);
			return NULL;
		}
		line[i++] = c;
		if (c == '\n')
			break;
	};
	line[i] = '\0';
	return line;
}

/* creates *argv[] from *cmd and *arg */
char **convert_args_to_arr(char *cmd, char *args)
{
	char *delim = SPACE_DELIM, *token;
	char **argv = malloc(sizeof(char *) * MAX_ARG_COUNT);
	int i = 1;

	if (!argv) {
		return NULL;
	} else {
		argv[0] = malloc(strlen(cmd) + 1);
		strcpy(argv[0], cmd);
		token = strtok(args, delim);
		while (1) {
			if (!token)
				break;
			token = trimwhitespace(token);
			argv[i] = malloc(strlen(token) + 1);
			if (!argv[i])
				return NULL;
			strcpy(argv[i], token);
			++i;
			token = strtok(NULL, delim);
		}
		argv[i] = (char *)0;
	}
	return argv;
}

/* call chdir() to change dir and update global var */
int change_dir(char *args)
{
	char *path = malloc(strlen(args) + 1);
	struct passwd *pwd = getpwuid(getuid());
	strcpy(path, args);

	/* cd with no args should change_dir to home directory */
	if (pwd && !strlen(path)) {
		return change_dir(pwd->pw_dir);
	}
	if (!strcmp(path, "-")) {
		/* switch to prev directory */
		if (!prev_working_dir) {
			fprintf(stdout, "shell: OLDPWD not set\n");
			goto err;
		}
		else
			return change_dir(prev_working_dir);
	}
	if (chdir(path)) {
		perror("ERROR");
		goto err;
	}

	prev_working_dir = realloc(prev_working_dir, strlen(current_working_dir) + 1);
	if (!prev_working_dir)
		goto err;

	strcpy(prev_working_dir, current_working_dir);
	if (!getcwd(current_working_dir, MAX_DIR_SIZE))
		goto err;

	free(path);
	return EXIT_SUCCESS;
err:
	free(path);
	return EXIT_FAILURE;
}

/* print the pushd / popd stack top to bottom */
int dirs(void)
{
	fprintf(stdout, "%s", current_working_dir);
	print_dir_stack(dirs_stack);
	fprintf(stdout, "\n");
	return EXIT_SUCCESS;
}

/* if arg given, push cwd on stack and cd to given arg.
 * if arg not given, cd to top of stack and push cwd.
 * if stack empty, print message */
int pushd(char *args)
{
	struct dir_node *node = NULL, *tmp_node;
	char *path = strtok(args, SPACE_DELIM);

	if (path) {
		node = create_node(current_working_dir);
		if (!node)
			goto err;
		if (!change_dir(path))
			push_dir(dirs_stack, node);
		else
			destroy_node(node);
	} else {
		tmp_node = create_node(current_working_dir);
		if (!tmp_node)
			goto err;
		node = pop_dir(dirs_stack);
		if (node) {
			push_dir(dirs_stack, tmp_node);
			change_dir(node->dir);
		} else {
			fprintf(stdout, "pushd: no other directory\n");
			return EXIT_SUCCESS;
		}
		destroy_node(node);
	}
	/* print complete stack, top to bottom */
	dirs();
	return EXIT_SUCCESS;
err:
	return EXIT_FAILURE;
}

/* cd to popped directory from stack */
int popd(char *args)
{
	struct dir_node *node = pop_dir(dirs_stack);
	if (!node) {
		fprintf(stdout, "popd: Directory stack empty\n");
		return EXIT_FAILURE;
	} else {
		fprintf(stdout, "Changing dir to %s\n", node->dir);
		change_dir(node->dir);
	}

	destroy_node(node);

	/* print complete stack, top to bottom */
	dirs();
	return EXIT_SUCCESS;
}

void print_path_list(struct dir_list *list)
{
	struct dir_node *node = list->head;

	while (node) {
		fprintf(stdout, "%s:", node->dir);
		node = node->next;
	}
	fprintf(stdout, "\n");
}

/* store / remove path from path_list */
int path(char *args)
{
	struct dir_node *node;
	char *operator = strtok(args, SPACE_DELIM);
	char *path = strtok(NULL, SPACE_DELIM);

	if (operator && !path)
		return EXIT_FAILURE;
	else if (!operator && !path) {
		print_path_list(path_list);
		return EXIT_SUCCESS;
	}
	if (!strcmp(operator, "+")) {
		node = create_node(path);
		append_to_list_tail(path_list, node);
		fprintf(stdout, "appended %s to path\n", path);
	} else if (!strcmp(operator, "-")) {
		if (remove_dir_from_list(path_list, path))
			fprintf(stdout, "removed %s from path\n", path);
	} else {
		;
	}
	return EXIT_SUCCESS;
}

/* run the parsed command by fork and exec */
int run_cmd(char *cmd, char *args)
{
	pid_t child_pid;
	char **argv = NULL;

	args = trimwhitespace(args);
	if (!strcmp(cmd, "exit")) {
		if (current_working_dir)
			free(current_working_dir);
		destroy_dir_stack(dirs_stack);
		exit(0);
	} else if (!strcmp(cmd, "cd")) {
		return change_dir(args);
	} else if (!strcmp(cmd, "pushd")) {
		return pushd(args);
	} else if (!strcmp(cmd, "popd")) {
		return popd(args);
	} else if (!strcmp(cmd, "dirs")) {
		return dirs();
	} else if (!strcmp(cmd, "path")) {
		return path(args);
	} else {
		child_pid = fork();
		if (child_pid == 0) {
			char *filepath = NULL;
			struct dir_node *node = path_list->head;
			argv = convert_args_to_arr(cmd, args);
			filepath = cmd;
			if (!argv)
				exit(EXIT_FAILURE);
			execv(cmd, argv);

			/* append path from node to cmd until execv succeeds */
			while (node && errno == ENOENT) {
				if (filepath != cmd)
					free(filepath);
				filepath = malloc(strlen(cmd) + strlen(node->dir) + 2);
				filepath[0] = '\0';
				if (!cmd)
					return EXIT_FAILURE;
				if (node->dir[strlen(node->dir) - 1] == '/') {
					strcat(filepath, node->dir);
					strcat(filepath, cmd);
				} else {
					strcat(filepath, node->dir);
					strcat(filepath, "/");
					strcat(filepath, cmd);
				}
				node = node->next;
				execv(filepath, argv);
			} /* while file is not found, recurse */
			perror("ERROR");
			exit(EXIT_FAILURE);
		} else if (child_pid > 0) {
			int status;
			waitpid(child_pid, &status, 0);
		} else {
			perror("ERROR");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

int parse_cmd(char *line)
{
	char *cmd = NULL;
	char *delim = SPACE_DELIM;

	cmd = strtok(line, delim);
	if (cmd)
		cmd = trimwhitespace(cmd);
	if (cmd && cmd[0] != '\0') {
		run_cmd(cmd, &line[strlen(cmd) + 1]);
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int wait_for_cmd(void)
{
	char *line = NULL;

	fprintf(stdout, SHELL_PROMPT);
	fprintf(stdout, "%s $ ", current_working_dir);
	line = read_line(stdin);
	if (line) {
		parse_cmd(line);
		free(line);
	}
	return EXIT_SUCCESS;
}

/* signal handler for SIGINT */
void sigint_handler(int signum)
{
	wait_for_cmd();
}

int main(int argc, char *argv[])
{
	int retval = EXIT_FAILURE;

	current_working_dir = malloc(MAX_DIR_SIZE + 1);
	if (!current_working_dir)
		goto err;

	current_working_dir[MAX_DIR_SIZE] = '\0';
	dirs_stack = create_dir_stack();
	if (!dirs_stack)
		goto err;

	path_list = create_dir_list();
	if (!path_list)
		goto err;

	if (!getcwd(current_working_dir, MAX_DIR_SIZE))
		perror("ERROR");

	/* handle SIGINT */
	if (signal(SIGINT, sigint_handler) == SIG_IGN)
		signal(SIGINT, SIG_IGN);

	while (1) {
		retval = wait_for_cmd();
		if (retval < 0)
			break;
	}
err:
	if (current_working_dir)
		free(current_working_dir);
	destroy_dir_stack(dirs_stack);
	return retval;
}

