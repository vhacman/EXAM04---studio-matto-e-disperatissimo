#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

/*    Flusso logico:
- Verifica che il puntatore alla funzione non sia NULL.
- Imposta il gestore per SIGALRM tramite struct sigaction.
- Esegue fork() per creare un processo figlio.
- Nel processo figlio:
* Esegue la funzione f().
* Termina con exit(0).
- Nel processo padre:
* Se timeout > 0, imposta un allarme con alarm(timeout).
		* Attende il figlio con waitpid(pid, &status, 0).
	- Se waitpid è interrotto da un segnale (errno == EINTR):
		* Uccide il figlio con SIGKILL.
		* Attende la sua terminazione.
		* Disattiva l’allarme.
		* Se verbose, stampa un messaggio di timeout.
		* Ritorna 0.
		- Se waitpid fallisce per altri motivi, disattiva l’allarme e ritorna -1.
		- Disattiva l’allarme una volta completata l’attesa.
		- Se il figlio termina normalmente (WIFEXITED):
		* Se WEXITSTATUS == 0:
		stampa "Nice function!" se verbose e ritorna 1.
		* Altrimenti:
		stampa il codice di uscita e ritorna 0.
		- Se il figlio termina per un segnale (WIFSIGNALED):
		* Ottiene il numero del segnale con WTERMSIG.
		* Stampa il nome del segnale (strsignal(sig)) se verbose.
		* Ritorna 0.
		- Ritorna -1 in caso di stato non previsto.
		*/

void alarm_handler(int sig) {
	(void)sig;
}
int	sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
	struct sigaction	sa;
	pid_t				pid;
	int					status;

	if (!f) return (-1);
	sa.sa_handler = alarm_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);
	pid = fork();
	if (pid == -1) return (-1);
	if (pid == 0) {
		f();
		exit(0);
	}
	if (timeout > 0) alarm(timeout);
	if (waitpid(pid, &status, 0) == -1) {
		if (errno == EINTR) {
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			alarm(0);
			if (verbose) printf("Bad function: timed out after %u seconds\n", timeout);
			return (0);
		}
		alarm(0);
		return (-1);
	}
	alarm(0);
	if (WIFEXITED(status)){
		if (WEXITSTATUS(status) == 0) {
			if (verbose) printf("Nice function!\n");
			return (1);
		} else {
			if (verbose) printf("Bad function: exited with code %d\n", WEXITSTATUS(status));
			return (0);
		}
	}
	if (WIFSIGNALED(status)){
		int	sig = WTERMSIG(status);
		if (verbose) printf("Bad function: %s\n", strsignal(sig));
		return (0);
	}
	return (-1);
}

 //TESTING
// void nice_ft()
// {
// 	return ;
// }

// void bad_ft_exit()
// {
// 	exit (1);
// }

// void bad_ft_segfault()
// {
// 	char *str = NULL;
// 	str[2] = 'a';
// }

// void bad_ft_timeout()
// {
// 	while (1)
// 		;
// }

// void bad_ft_sigkill()
// {
// 	sleep(5);
// }

// int main()
// {
// 	int res;

// 	res = sandbox(nice_ft, 5, true);
// 	printf("res is %d\n", res);
// 	res = sandbox(bad_ft_exit, 5, true);
// 	printf("res is %d\n", res);
// 	res = sandbox(bad_ft_segfault, 5, true);
// 	printf("res is %d\n", res);
// 	res = sandbox(bad_ft_timeout, 2, true);
// 	printf("res is %d\n", res);
// 	res = sandbox(bad_ft_sigkill, 2, true);
// 	printf("res is %d\n", res);
// }
