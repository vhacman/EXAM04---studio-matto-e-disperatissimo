/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sandbox.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vhacman <vhacman@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 17:36:20 by vhacman           #+#    #+#             */
/*   Updated: 2025/10/23 17:58:37 by vhacman          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** Gestisce l'uscita normale di un processo figlio.
** Controlla il codice di uscita e stampa un messaggio appropriato.
** - Se il codice è 0: funzione eseguita con successo, ritorna 1
** - Se il codice è != 0: funzione ha avuto un errore, ritorna 0
** - Se verbose è true: stampa un messaggio di feedback
*/
int	handle_exit(int status, bool verbose)
{
	int	exit_code = WEXITSTATUS(status);

	if (exit_code == 0)
	{
		if (verbose)
			printf("Nice function!\n");
		return (1);
	}
	if (verbose)
		printf("Bad function: exited with code %d\n", exit_code);
	return (0);
}

/*
** Gestisce la terminazione di un processo figlio per segnale.
** Controlla quale segnale ha terminato il processo.
** - Se il segnale è SIGALRM: timeout scaduto, stampa il tempo limite
** - Per altri segnali: stampa il nome del segnale ricevuto
** - Ritorna sempre 0 perché un segnale indica un fallimento
** - Se verbose è true: stampa un messaggio di feedback
*/
int	handle_signal(int status, unsigned int timeout, bool verbose)
{
	int	sig = WTERMSIG(status);

	if (verbose)
	{
		if (sig == SIGALRM)
			printf("Bad function: timed out after %u seconds\n", timeout);
		else
			printf("Bad function: %s\n", strsignal(sig));
	}
	return (0);
}

/*
** Esegue una funzione in un processo isolato (sandbox) con timeout.
** - f: puntatore alla funzione da eseguire
** - timeout: secondi massimi per l'esecuzione (0 = nessun timeout)
** - verbose: se true, stampa messaggi di feedback
**
** Comportamento:
** - Crea un processo figlio con fork()
** - Nel figlio: imposta un timeout con alarm() e esegue la funzione
** - Nel padre: aspetta il figlio e analizza lo stato di uscita
** - Gestisce tre scenari: uscita normale, segnali, errori di fork/wait
**
** Valori di ritorno:
** - 1: successo (funzione eseguita con exit code 0)
** - 0: fallimento (timeout, segfault, exit code != 0, o segnale ricevuto)
** - -1: errore critico (fork o waitpid fallito, o f è NULL)
*/
int	sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
	pid_t	pid;
	int		status;

	if (!f)
		return (-1);
	pid = fork();
	if (pid == -1)
		return (-1);
	if (pid == 0)
	{
		alarm(timeout);
		f();
		exit(0);
	}
	if (waitpid(pid, &status, 0) == -1)
		return (-1);
	if (WIFEXITED(status))
		return (handle_exit(status, verbose));
	if (WIFSIGNALED(status))
		return (handle_signal(status, timeout, verbose));
	return (-1);
}

// //TESTING
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

