#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int    picoshell(char **cmds[])
{
	int i = 0;
	int num_cmds = 0;
	int **pipes;
	int pid;

	// Count number of commands
	while (cmds[num_cmds])
		num_cmds++;

	if (num_cmds == 0)
		return 0;

	// Allocate pipes (we need num_cmds - 1 pipes)
	pipes = malloc(sizeof(int *) * (num_cmds - 1));
	if (!pipes)
		return 1;

	// Create all pipes
	for (i = 0; i < num_cmds - 1; i++)
	{
		pipes[i] = malloc(sizeof(int) * 2);
		if (!pipes[i] || pipe(pipes[i]) == -1)
		{
			// Clean up allocated pipes
			while (i >= 0)
			{
				if (pipes[i])
				{
					close(pipes[i][0]);
					close(pipes[i][1]);
					free(pipes[i]);
				}
				i--;
			}
			free(pipes);
			return 1;
		}
	}

	// Fork and execute each command
	for (i = 0; i < num_cmds; i++)
	{
		pid = fork();
		if (pid == -1)
		{
			// Error: close all pipes and return
			for (int j = 0; j < num_cmds - 1; j++)
			{
				close(pipes[j][0]);
				close(pipes[j][1]);
				free(pipes[j]);
			}
			free(pipes);
			return 1;
		}

		if (pid == 0)  // Child process
		{
			// If not first command, redirect stdin from previous pipe
			if (i > 0)
			{
				if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
					exit(1);
			}

			// If not last command, redirect stdout to next pipe
			if (i < num_cmds - 1)
			{
				if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
					exit(1);
			}

			// Close all pipe fds in child
			for (int j = 0; j < num_cmds - 1; j++)
			{
				close(pipes[j][0]);
				close(pipes[j][1]);
			}

			// Execute command
			execvp(cmds[i][0], cmds[i]);
			exit(1);  // If execvp fails
		}
	}

	// Parent: close all pipes
	for (i = 0; i < num_cmds - 1; i++)
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
		free(pipes[i]);
	}
	free(pipes);

	// Wait for all children
	for (i = 0; i < num_cmds; i++)
	{
		wait(NULL);
	}

	return 0;
}
