#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
/*
  Variabili:
	- pid: pid del processo figlio
	- fd[2]: file descriptors per la pipe
	- prev_fd: file descriptor per l'input del comando corrente
	- status: stato di terminazione del processo figlio
	- exit_code: codice di uscita complessivo
	- i: indice del comando corrente
  Flusso:
	- Crea la pipe se non è l'ultimo comando.
	- Errore pipe: se accade, chiude prev_fd (se valido) e ritorna 1.
	- Crea un nuovo processo con fork.
	- Errore fork:
		se la pipe esiste, chiude fd[0] e fd[1]; chiude prev_fd (se valido)
		e ritorna 1.
	- Processo figlio:
		* Se non è il primo comando, duplica prev_fd su STDIN; poi chiude
		  prev_fd.
		* Se non è l'ultimo comando, chiude il lato di lettura della pipe,
		  duplica fd[1] su STDOUT e chiude fd[1].
		* Esegue il comando con execvp; se fallisce, termina con exit(1).
	- Processo padre:
		* Chiude l'fd di input del comando precedente, se presente.
		* Se non è l'ultimo comando, chiude il lato di scrittura della
		  pipe e salva fd[0] in prev_fd per il prossimo comando.
	- Attende la terminazione di tutti i processi figli; se uno termina
	  con errore (WIFEXITED e WEXITSTATUS != 0), imposta exit_code a 1.
	- Ritorna exit_code come codice di uscita complessivo.
*/
int	picoshell(char **cmds[])
{
	pid_t	pid;
	int		fd[2];
	int		prev_fd = -1;
	int		status;
	int		exit_code = 0;
	int		i = 0;
	while (cmds[i])
	{
		if (cmds[i + 1] && (pipe(fd) == -1)) {
			if (prev_fd != -1) close(prev_fd);
			return (1);
		}
		pid = fork();
		if (pid == -1) {
			if (cmds[i + 1]) {
				close(fd[0]);
				close(fd[1]);
			}
			if (prev_fd != -1) close(prev_fd);
			return (1);
		}
		if (pid == 0) {
			if (prev_fd != -1) {
				if (dup2(prev_fd, STDIN_FILENO) == -1) exit(1);
				close(prev_fd);
			}
			if (cmds[i + 1]) {
				close(fd[0]);
				if (dup2(fd[1], STDOUT_FILENO) == -1) exit(1);
				close(fd[1]);
			}
			execvp(cmds[i][0], cmds[i]);
			exit(1);
		}
		if (prev_fd != -1) close(prev_fd);
		if (cmds[i + 1]) {
			close(fd[1]);
			prev_fd = fd[0];
		}
		i++;
	}
	while (wait(&status) != -1){
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0) exit_code = 1;
	}
	return (exit_code);
}

// int	main(void)
// {
// 	// Test 1: ls | grep picoshell
// 	printf("=== Test 1: ls | grep picoshell ===\n");
// 	char *cmd1[] = {"ls", NULL};
// 	char *cmd2[] = {"grep", "picoshell", NULL};
// 	char **cmds1[] = {cmd1, cmd2, NULL};
// 	int result1 = picoshell(cmds1);
// 	printf("Result: %d\n\n", result1);

// 	// Test 2: echo 'squalala' | cat | sed 's/a/b/g'
// 	printf("=== Test 2: echo 'squalala' | cat | sed 's/a/b/g' ===\n");
// 	char *cmd3[] = {"echo", "squalala", NULL};
// 	char *cmd4[] = {"cat", NULL};
// 	char *cmd5[] = {"sed", "s/a/b/g", NULL};
// 	char **cmds2[] = {cmd3, cmd4, cmd5, NULL};
// 	int result2 = picoshell(cmds2);
// 	printf("Result: %d\n\n", result2);

// 	// Test 3: echo 'hello' | wc -c
// 	printf("=== Test 3: echo 'hello' | wc -c ===\n");
// 	char *cmd6[] = {"echo", "hello", NULL};
// 	char *cmd7[] = {"wc", "-c", NULL};
// 	char **cmds3[] = {cmd6, cmd7, NULL};
// 	int result3 = picoshell(cmds3);
// 	printf("Result: %d\n\n", result3);

// 	// Test 4: pwd | cat
// 	printf("=== Test 4: pwd | cat ===\n");
// 	char *cmd8[] = {"pwd", NULL};
// 	char *cmd9[] = {"cat", NULL};
// 	char **cmds4[] = {cmd8, cmd9, NULL};
// 	int result4 = picoshell(cmds4);
// 	printf("Result: %d\n\n", result4);

// 	// Test 5: cat /etc/hostname | tr a-z A-Z
// 	printf("=== Test 5: cat /etc/hostname | tr a-z A-Z ===\n");
// 	char *cmd10[] = {"cat", "/etc/hostname", NULL};
// 	char *cmd11[] = {"tr", "a-z", "A-Z", NULL};
// 	char **cmds5[] = {cmd10, cmd11, NULL};
// 	int result5 = picoshell(cmds5);
// 	printf("Result: %d\n\n", result5);

// 	return (0);
// }
