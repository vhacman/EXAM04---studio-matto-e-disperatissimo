#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>

// Gestisce il processo figlio dopo il fork
void	handleChild(int fd[2], const char *file, char *const argv[], char type)
{
	// Se tipo è 'r' (read), redirige stdout verso la pipe
	// così l'output del comando va nella pipe
	if (type == 'r')
	{
		// Duplica il descrittore di scrittura della pipe su stdout
		if(dup2(fd[1], STDOUT_FILENO) == -1)
			exit(1);
	}
	// Se tipo è 'w' (write), redirige stdin dalla pipe
	// così il comando legge input dalla pipe
	else
	{
		// Duplica il descrittore di lettura della pipe su stdin
		if(dup2(fd[0], STDIN_FILENO) == -1)
			exit(1);
	}
	// Chiude entrambi i descrittori originali della pipe
	// (non servono più dopo il dup2)
	close(fd[0]);
	close(fd[1]);
	// Esegue il comando sostituendo il processo figlio
	execvp(file, argv);
	// Se execvp fallisce, esce con errore
	exit(1);
}

// Gestisce il processo padre dopo il fork
int	handleParent(int fd[2], char type)
{
	// Se tipo è 'r' (read), il padre leggerà dalla pipe
	if(type == 'r')
	{
		// Chiude il lato scrittura (non serve al padre)
		close(fd[1]);
		// Ritorna il descrittore di lettura
		return (fd[0]);
	}
	// Se tipo è 'w' (write), il padre scriverà nella pipe
	else
	{
		// Chiude il lato lettura (non serve al padre)
		close(fd[0]);
		// Ritorna il descrittore di scrittura
		return (fd[1]);
	}
}

// Crea un processo figlio che esegue un comando e ritorna un file descriptor
// per comunicare con esso tramite pipe
int	ft_popen(const char *file, char *const argv[], char type)
{
	int		fd[2];  // Array per i descrittori della pipe
	pid_t	pid;    // ID del processo

	// Validazione parametri: controlla che i parametri siano validi
	// e che type sia 'r' (read) o 'w' (write)
	if(!file || !argv || !argv[0] || (type != 'r' && type != 'w'))
		return (-1);
	// Crea la pipe: fd[0] per leggere, fd[1] per scrivere
	if(pipe(fd) == -1)
		return (-1);
	// Crea un processo figlio
	pid = fork();
	if (pid == -1)
	{
		// Se fork fallisce, chiude la pipe e ritorna errore
		close(fd[0]);
		close(fd[1]);
		return (-1);
	}
	// Codice eseguito solo dal processo figlio (pid == 0)
	if (pid == 0)
		handleChild(fd, file, argv, type);
	// Codice eseguito solo dal processo padre
	// Ritorna il descrittore appropriato per comunicare col figlio
	return (handleParent(fd, type));
}

// int	main(void)
// {
// 	int		fd;
// 	char	buffer[1024];
// 	int		bytes;

// 	fd = ft_popen("ls", (char *const []){"ls", NULL}, 'r');
// 	if (fd == -1)
// 	{
// 		printf("ft_popen failen\n");
// 		return (1);
// 	}
// 	while((bytes = read(fd, buffer, sizeof(buffer) - 1)) > 0)
// 	{
// 		buffer[bytes] = '\0';
// 		printf("%s", buffer);
// 	}
// 	close(fd);
// 	return(0);
// }
