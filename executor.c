/* Alexander Bokelman
 * 119602676 */

#define OPEN_FLAGS (O_WRONLY | O_TRUNC | O_CREAT)
#define DEF_MODE 0644

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sysexits.h>
#include <fcntl.h>
#include <err.h>
#include "command.h"
#include "executor.h"

static int recursive_tree(struct tree *t, int input_fd, int output_fd);

int execute(struct tree *t) {

	if (t != NULL) {
		recursive_tree(t, -1, -1);
	} 

   return 0;
}

static int recursive_tree(struct tree *t, int input_fd, int output_fd) {

	pid_t pid;
	int stat1, stat2;

	if (t->conjunction == NONE) {

		if (!strcmp(t->argv[0], "cd")){
			if (t->argv[1] != NULL){
				if (chdir(t->argv[1])) {
					perror("could not cd to dest\n");
					exit(EX_OSERR);
				}
			}
			else {
				chdir(getenv("HOME"));
			}
		}
		else if (!strcmp(t->argv[0], "exit")) {
			exit(0);
		}
		else {
			if ((pid = fork()) < 0) {
				perror("forking error\n");
				exit(EX_OSERR);
			}
			else if (pid > 0) {	/* parent code */
				wait(&stat1);
				return stat1;
   			}
			else {						/* child code */
				if (t->input) {

					if ((input_fd = open(t->input, O_RDONLY)) < 0) {
						perror("Can't open input file - NONE\n");
						exit(EX_OSERR);
					}
					if ((dup2(input_fd, STDIN_FILENO)) < 0) {
						perror("dup2 failed in input - NONE\n");
						exit(EX_OSERR);
					}
					close(input_fd);
				}

				if (t->output) {
				
					if ((output_fd = open(t->output, OPEN_FLAGS, DEF_MODE)) < 0) {
						perror("Can't open output file - NONE\n");
						exit(EX_OSERR);
					}
					if ((dup2(output_fd, STDOUT_FILENO)) < 0) {
						perror("dup2 fialed in output - NONE\n");
						exit(EX_OSERR);
					}
					close(output_fd);
				}

				execvp(t->argv[0], t->argv);
				fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
				fflush(stdout);
				exit(EX_OSERR);

			}
			printf("other command given\n");
		}
	}
	else if (t->conjunction == AND) {

		if ((pid = fork()) > 0) {	/* parent code */
			wait(&stat1);
			return stat1;
 		}
		else {						/* child code */
			if (t->input) {

				input_fd = open(t->input, O_RDONLY);
				if (input_fd < 0) {
					perror("Can't open input file - AND\n");
					exit(EX_OSERR);
				}
				if ((dup2(input_fd, STDIN_FILENO)) < 0) {
					perror("dup2 failed in input - AND\n");
					exit(EX_OSERR);
				}
				close(input_fd);
			}

			if (t->output) {

				output_fd = open(t->output, OPEN_FLAGS, DEF_MODE);
				if (output_fd < 0) {
					perror("Can't open output file - AND\n");
					exit(EX_OSERR);
				}
				if ((dup2(output_fd, STDOUT_FILENO)) < 0) {
					perror("dup2 failed in output - AND\n");
					exit(EX_OSERR);
				}
				close(output_fd);
			}
		}

		stat2 = recursive_tree(t->left, input_fd, output_fd);
		if (stat2 == 0) {
			recursive_tree(t->right, input_fd, output_fd);
		}
		exit(0);
	}
	else if (t->conjunction == PIPE) {
		int pipe_fd[2];

		if (t->left->output) {
			printf("Ambiguous output redirect.\n");
			fflush(stdout);
		} 
		else if (t->right->input){
			printf("Ambiguous input redirect.\n'");
			fflush(stdout);
		}
		else {

			if (t->input) {
				if ((input_fd = open(t->input, O_RDONLY)) < 0) {
					perror("Can't open input file - PIPE\n");
					exit(EX_OSERR);
				}
			}
			if (t->output) {
				
				if ((output_fd = open(t->output, OPEN_FLAGS, DEF_MODE)) < 0) {
					perror("Can't open output file - PIPE\n");
					exit(EX_OSERR);
				}
			}		
	
			if (pipe(pipe_fd) < 0) {
				perror("Pipe failed\n");
				exit(EX_OSERR);
			}
			if ((pid = fork()) < 0) {
				perror("forking error\n");
				exit(EX_OSERR);
			}
			if (pid == 0) {				/* Child process */
				close(pipe_fd[0]);
				if ((dup2(pipe_fd[1], STDOUT_FILENO) < 0)) {
					perror("dup2 error - PIPE\n");
					exit(EX_OSERR);
				}
				recursive_tree(t->left, input_fd, pipe_fd[1]);
				close(pipe_fd[1]);
				exit(0);
			}
			else {						/* Parent process */
				close(pipe_fd[1]);
				if ((dup2(pipe_fd[0], STDIN_FILENO)) < 0) {
					perror("dup2 failed - PIPE\n");
					exit(EX_OSERR);
				}
				recursive_tree(t->right, pipe_fd[0], output_fd);
				close(pipe_fd[0]);
				wait(NULL);
			}
		}

	}
	else if (t->conjunction == SUBSHELL) {

		if (t->input) {
			if ((input_fd = open(t->input, O_RDONLY)) < 0) {
				perror("Can't open input file - SUBSHELL\n");
				exit(EX_OSERR);
				}
		}
		if (t->output) {
				
			if ((output_fd = open(t->output, OPEN_FLAGS, DEF_MODE)) < 0) {
				perror("Can't open output file - SUBSHELL\n");
				exit(EX_OSERR);
				}
		}		

		if ((pid = fork()) < 0) {
			perror("forking error");
			exit(EX_OSERR);
		}
		else if (pid > 0) {						/* Parent process */
			wait(NULL);
		} 
		else {									/* Child process */
			recursive_tree(t->left, input_fd, output_fd);
		}
		exit(0);
	}


	return 0;
}




