#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

/*  ft_popen(const char *file, char *const argv[], char type)
	Implementa una versione semplificata di popen(), creando una pipe
	unidirezionale tra il processo chiamante e un processo figlio che
	esegue un comando.

  Parametri:
	- file: nome del programma da eseguire
	- argv: argomenti del programma, terminati da NULL
	- type: 'r' per leggere l’output del comando, 'w' per scrivere nel
			 suo input

  Flusso:
	- Controlla che i parametri siano validi e che il tipo sia 'r' o 'w';
	  altrimenti ritorna -1.
	- Crea una pipe (fd[0] per lettura, fd[1] per scrittura).
	- In caso di errore nella creazione della pipe, ritorna -1.
	- Esegue fork() per creare un processo figlio.
	  * Se fork fallisce, chiude entrambi i file descriptors e ritorna -1.
	- Nel processo figlio:
		* Se type == 'r':
			chiude fd[0] (lettura),
			duplica fd[1] su STDOUT,
			chiude fd[1].
		* Se type == 'w':
			chiude fd[1] (scrittura),
			duplica fd[0] su STDIN,
			chiude fd[0].
		* Esegue il comando con execvp(file, argv).
		* Se execvp fallisce, termina con exit(1).
	- Nel processo padre:
		* Se type == 'r':
			chiude fd[1] e ritorna fd[0] per leggere l’output del figlio.
		* Se type == 'w':
			chiude fd[0] e ritorna fd[1] per scrivere sull’input del figlio.
	- Il descrittore restituito può essere usato per comunicare con il
	  processo figlio attraverso la pipe.
*/
int	ft_popen(const char *file, char *const argv[], char type)
{
	if(!file || !argv || (type != 'r' && type != 'w')) return (-1);
	int	fd[2];
	if(pipe(fd) == -1) return (-1);
	pid_t	pid = fork();
	if(pid == -1) {
		close(fd[0]);
		close(fd[1]);
		return -1;
	}
	if(pid == 0) {
		if(type == 'r')
		{
			close(fd[0]);
			dup2(fd[1], 1);
			close(fd[1]);
		} else {
			close(fd[1]);
			dup2(fd[0], 0);
			close(fd[0]);
		}
		execvp(file, argv);
		exit(1);
	}
	if(type == 'r') {
		close(fd[1]);
		return fd[0];
	} else {
		close(fd[0]);
		return fd[1];
	}
}

// int main(void)
// {
// 	int		fd;
// 	char	buffer[1024];
// 	ssize_t	n;

// 	fd = ft_popen("ls", (char *const []){"ls", NULL}, 'r');
// 	if (fd != -1)
// 	{
// 		while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
// 			buffer[n] = '\0';
// 			printf("%s", buffer);
// 		}
// 		close(fd);
// 		wait(NULL);
// 	}
// 	return (0);
// }
// int	main(void)
// {
// 	char	buffer[1024];
// 	int		fd;
// 	ssize_t	n;

// 	printf("=== Test 1: echo ===\n");
// 	char	*av1[] = {"echo", "Hello from ft_popen!", NULL};
// 	fd = ft_popen("echo", av1, 'r');
// 	if(fd != -1) {
// 		while((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
// 			buffer[n] = '\0';
// 			printf("%s", buffer);
// 		}
// 		close(fd);
// 		wait(NULL);
// 	}
// 	printf("\n=== Test 2: ls -la /tmp (primi 1000 byte) ===\n");
// 	char	*av2[] = {"ls", "-la", "/tmp", NULL};
// 	fd = ft_popen("ls", av2, 'r');
// 	if(fd != -1) {
// 		n = read(fd, buffer, sizeof(buffer) - 1);
// 		if(n > 0) {
// 			buffer[n] = '\0';
// 			printf("%s\n", buffer);
// 		}
// 		close(fd);
// 		wait(NULL);
// 	}
// 	printf("\n=== Test 3: pwd ===\n");
// 	char	*av3[] = {"pwd", NULL};
// 	fd = ft_popen("pwd", av3, 'r');
// 	if(fd != -1) {
// 		while((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
// 			buffer[n] = '\0';
// 			printf("%s", buffer);
// 		}
// 		close(fd);
// 		wait(NULL);
// 	}
// 	printf("\n=== Test 4: whoami ===\n");
// 	char	*av4[] = {"whoami", NULL};
// 	fd = ft_popen("whoami", av4, 'r');
// 	if(fd != -1) {
// 		while((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
// 			buffer[n] = '\0';
// 			printf("%s", buffer);
// 		}
// 		close(fd);
// 		wait(NULL);
// 	}
// 	printf("\n=== Test 5: date ===\n");
// 	char	*av5[] = {"date", NULL};
// 	fd = ft_popen("date", av5, 'r');
// 	if(fd != -1) {
// 		while((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
// 			buffer[n] = '\0';
// 			printf("%s", buffer);
// 		}
// 		close(fd);
// 		wait(NULL);
// 	}
// 	printf("\n");
// 	return (0);
// }
