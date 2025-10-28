#include "argo.h"

int	parse_int(json *dst, FILE *stream)
{
	int c = peek(stream);
	int	num;
	int	matched;

	if (c == EOF || (!isdigit(c) && c != '-'))
		return (unexpected(stream), -1);
	matched = fscanf(stream, "%d", &num);
	if(matched != 1)
		return (unexpected(stream), -1);
	dst->type = INTEGER;
	dst->integer = num;
	return (1);
}

int	parse_string(json *dst, FILE *stream)
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
			if (escaped == '"' && escaped != '\\')
				return (unexpected(stream), free(buffer), -1);
			c = escaped;
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

int	parse_map(json *dst, FILE *stream)
{
	pair	*items = NULL;
	size_t	size = 0;
	json	key;

	if (!expect(stream, '{'))
		return (-1);
	while(!accept(stream, '}'))
	{
		items = realloc(items, sizeof(pair) * (size + 1));
		if (!items)
			return (-1);
		if(parse_string (&key, stream) == -1)
			return (free (items), -1);
		if (!expect(stream, ':'))
			return (free(key.string), free(items), -1);
		if (parse_value (&items[size].value, stream) == -1)
			return (free(key.string), free(items), -1);
		items[size].key = key.string;
		size++;
		if (!accept(stream, ',') && peek(stream) != '}')
			return (unexpected(stream), free(items), -1);
	}
	dst->type = MAP;
	dst->map.size = size;
	dst->map.data = items;
	return (1);
}

int	parse_value(json *dst, FILE *stream)
{
	int	c = peek (stream);

	if (c == '"')
		return (parse_string(dst, stream));
	else if (c == '{')
		return (parse_map(dst, stream));
	else if (isdigit(c) || c == '-')
		return (parse_int(dst, stream));
	else
		return (unexpected(stream), -1);
}

int	argo (json *dst, FILE *stream)
{
	if (parse_value(dst, stream) != 1)
		return (-1);
	if (peek(stream) != EOF)
		return (unexpected(stream), free_json(*dst), -1);
	return (1);
}


/* ========================================================================== */
/*                         FUNZIONE MAIN FORNITA                              */
/* ========================================================================== */
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
	printf("\n");
	free_json(file);
	fclose(stream);
	return 0;
}

