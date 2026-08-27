

#include <string.h>
#include	"ed.h"

#define		SPACE		0x20

char* ck_spaces(char* str_ptr, char size);
char	tabstops[]	= {	4,4,4,4,8,8,8,8,12,12,12,12,16,16,16,16,\
								20,20,20,20,24,24,24,24,28,28,28,28,\

								32,32,32,32,36,36,36,36,40,40,40,40,\
								44,44,44,44,48,48,48,48,52,52,52,52,\
								56,56,56,56,60,60,60,60,64,64,64,64,\
								68,68,68,68,72,72,72,72,76,76,76,76,\
								79,79,79,79	};

#define	LENGTH	82

#ifdef	DEBUG
static	char	string[LENGTH];
static	char	dest_string[LENGTH];

main()	{
	gets(string);
	print(string);	println("!");
	pack_string(dest_string, string);
	printf("dest_string= %x\n",	dest_string);
	print(dest_string);	println("@");
	puts(dest_string);
}
#endif


/*
	This function expands all tab chars. in	a string with spaces
	It also	removes	any	<cr>'s in the string.
	example:	expand_string(destinationstring, sourcestring);
*/

void expand_string(dest, src)
char* dest;
char* src;
{
	register	char	spaces;
	register	int		c, count;
	register	char	*src_ptr;
	register	char	*dest_ptr;

	src_ptr	= src;	dest_ptr = dest;
	c =	0;

	do	{
		switch(*src_ptr)	{
			case '\t':
				spaces = tabstops[c] - c;
				for(count=0; count < spaces; count++)	{
					*dest_ptr =	0x20;	/* space */
					dest_ptr++;
					c++;
				}
				src_ptr++;
				break;
			case 0x0d:
			case '\n':
				src_ptr++;
				c++;
				break;
			case 0:
				src_ptr++;
				break;
			default:
				*dest_ptr =	*src_ptr;
				dest_ptr++;
				src_ptr++;
				c++;
		}
	} while( *(src_ptr-1) );

	while(c	< LENGTH-1)	{	/* pad with	spaces to end of string	*/
		*dest_ptr =	0x20;
		dest_ptr++;
		c++;
	}
	*dest_ptr =	0;

}


/* This	function compresses	multiple spaces	in a string, replacing
	them with the proper amount	of tab chars.
	example:	pack_string(destinationstring, sourcestring);
*/

void pack_string(dest,src)
char*	dest;
char*	src;
{
	register	int		i;
	register	char	quote;	/* true	if quotation mark was found	*/
	register	char	xcnt;
	register	char	spaces;	/* number of spaces	to next	tabstop	*/
	register	char*	dest_ptr;
	register	char*	src_ptr;
	register	char*	new_ptr;

/*
 * Buffer must match LENGTH. expand_string() produces up to
 * LENGTH-1 display characters plus the terminating NUL.
 */
	char lsrc[LENGTH];

	strcpy(lsrc, src); /* copy source string into temp.	string */
	dest_ptr = dest;	src_ptr	= lsrc;

	for( ; *src_ptr	!= 0; src_ptr++);	/* point src_ptr to	end	of line	*/

	src_ptr--;
	while(*src_ptr == 0x20)	{
		src_ptr--;
	}

	src_ptr++;
	*src_ptr = 0;
	src_ptr	= lsrc;

	quote =	0;
	for(xcnt=0;	*src_ptr &&	*src_ptr !=	0;	)	{
		switch(*src_ptr)	{
			case 0x22:		/* quotation mark? */
				if(quote ==	0)	{
					quote =	1;
					*dest_ptr =	*src_ptr;
					dest_ptr++;	src_ptr++;
					xcnt++;
				}
				else	{  /* set quote	flag as	needed so we can avoid */
					quote =	0;	/* altering	string literals	in the file	*/
					*dest_ptr =	*src_ptr;
					dest_ptr++;	src_ptr++;
					xcnt++;
				}
				break;

			case 0x27:	/* if it's a literal space char. ('	') don't
							change it */
				if(src_ptr[0]==39 && src_ptr[1]==32	&& src_ptr[2]==39)	  {
					for(i=0; i<3; i++)	{
						*dest_ptr =	*src_ptr;
						dest_ptr++;	src_ptr++;
						xcnt++;
					}
				}
				else	{
					*dest_ptr =	*src_ptr;
					dest_ptr++;		src_ptr++;
					xcnt++;
				}
				break;

			case SPACE:
				if(quote)	{ /* don't change text within quotation	marks */
					*dest_ptr =	*src_ptr;
					dest_ptr++;	src_ptr++;
					xcnt++;
					break;
				}
				spaces = tabstops[xcnt]	- xcnt;
				if(	(new_ptr = ck_spaces(src_ptr, spaces)) == 0	)	{
					*dest_ptr =	*src_ptr;
					dest_ptr++;	src_ptr++;
					xcnt++;
				}
				else	{
					*dest_ptr =	'\t';
					dest_ptr++;
					src_ptr	= new_ptr;
					xcnt +=	spaces;
				}
				break;

			default:
				*dest_ptr =	*src_ptr;
				src_ptr++; dest_ptr++;
				xcnt++;
				break;
		}
	}

	*dest_ptr =	0x0d; /* put <cr> at end of	string */
	dest_ptr++;
	*dest_ptr =	0;
}



/*
	Function to	see	if the char	pointer	is pointing	to
	size spaces. If	so,	pointer	to the char. following the spaces
	is returned, else a	null pointer is	returned.
	pass: pointer to string	and	number of spaces to	check for
	for	example:
	if(	(ptr = ck_spaces(str_ptr, size)) ==	NULL)
		printf("no spaces found");
*/

char* ck_spaces(char* str_ptr, char size)
{
	register	int		spc_count;
	register	char*	tmp_ptr;

	tmp_ptr	= str_ptr;
	for(spc_count=1; ! (spc_count>size); spc_count++)	{
		if(*tmp_ptr	!= 0x20)	{
			tmp_ptr	= 0;
			break;
		}
		else
			tmp_ptr++;
	}
	return(tmp_ptr);
}


