/*
	File edit.c

	Function to edit the current line in the current file.
*/

#include "ed.h"
#include <string.h>

char* getmem();
char* regetmem();

/*
 * Function declarations required by cc65.
 *
 * Original Aztec C allowed calls without prior declarations.
 * These preserve the original interfaces.
 */
void pack_string();
void expand_string();

int quit();
void bell();
void message();
int waitkey();

/*
	Function to	edit the current line in the current file. If neccessary,
	it will	reallocate the memory buffer used to store the line.
	pass a pointer to the current fileinfo structure
*/

#define STRINGSIZE 82
#define EDIT_MAX_LENGTH 79

struct inpstat* edit(fip)
struct fileinfo* fip;
{
	static char s1[STRINGSIZE];
	static char s2[STRINGSIZE];
	register char i;
	register char* ptr;
	register struct inpstat* sp;	/* pointer to input return status
									   struct */


/* if this line is empty, get user input, then get mem for it */

	if(fip->index[fip->edline] == 0) {
		for(i=0; i<STRINGSIZE-1; s1[i++]=' '); /* init s1 */
		s1[i] = 0;

		gotoxy(0, fip->curln_ycor);

		sp = editline2(
			s1,
			EDIT_MAX_LENGTH,
			fip->x
		);

		pack_string(s2, s1);

		if((ptr=getmem(s2)) == 0) {
			while((quit()) == -1) { /* if out of mem, quit */
				bell();	/* if file error, give another chance */
				message("File error.");
				waitkey();
			}
		}

		fip->index[fip->edline] = ptr;
		strcpy(fip->index[fip->edline], s2);

		if(sp->changed)
			fip->update = 1;

		fip->x = sp->last_xcor;
	}


/* if the line exists, edit it, then realloc the mem. block if we need to */

	else {
		expand_string(s1, fip->index[fip->edline]);

		gotoxy(0, fip->curln_ycor);

		sp = editline2(
			s1,
			EDIT_MAX_LENGTH,
			fip->x
		);

		pack_string(s2, s1);

		if((ptr=regetmem(s2, fip->index[fip->edline])) == 0) {
			while((quit()) == -1) { /* if out of mem, quit */
				bell();	/* if file error, give another chance */
				message("File error.");
				waitkey();
			}
		}

		fip->index[fip->edline] = ptr;
		strcpy(fip->index[fip->edline], s2);

		if(sp->changed)
			fip->update = 1;

		fip->x = sp->last_xcor;
	}

	return(sp);
}