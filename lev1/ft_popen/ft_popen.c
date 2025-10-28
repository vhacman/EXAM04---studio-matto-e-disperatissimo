/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_popen.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vhacman <vhacman@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 17:15:35 by vhacman           #+#    #+#             */
/*   Updated: 2025/10/28 16:56:13 by vhacman          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>

/*
**   - fd[2]: pipe creata [0]=lettura, [1]=scrittura
**   - file: path o nome del comando da eseguire
**   - argv: array di argomenti per il comando (terminato con NULL)
**   - type: 'r' per lettura (comando scrive), 'w' per scrittura (comando legge)
  - type == 'r' (Read mode):
**      * Reindirizza stdout del figlio su fd[1] (scrittura della pipe)
**      * Il padre leggerà da fd[0] l'output del comando
  - type == 'w' (Write mode):
**      * Reindirizza stdin del figlio da fd[0] (lettura della pipe)
**      * Il padre scriverà su fd[1] che il figlio leggerà dallo stdin
*/
void	handleChild(int fd[2], const char *file, char *const argv[], char type)
{
	if (type == 'r')
	{
		if(dup2(fd[1], STDOUT_FILENO) == -1)
			exit(1);
	}
	else
	{
		if(dup2(fd[0], STDIN_FILENO) == -1)
			exit(1);
	}
	close(fd[0]);
	close(fd[1]);
	execvp(file, argv);
	exit(1);
}

/*
** Operazioni in base al tipo:
**   - type == 'r' (Read mode):
**      * Chiude fd[1] perché il padre non scrive
**      * Ritorna fd[0] per leggere l'output del comando
**   - type == 'w' (Write mode):
**      * Chiude fd[0] perché il padre non legge
**      * Ritorna fd[1] per scrivere l'input del comando
**
** Ritorno: il file descriptor da usare nel padre (fd[0] o fd[1])
*/
int	handleParent(int fd[2], char type)
{
	if(type == 'r')
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

/*
** ft_popen: Apre un comando come se fosse un file per leggere o scrivere
**           Simula il comportamento della funzione standard POSIX popen()
**
** Parametri:
**   - file: path o nome del comando da eseguire (es: "ls", "/bin/cat")
**   - argv: array di argomenti per il comando, primo elemento è il comando stesso
**           Deve terminare con NULL (es: {"ls", "-la", NULL})
**   - type: 'r' per leggere l'output del comando
**          'w' per scrivere l'input del comando
** Validazioni:
**   1. Controlla che file non sia NULL
**   2. Controlla che argv non sia NULL
**   3. Controlla che argv[0] non sia NULL
**   4. Controlla che type sia 'r' o 'w'

**   1. Crea una pipe con pipe()
**   2. Crea un processo figlio con fork()
**   3. Nel figlio: chiama handleChild() per eseguire il comando
**   4. Nel padre: chiama handleParent() per ritornare il file descriptor corretto
*/
int	ft_popen(const char *file, char *const argv[], char type)
{
	int		fd[2];
	pid_t	pid;

	if(!file || !argv || !argv[0] || (type != 'r' && type != 'w'))
		return (-1);
	if(pipe(fd) == -1)
		return (-1);
	pid = fork();
	if (pid == -1)
	{
		close(fd[0]);
		close(fd[1]);
		return (-1);
	}
	if (pid == 0)
		handleChild(fd, file, argv, type);
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
