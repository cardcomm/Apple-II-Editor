
#include	"ed.h"
#include <stdio.h>

#define	TRUE	1
#define	FALSE	0

extern	char	tabstops[];		/* defined in the "strings.c" source file */

void message();
void drawscrn();
int waitkey();

int search_string();
int check_match();
int calc_offset();
void debug();

#ifdef	DEBUG
main()		{
static	char	str[] =	"This is a test string. Is it working?\n";
static	char	pat[] =	"is";

	search_string(str, pat);
}
#endif

/*
	handle the Find	command
*/

void do_search(fip)
struct fileinfo *fip;
{
    register char *t;
    static struct inpstat *sp;
    static char pattern[50];
    register int search_line;
    register int offset;

    message("Enter search string:");

    sp = editline(
        pattern,
        (sizeof(pattern) - 2)
    );

    if(sp->esc_stat)
        return;

    /*
     * editline() pads the input with spaces.
     * Terminate after the last non-space character.
     */
    for(t = pattern + (sizeof(pattern) - 2);
        t >= pattern && *t == 0x20;
        t--)
        ;

    t++;
    *t = 0;

    /*
     * Don't search for an empty string.
     */
    if(pattern[0] == 0)
        return;

    /*
     * Search the current line and then each following line
     * until EOF.
     */
    search_line = fip->edline;

    while(search_line < max_lines &&
          fip->index[search_line] != 0) {

        offset = search_string(
            fip->index[search_line],
            pattern
        );

	if(offset >= 0) {
	
		/*
		 * If the matching line is already visible, keep the
		 * viewport where it is and move only the cursor.
		 */
		if(search_line >= fip->topline &&
		   search_line <= fip->botline) {
	
			fip->dist_dwn = search_line - fip->topline;
		}
	
		fip->edline = search_line;
		fip->x = offset;
	
		drawscrn();
		return;
	}

        search_line++;
    }

    message("Not found.");
}

/*
	Function to	look for a character pattern within	a string. The string
	should be terminated by	a newline char.

	Pass the following parms:
	1. pointer to string to	examine.
	2. pointer to pattern to look for.

	Returns	offset into	the	string of the first	char. of found match
	 if	no match, -1	is returned
*/

int search_string(string, pattern)
char *string;
char *pattern;
{
    register char *str_start;

    str_start = string;

    while(*string != '\r' && *string != 0) {

        if(*string == *pattern ||
           *string == (*pattern)-32) {

            if(check_match(string, pattern)) {

                return(
                    calc_offset(str_start, string)
                );
            }
        }

        string++;
    }

    return(-1); // no match found
}

/*
	func. to see if	pattern	matches. Called	when first char	in pattern
	is found somewhere in the string.
	Pass the following:
	1. pointer to the matching char	in the string
	2. pointer to the pattern

	Returns	TRUE if	the	phrase matches,	FALSE in not
*/

int check_match(s, p)
char *s, *p;
{
    s++;
    p++;

    while(*p) {

        if(*s == '\r' || *s == 0)
            return(FALSE);

        if(*s == *p || *s == (*p)-32) {
            p++;
            s++;
        }
        else
            return(FALSE);
    }

    return(TRUE);
}


/*
	Calculate the proper offset	from the start of the search string
	that the pattern string	was	found at. This function	takes into
	consideration the tab characters in	the	line.

	pass: 1	pointer	to start of	the	search string
	pass: 2	pointer	to where pattern string	was	found in search	string

	returns: character offset into the line
*/

int calc_offset(searchptr, foundptr)
char	*searchptr,	*foundptr;
{
	register	int	offset;
	register	int	spaces;
	register	int	count;

	offset = 0;
	while(searchptr<foundptr)	{
		/* TODO: Original recovered source used '/t'; corrected to '\t'. */
		if(*searchptr == '\t')	 {
			spaces = tabstops[offset] -	offset;
			for(count=0; count < spaces; count++)	{
				offset++;		   /* inc offset the right number of times	*/
								   /* for the tab char */
			}
			searchptr++;
		}
		else			{
			offset++;
			searchptr++;
		}
	}
	return(offset);
}

/*
	func. to use for general purpose debugging
	parms to pass will vary	with the application
*/
void debug(o)
int		o;
{
	char	*syscur_y =	(char *)0x25;
	char	*syscur_x =	(char *)1403;
	char	ytmp, xtmp;

	ytmp = *syscur_y;
	xtmp = *syscur_x;

/* the debugging code goes here	*/

	message(" ");
	printf("%d", o);
	waitkey();

/* End of debugging	code		*/

	gotoxy(xtmp, ytmp);
}
