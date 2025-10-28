#ifndef ARGO_H
#define ARGO_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

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

/*GIVEN*/
int		peek(FILE *stream);
void	unexpected(FILE *stream);
int		accept(FILE *stream, char c);
int		expect(FILE *stream, char c);
void	free_json(json j);
void	serialize(json j);

/*WRITTEN*/
int	parse_str(json *dst, FILE *stream);
int	parse_int(json *dst, FILE *stream);
int	parse_map(json *dst, FILE *stream);
int	parse_value (json *dst, FILE *stream);
int	argo(json *dst, FILE *stream);
#endif
