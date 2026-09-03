
#include	"ed.h"
#include <stdio.h>
#include <string.h>

#define	TRUE	1
#define	FALSE	0

extern	char	tabstops[];		/* defined in the "strings.c" source file */

void message();
void drawscrn();
int waitkey();

int search_string();
int check_match();
int calc_offset();

void expand_string();
void inverse();
void normal();
void highlight_match();

int source_offset();

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
    register int start;
    register int key;

    int original_line;
    int original_start;
    int wrapped;

    message("Enter search string:");

    sp = editline(
        pattern,
        (sizeof(pattern) - 2)
    );

    if(sp->esc_stat)
        return;

    /*
     * Remove trailing spaces supplied by editline().
     */
    for(t = pattern + (sizeof(pattern) - 2);
        t >= pattern && *t == 0x20;
        t--)
        ;

    t++;
    *t = 0;

    if(pattern[0] == 0)
        return;

    /*
     * Remember where this search began.
     */
    original_line = fip->edline;

    original_start = source_offset(
        fip->index[original_line],
        fip->x
    );

    search_line = original_line;
    start = original_start;

    wrapped = FALSE;

    for(;;) {

        /*
         * Search forward for the next occurrence.
         */
        while(search_line < max_lines &&
              fip->index[search_line] != 0) {

            /*
             * Once we've wrapped, don't search beyond the
             * original starting line.
             */
            if(wrapped && search_line > original_line) {
                message("No more occurrences.");
                return;
            }

            offset = search_string(
                fip->index[search_line],
                pattern,
                start
            );

            if(offset >= 0) {

                /*
                 * If we've wrapped back to the original line,
                 * don't accept a match at or beyond the original
                 * starting position.
                 */
                if(wrapped &&
                   search_line == original_line &&
                   offset >= original_start) {

                    message("No more occurrences.");
                    return;
                }

                break;
            }

            search_line++;
            start = 0;
        }

        /*
         * EOF reached. Wrap once to the beginning.
         */
        if(search_line >= max_lines ||
           fip->index[search_line] == 0) {

            if(wrapped) {
                message("No more occurrences.");
                return;
            }

            wrapped = TRUE;
            search_line = 0;
            start = 0;

            continue;
        }

        /*
         * Keep the viewport stationary if the matching line
         * is already visible.
         */
        if(search_line >= fip->topline &&
           search_line <= fip->botline) {

            fip->dist_dwn =
                search_line - fip->topline;
        }

        fip->edline = search_line;

        /*
         * Convert packed-source offset to display column.
         */
        fip->x = calc_offset(
            fip->index[search_line],
            fip->index[search_line] + offset
        );

        drawscrn();

        highlight_match(fip, pattern);

        message("Find next occurrence? Y/N");

        for(;;) {

            key = waitkey();

            if(key == 'Y' || key == 'y')
                break;

            if(key == 'N' ||
               key == 'n' ||
               key == 27)
                return;
        }

        /*
         * Continue immediately after this match.
         */
        start = offset + 1;
    }
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

int search_string(string, pattern, start)
char *string;
char *pattern;
int start;
{
    register char *str_start;
    register char *search;

    str_start = string;
    search = string + start;

    while(*search != '\r' && *search != 0) {

        if(*search == *pattern ||
           *search == (*pattern)-32) {

            if(check_match(search, pattern)) {
                return(search - str_start);
            }
        }

        search++;
    }

    return(-1);
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


void highlight_match(fip, pattern)
struct fileinfo *fip;
char *pattern;
{
    static char expanded[82];
    static char match[50];

    register int i;
    register int len;

    /*
     * Expand the packed source line exactly as it is displayed.
     */
    expand_string(
        expanded,
        fip->index[fip->edline]
    );

    len = strlen(pattern);

    /*
     * Copy the displayed characters corresponding to the match.
     *
     * fip->x is already the display-column position calculated
     * by calc_offset().
     */
    for(i = 0; i < len && (fip->x + i) < 81; i++)
        match[i] = expanded[fip->x + i];

    match[i] = 0;

    /*
     * Redraw only the matching text in inverse video.
     */
    gotoxy(
        fip->x,
        fip->curln_ycor
    );

    inverse();
    print(match);
    normal();
}

/*
 * Convert a displayed cursor column to an offset in the
 * packed source string.
 */
int source_offset(string, x)
char *string;
int x;
{
    register int offset;

    offset = 0;

    while(string[offset] != '\r' &&
          string[offset] != 0) {

        if(calc_offset(string, string + offset) >= x)
            break;

        offset++;
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
