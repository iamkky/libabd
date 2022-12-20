#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double strAtof(char *str)
{
char	temp[32];
double	value = 0.0;
double	div = 1.0;
int	signal = 1;
int	esignal = 1;
int	c = 0;

	while(*str=='-' || *str=='+'){
		if(*str=='-') signal = -signal;
		str++;
	}

	while(*str>='0' && *str<='9'){
		value = value * 10.0 + (*str - '0');
		str++;
	}

	if(*str=='.'){
		str++;
		c = 0;
		while(*str>='0' && *str<='9' && c<18){
			temp[c++] = *str++;
			div *= 10.0;
		}
		temp[c] = 0;

		while(*str>='0' && *str<='9') str++;

		value = value + (((double)atol(temp)) / div);
	}


	if(*str=='E' || *str=='e'){
		str++;
		while(*str=='-' || *str=='+'){
			if(*str=='-') esignal = -esignal;
			str++;
		}
		c = 0;
		while(*str>='0' && *str<='9' && c<18){
			temp[c++] = *str++;
		}
		temp[c] = 0;

		while(*str>='0' && *str<='9') str++;

		value = value * pow(10.0, esignal * (double)atol(temp));
	}

	return signal * value;
}
