/*Il seguente codice è stato realizzato da: lzarleng*/

#include <stdio.h> //per printf
#include <stdlib.h> //per exit
#include <ctype.h>//per isdigit

//puntatore globale alla posizione corrente nella stringa di input
char	*s;


/*
** - Riceve come parametro il carattere 'c' che ha causato l'errore
** - Se c != 0: stampa "Unexpected token" seguito dal carattere problematico
** - Se c == 0: significa che la stringa è finita prima del previsto, stampa "Unexpected end of input"
** - Termina il programma con exit(1) per segnalare un errore
*/
//gestisce errori di parsing e termina con exit1
void	error(char c)
{
	if (c) printf("Unexpected token '%c'\n", c);
	else printf("Unexpected end of input\n");
	exit(1);
}

//forward declarations per la ricorsione mutua
int expr();
int term();
int factor();


/*
** Cosa riconosce:
** - Cifre singole da 0 a 9
** - Espressioni tra parentesi: (espressione)
** 1. Controlla se il carattere corrente (*s) è una cifra con isdigit()
**    - Se SI: converte il carattere in numero (*s - '0')
**             incrementa il puntatore s (s++)
**             ritorna il valore numerico
** 2. Se non è una cifra, controlla se è una parentesi aperta '('
**    - Se SI: salta la '(' incrementando s
**             chiama ricorsivamente expr() per valutare l'espressione interna
**             verifica che ci sia la ')' corrispondente
**             se manca la ')': chiama error() e termina
**             se c'è: salta la ')' e ritorna il valore calcolato
** 3. Se non è né cifra né '(': chiama error() perché è un token invalido
** Esempio di chiamate:
** - Input "5" -> ritorna 5
** - Input "(3+2)" -> chiama expr() che ritorna 5
** - Input "x" -> errore, carattere non valido
*/
int	factor()
{
	if (isdigit(*s)) return (*s++ - '0');
	if (*s == '(') {
		s++;
		int val = expr();
		if (*s != ')') error(*s);
		s++;
		return (val);
	}
	else error(*s);
	return (0);
}

/*
** Livello della grammatica: intermedio (tra expr e factor)
** 1. Chiama factor() per ottenere il primo operando (può essere una cifra o un'espressione tra parentesi)
**    - Esempio: in "3*2" ottiene 3
**    - Esempio: in "(1+2)*5" ottiene 5 (risultato di 1+2)
** 2. Entra in un ciclo while che continua finché trova il simbolo '*'
**    - Ogni volta che trova '*':
**      a) Incrementa s per saltare il simbolo '*'
**      b) Chiama factor() per ottenere il prossimo operando
**      c) Moltiplica il valore accumulato (val) per il nuovo operando
** 3. Ritorna il risultato finale quando non ci sono più '*'
** Perché serve questo livello:
** - Gestisce la precedenza degli operatori: la moltiplicazione ha priorità sull'addizione
** - Permette di calcolare correttamente espressioni come "2+3*4" -> 2+(3*4) = 14
** Esempi di esecuzione:
** - Input "5" -> chiama factor() -> ritorna 5
** - Input "2*3" -> factor()=2, trova '*', factor()=3 -> 2*3 = 6
** - Input "2*3*4" -> factor()=2, '*', factor()=3 (val=6), '*', factor()=4 -> 6*4 = 24
*/
int	term() {
	int val = factor();
	while (*s == '*') {
		s++;
		val *= factor();
	}
	return (val);
}

/*
** Livello della grammatica: massimo (punto di partenza del parsing)
** 1. Chiama term() per ottenere il primo operando
**    - term() gestirà eventuali moltiplicazioni nel primo operando
**    - Esempio: in "2*3+5" ottiene 6 (risultato di 2*3)
** 2. Entra in un ciclo while che continua finché trova il simbolo '+'
**    - Ogni volta che trova '+':
**      a) Incrementa s per saltare il simbolo '+'
**      b) Chiama term() per ottenere il prossimo operando (con eventuali moltiplicazioni)
**      c) Somma il valore accumulato (val) al nuovo operando
** 3. Ritorna il risultato finale quando non ci sono più '+'
** Perché è il livello più alto:
** - È la prima funzione chiamata dal main() per iniziare il parsing
** - Delega a term() la gestione delle moltiplicazioni, rispettando così la precedenza
** - Struttura la grammatica in modo ricorsivo discendente
**
** Esempi di esecuzione:
** - Input "5" -> chiama term() che chiama factor() -> ritorna 5
** - Input "2+3" -> term()=2, trova '+', term()=3 -> 2+3 = 5
** - Input "2+3*4" -> term()=2, '+', term()=12 (3*4) -> 2+12 = 14
** - Input "2*3+4*5" -> term()=6 (2*3), '+', term()=20 (4*5) -> 6+20 = 26
*/
int	expr() {
	int val = term();
	while (*s == '+') {
		s++;
		val += term();
	}
	return (val);
}

/*
** 1. Validazione input:
**    - Verifica che argc == 2 (nome programma + 1 argomento)
**    - Se argc != 2: ritorna 1 (errore) senza stampare nulla
** 2. Inizializzazione:
**    - Assegna argv[1] al puntatore globale 's'
**    - 's' punterà al primo carattere dell'espressione da valutare
** 3. Parsing ed esecuzione:
**    - Chiama expr() che avvia il parsing ricorsivo dell'espressione
**    - expr() restituisce il valore calcolato dell'intera espressione
** 4. Validazione finale:
**    - Controlla se *s != '\0' (ci sono caratteri rimasti dopo il parsing)
**    - Se SI: chiama error(*s) perché ci sono caratteri extra non validi
**    - Se NO: l'espressione è stata completamente parsata correttamente
** 5. Output:
**    - Stampa il risultato seguito da newline
**    - Ritorna 0 per indicare successo
*/
//esegue parsing e stampa risultato, exit code 0 = successo, exit code 1 = errore
int	main(int argc, char **argv)
{
	if (argc != 2) return (1);
	s = argv[1];
	int	result = expr();
	if (*s) error(*s);
	printf("%d\n", result);
	return (0);
}
