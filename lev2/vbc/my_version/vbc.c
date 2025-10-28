#include "vbc.h"

char *s;

int is_whitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
}

void skip_whitespace()
{
	while(is_whitespace(*s))
		s++;
}
int ft_product()
{
	int	a = ft_factor();
	while(*s == '*')
	{
		s++;
		a = a * ft_factor();
	}
	return(a);
}

int ft_sum()
{
	int sum = ft_product();
	while(*s == '+')
	{
		s++;
		sum += ft_product();
	}
	return(sum);
}

int	ft_factor()
{
	int	n;
	
	skip_whitespace();
	if(isdigit(*s))
		return(*s++ - '0');
	if(*s == '(')
	{
		s++;
		n = ft_sum();
		skip_whitespace();
		if(*s == ')')
			s++;
		return (n);
	}
	return (0);
}

int check_input(char *str)
{
	int par = 0;
	int i = 0;
	char prev = 0;
	while(str[i])
	{
		if(is_whitespace(str[i]))
		{
			i++;
			continue;
		}
		par += (str[i] == '(') - (str[i] == ')');
		if(par < 0)
			return(unexpected(')'), 1);
		if(isdigit(str[i]) && isdigit(prev))
			return(unexpected(str[i]), 1);
		prev = str[i];
		i++;
	}
	if(par > 0)
		return(unexpected('('), 1);
	if(prev == '+' || prev == '*')
		return(unexpected(0), 1);
	return(0);
}

int main(int argc, char **argv)
{
	if(argc != 2)
		return(1);
	if(check_input(argv[1]))
		return(1);
	s = argv[1];
	int res = ft_sum();
	printf("%d\n", res);
	return(0);
}