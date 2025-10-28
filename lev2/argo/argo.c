/*
** ============================================================================
** ARGO - JSON Parser Minimale
** ============================================================================
** Questo programma implementa un parser JSON semplificato che supporta:
** - Interi (INTEGER)
** - Stringhe (STRING)
** - Mappe/Oggetti (MAP)
**
** Il parser legge da file, costruisce una struttura dati JSON in memoria
** e poi la serializza nuovamente in formato JSON compatto.
** ============================================================================
*/

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* ========================================================================== */
/*                       STRUTTURE DATI FORNITE                               */
/* ========================================================================== */

/*
** Struttura principale per rappresentare un valore JSON.
** Usa un'union per risparmiare memoria: ogni istanza può essere
** solo uno dei tre tipi (MAP, INTEGER, STRING).
*/
typedef struct	json {
	enum {
		MAP,
		INTEGER,
		STRING
	} type;
	union {
		struct {
			struct pair	*data;
			size_t		size;
		} map;
		int		integer;
		char	*string;
	};
}	json;

/*
** Coppia chiave-valore per rappresentare un elemento di una mappa JSON.
** La chiave è sempre una stringa, il valore può essere qualsiasi tipo JSON.
*/
typedef struct	pair {
	char	*key;
	json	value;
}	pair;

int	parse_value(json *dst, FILE *stream);

/* ========================================================================== */
/*                         FUNZIONI UTILITY FORNITE                           */
/* ========================================================================== */
int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

/* ========================================================================== */
/*                      FUNZIONI DA IMPLEMENTARE                              */
/* ========================================================================== */
/*
** parse_int: Legge e parsa un numero intero dallo stream
** @dst: Dove salvare il valore parsato
** @stream: Lo stream da cui leggere
** @return: 1 se successo, -1 se errore
**
** Legge un numero intero (può essere negativo con '-').
** Usa fscanf per la conversione. Controlla prima che il carattere
** successivo sia una cifra o un meno.
*/
int	parse_int(json *dst, FILE *stream)
{
	int	c = peek(stream);
	int	num;
	int	matched;

	if (c == EOF || (!isdigit(c) && c != '-'))
		return (unexpected(stream), -1);
	matched = fscanf(stream, "%d", &num);
	if (matched != 1)
		return (unexpected(stream), -1);
	dst->type = INTEGER;
	dst->integer = num;
	return (1);
}

/*
** parse_str: Legge e parsa una stringa JSON dallo stream
** @dst: Dove salvare la stringa parsata
** @stream: Lo stream da cui leggere
** @return: 1 se successo, -1 se errore
**
** Le stringhe JSON sono racchiuse tra virgolette: "ciao mondo"
** Supporta escape sequences: \" e \\
** Alloca dinamicamente un buffer che cresce se necessario.
*/
int	parse_str(json *dst, FILE *stream)
{
	if (!expect(stream, '"'))
		return (-1);
	size_t	capacity = 32;
	size_t	len = 0;
	char	*buffer = malloc(capacity);
	if (!buffer)
		return (-1);
	while (1)
	{
		int	c = getc(stream);
		if (c == EOF)
			return (unexpected(stream), free(buffer), -1);
		if (c == '"')
		{
			buffer[len] = '\0';
			dst->type = STRING;
			dst->string = buffer;
			return (1);
		}
		if (c == '\\')
		{
			int	escaped = getc(stream);
			if (escaped == EOF)
				return (unexpected(stream), free(buffer), -1);
			if (escaped == '"' || escaped == '\\')
				c = escaped;
			else
			{
				ungetc(escaped, stream);
				return (unexpected(stream), free(buffer), -1);
			}
		}
		if (len + 1 >= capacity)
		{
			capacity *= 2;
			char	*new_buffer = realloc(buffer, capacity);
			if (!new_buffer)
				return (free(buffer), -1);
			buffer = new_buffer;
		}
		buffer[len++] = c;
	}
}

/*
** free_items: Libera un array parziale di coppie chiave-valore
** @items: Array di coppie da liberare
** @size: Numero di elementi nell'array
**
** Funzione helper per parse_map. Usata per cleanup in caso di errore
** durante il parsing di una mappa.
*/
static void	free_items(pair *items, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		free(items[i].key);
		free_json(items[i].value);
	}
	free(items);
}

/*
** parse_map: Legge e parsa una mappa (oggetto) JSON dallo stream
** @dst: Dove salvare la mappa parsata
** @stream: Lo stream da cui leggere
** @return: 1 se successo, -1 se errore
**
** Struttura: { "chiave1": valore1, "chiave2": valore2, ... }
** Le chiavi sono SEMPRE stringhe.
** I valori possono essere qualsiasi tipo JSON (int, string, o altra mappa).
** Gestisce anche mappe vuote: {}
*/
int parse_map(json *dst, FILE *stream)
{
	pair	*items = NULL;
	size_t	size = 0;
	json	key;
	
	/* Expect opening brace for JSON object */
	if (!expect(stream, '{'))
		return (-1);
	/* Parse key-value pairs until closing brace */
	while (!accept(stream, '}'))
	{
		/* Expand array to accommodate new pair */
		items = realloc(items, sizeof(pair) * (size + 1));
		if (!items)
			return (-1);
		/* Parse the key (must be a string) */
		if (parse_str(&key, stream) == -1)
			return (free_items(items, size), -1);
		/* Expect colon separator between key and value */
		if (!expect(stream, ':'))
			return (free(key.string), free_items(items, size), -1);
		/* Parse the value */
		if (parse_value(&items[size].value, stream) == -1)
			return (free(key.string), free_items(items, size), -1);
		/* Store key and increment size */
		items[size].key = key.string;
		size++;
		/* Handle comma separator between pairs */
		if (peek(stream) == ',')
		{
			getc(stream);
			int next = peek(stream);
			ungetc(',', stream);
			/* Trailing comma before closing brace is invalid */
			if (next == '}')
				return (free_items(items, size), unexpected(stream), -1);
			accept(stream, ',');
		}
		/* If not comma, must be closing brace */
		else if (peek(stream) != '}')
			return (free_items(items, size), unexpected(stream), -1);
	}
	/* Store parsed map in destination */
	dst->type = MAP;
	dst->map.size = size;
	dst->map.data = items;
	return (1);
}

/*
** parse_value: Determina il tipo di valore e chiama il parser appropriato
** @dst: Dove salvare il valore parsato
** @stream: Lo stream da cui leggere
** @return: 1 se successo, -1 se errore
**
** Funzione dispatcher che guarda il primo carattere per decidere
** quale parser chiamare:
** - " → stringa
** - { → mappa
** - cifra o - → intero
*/
int	parse_value(json *dst, FILE *stream)
{
	int	c = peek(stream);

	if (c == '"')
		return (parse_str(dst, stream));
	else if (c == '{')
		return (parse_map(dst, stream));
	else if (isdigit(c) || c == '-')
		return (parse_int(dst, stream));
	else
		return (unexpected(stream),-1);
}

/*
** argo: Funzione principale di parsing JSON
** @dst: Dove salvare il JSON parsato
** @stream: Lo stream da cui leggere
** @return: 1 se successo, -1 se errore
**
** Parsa un valore JSON completo e verifica che non ci sia
** altro contenuto dopo di esso. Se c'è contenuto extra,
** è un errore (il JSON deve essere completo e unico).
*/
int	argo(json *dst, FILE *stream)
{
	if (parse_value(dst, stream) != 1)
		return (-1);
	/* Verifica che non ci sia nient'altro dopo il JSON */
	if (peek(stream) != EOF)
		return (unexpected(stream), free_json(*dst),-1);
	return (1);
}

/* ========================================================================== */
/*                         FUNZIONE MAIN FORNITA                              */
/* ========================================================================== */
/*cosi ha dei leak perche manca fclose(stream), free_json(file)*/
int	main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	char *filename = argv[1];
	FILE *stream = fopen(filename, "r");
	if (!stream)
		return 1;
	json	file;
	if (argo(&file, stream) != 1)
	{
		fclose(stream);
		return 1;
	}
	serialize(file);
	// free_json(file);
	// fclose(stream);
	printf("\n");
	return 0;
}