/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   picoshell1.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vhacman <vhacman@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 17:17:00 by vhacman           #+#    #+#             */
/*   Updated: 2025/10/29 12:01:35 by vhacman          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

/*
** execute_child: Esegue il comando nel processo figlio
**
** Parametri:
**   - cmd: array di stringhe con il comando e i suoi argomenti
**   - prev: file descriptor dello stdin da un comando precedente (-1 se nessuno)
**   - fd[2]: pipe per collegare al comando successivo [0]=lettura, [1]=scrittura
**   - has_next_cmd: flag che indica se c'è un comando successivo
**
** Operazioni:
**   1. Se esiste un comando precedente (prev != -1):
**      - Reindirizza lo stdin dal pipe precedente usando dup2()
**      - Chiude il vecchio file descriptor
**   2. Se esiste un comando successivo (has_next_cmd):
**      - Reindirizza lo stdout sulla pipe per il prossimo comando
**   3. Chiude tutti i file descriptor non utilizzati (prevenzione di leak)
**   4. Esegue il comando con execvp() che sostituisce il processo figlio
**   5. Se execvp() fallisce, esce con codice 1
*/
void	execute_child(char **cmd, int prev, int fd[2], int has_next_cmd)
{
	if (prev != -1)
	{
		if (dup2(prev, STDIN_FILENO) == -1)
			exit(1);
		close(prev);
	}
	if (has_next_cmd)
	{
		if (dup2(fd[1], STDOUT_FILENO) == -1)
			exit(1);
	}
	close(fd[0]);
	close(fd[1]);
	execvp(cmd[0], cmd);
	exit(1);
}

/*
** fork_execute: Crea un nuovo processo figlio e lo configura per eseguire il comando
**
** Parametri:
**   - cmd: array di stringhe con il comando e i suoi argomenti
**   - prev: puntatore al file descriptor dello stdin precedente (viene aggiornato)
**   - fd[2]: pipe creata per il comando successivo
**   - has_next_cmd: flag che indica se c'è un comando successivo
**
** Operazioni (nel processo padre):
**   1. Crea un fork() e controlla se ha successo
**   2. Nel processo figlio (pid == 0): chiama execute_child()
**   3. Nel processo padre:
**      - Chiude il precedente file descriptor di input se era aperto
**      - Salva il file descriptor di lettura della pipe per il prossimo comando
**      - Chiude il file descriptor di scrittura (non serve al padre)
**
** Ritorno: 0 in caso di successo, 1 in caso di errore del fork()
*/
int	fork_execute(char **cmd, int *prev, int fd[2], int has_next_cmd)
{
	pid_t pid = fork();
	if (pid == -1)
		return (close(fd[0]), close(fd[1]), 1);
	if (pid == 0)
		execute_child(cmd, *prev, fd, has_next_cmd);
	if (*prev != -1)
		close(*prev);

	/* Salva fd[0] per il prossimo comando, o -1 se è l'ultimo */
	*prev = has_next_cmd ? fd[0] : -1;

	if (has_next_cmd)
		close(fd[1]);
	return (0);
}

/*
** picoshell: Gestisce l'esecuzione di una pipeline di comandi
**
** Parametri:
**   - cmds: array di array di stringhe (ogni array è un comando con i suoi argomenti)
**           L'ultimo elemento deve essere NULL per marcare la fine
**
** Operazioni:
**   1. Inizializza fd e prev (prev memorizza il file descriptor del comando precedente)
**   2. Per ogni comando in cmds:
**      - Se non è l'ultimo comando, crea una pipe() per collegarlo al prossimo
**      - Chiama fork_execute() per creare e eseguire il processo figlio
**   3. Attende che tutti i processi figli terminino con wait()
**   4. Chiude il file descriptor di input rimasto (se esiste)
**   5. Ritorna 0 se tutto va bene, 1 se ci sono errori
*/
int	picoshell(char ***cmds)
{
	int	fd[2];
	int	prev = -1;
	int	i = -1;

	fd[0] = -1;
	fd[1] = -1;
	for (i = 0; cmds[i] != NULL; i++)
	{
		if (cmds[i + 1] != NULL)
			pipe(fd);
		if (fork_execute(cmds[i], &prev, fd, cmds[i + 1]) != NULL)
			return (1);
	}
	while (wait(NULL) > 0);
	if (prev >= 0)
		close(prev);
	return (0);
}

// int	main()
// {
// 	char	*cmd1[] = {"ls", "-la", NULL};
// 	char	*cmd2[] = {"grep", ".c", NULL};
// 	char	*cmd3[] = {"wc", "-l", NULL};
// 	char	*cmd4[] = {"echo", "Hello World!", NULL};
// 	char	*cmd5[] = {"nonexisting", NULL};

// 	char	**pipe1[] = {cmd1, cmd2, cmd3, NULL};
// 	char	**pipe2[] = {cmd4, NULL};
// 	char	**pipe3[] = {cmd5, NULL};
// 	char	**pipe4[] = {cmd1, NULL};
// 	printf("Running pipe: ls -la | grep .c | wc -l\n");
// 	printf("Pipe returned: %d\n\n", picoshell(pipe1));
// 	printf("Running pipe: echo \"Hello World!\"\n");
// 	printf("Pipe returned: %d\n\n", picoshell(pipe2));
// 	printf("Running pipe: nonexisting\n");
// 	printf("Pipe returned: %d\n\n", picoshell(pipe3));
// 	printf("Running pipe: ls -la\n");
// 	printf("Pipe returned: %d\n", picoshell(pipe4));
// 	return (0);
// }
