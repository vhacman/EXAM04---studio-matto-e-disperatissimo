#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>

static void	handle_child_process(int fd[2], const char *file, char *const argv[], char type)
{
	if (type == 'r')
	{
		if (dup2(fd[1], STDOUT_FILENO) == -1)
			exit(1);
	}
	else
	{
		if (dup2(fd[0], STDIN_FILENO) == -1)
			exit(1);
	}
	close(fd[0]);
	close(fd[1]);
	execvp(file, argv);
	exit(1);
}

static int	handle_parent_process(int fd[2], char type)
{
	if (type == 'r')
	{
		close(fd[1]);
		return (fd[0]);
	}
	else
	{
		close(fd[0]);
		return (fd[1]);
	}
}

int	ft_popen(const char *file, char *const argv[], char type)
{
	int		fd[2];
	pid_t	pid;

	if (!file || !argv || !argv[0] || (type != 'r' && type != 'w'))
		return (-1);
	if (pipe(fd) == -1)
		return (-1);
	pid = fork();
	if (pid == -1)
	{
		close(fd[0]);
		close(fd[1]);
		return (-1);
	}
	if (pid == 0)
		handle_child_process(fd, file, argv, type);
	else
		return (handle_parent_process(fd, type));
}

// int	main(void)
// {
// 	int		fd;
// 	char	buffer[1024];
// 	int		bytes;

// 	// Test ls
// 	fd = ft_popen("ls", (char *const []){"ls", NULL}, 'r');
// 	if (fd == -1)
// 	{
// 		printf("ft_popen failed\n");
// 		return (1);
// 	}
// 	while ((bytes = read(fd, buffer, sizeof(buffer) - 1)) > 0)
// 	{
// 		buffer[bytes] = '\0';
// 		printf("%s", buffer);
// 	}
// 	close(fd);
// 	return (0);
// }


// // Test cat
// fd = ft_popen("cat", (char *const []){"cat", "file.txt", NULL}, 'r');
// if (fd == -1)
// {
// 	printf("ft_popen cat failed\n");
// 	return (1);
// }
// while ((bytes = read(fd, buffer, sizeof(buffer) - 1)) > 0)
// {
// 	buffer[bytes] = '\0';
// 	printf("%s", buffer);
// }
// close(fd);
// // Test grep
// fd = ft_popen("grep", (char *const []){"grep", "pattern", "file.txt", NULL}, 'r');
// if (fd == -1)
// {
// 	printf("ft_popen grep failed\n");
// 	return (1);
// }
// while ((bytes = read(fd, buffer, sizeof(buffer) - 1)) > 0)
// {
// 	buffer[bytes] = '\0';
// 	printf("%s", buffer);
// }
// close(fd);
