// Based on wikipedia implementation :-)
// Could have done one easily... but...
// https://en.wikibooks.org/wiki/C_Programming/stdlib.h/itoa
//
// required for snprintf (mpaland implementation)
// missing on most unixes... and linux as well

void strItoa(char *str, int value)
{
char	c;
int	i, j, sign;

	if ((sign = value) < 0) value = -value;		/* make n positive */

	i = 0;

	str[i++] = '\0';

	do {						/* generate digits in reverse order */
		str[i++] = value % 10 + '0';		/* get next digit */
	} while ((value /= 10) > 0);			/* delete it */

	if (sign < 0) str[i++] = '-';

	i--;
 
	for (j = 0; j <= i; j++,i--) {
		c = str[i];
		str[i] = str[j];
		str[j] = c;
	}
}

// EOF
