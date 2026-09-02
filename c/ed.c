
/* 
	File ed.c
This	is the main	module of my very own programmer's text	editor.	*/

#include	"ed.h"
#include <stdio.h>
#include <string.h>
#include <apple2enh.h>

/*
	global data	declarations
*/

struct	fileinfo	file1, file2; /* this struct type defined in ed.h */
struct	fileinfo*	fip;
struct	fileinfo*	fip1 = &file1;
struct	fileinfo*	fip2 = &file2;
struct	inpstat*	is_ptr;	/* pointer to input	status structure */
char	div_line = 23;		   /* y-cor	to place vport dividing	line */
char	vport_actv = 1;			   /* which	viewport is	active (1 or 2)	*/
char* vport_top = (char*)0x22;
char* vport_bot = (char*)0x23;
char* cursor_y  = (char*)0x25;

char* regetmem();
char* getmem();
struct inpstat* edit();

/*
 * Function declarations required by cc65.
 *
 * Original Aztec C allowed calls to functions without prior declarations.
 * These declarations preserve the original interfaces and behavior.
 */
void init_ed();
void drawscrn();

int send_highlight();
void select();
void deselect();

int move_dn();
int move_up();

int insert();
int delete();

int do_search();
int replace();

int do_save();
int do_load();

int set_div_ln();
int jump();
int gotoline();

void copy();
void paste();
int cut();

int show_fileinfo();

int pagedown();
int pageup();

int quit();

int main(argc,argv)
int argc;
char *argv[];
{
	// Modification from original code
    int prompt_load;
    prompt_load = (argc == 1); // save arg flag = true if no args

	init_ed(argc, argv);

	drawscrn(fip1, fip2);
	
	// Modification from original code - if no cmd line args, prompt to load file
	//		under ProDOS basic.system, command line args are not supported
	if(prompt_load)
		do_load(&file1);

	do	{

		if(vport_actv == 1)	/* set up pointer to active	fileinfo struct	*/
			fip	= &file1;
		else
			fip	= &file2;

		is_ptr = edit(fip);

		switch(is_ptr->control_cr)	{ /* check for control char	keypress */

			case 0x0a:	/* down	arrow */
				if(is_ptr->applecmd	!= 0xa0) /*	make sure it's not apple cmd */
					break;
				if(	(send_highlight()) )
					select(fip); /*	"select" this line for cutting if req. */
				else
					move_dn(fip);
				break;

			case 0x0b:	/* up arrow	*/
				if(is_ptr->applecmd	!= 0xa0) /*	make sure it's not apple cmd */
					break;
				if(	(send_highlight()) )
					deselect(fip); /* "de-select" if required */
				else
					move_up(fip);
				break;

			case 0x0d:	/* return char */
				insert(fip);
				break;
		}

		switch(is_ptr->applecmd)	{	/* check for apple keypress	*/

			case 'd':	/* open	apple Delete command */
			case 'D':
				delete(fip);
				break;

			case 'f':
			case 'F':
				do_search(fip);	   /* open app;le Find command */
				break;

			case 'r':	/* open	apple Replace command */
			case 'R':
				replace(fip);
				break;

			case 's':
			case 'S':	/* open	apple Save command */
				do_save(fip);
				break;

			case 'l':
			case 'L':	/* open	apple Load command */
				do_load(fip);
				break;

			case 'w':
			case 'W':	/* open	apple Window command */
				set_div_ln();
				break;

			case 'j':
			case 'J':	/* open	apple Jump command */
				jump();
				break;

			case 'g':
			case 'G':	/* open	apple Goto command */
				gotoline(fip);
				break;

			case 'C':
			case 'c':	/* open	apple Copy command */
				copy(fip);
				break;

			case 'P':
			case 'p':
			case 'V':
			case 'v':	/* open	apple Paste	command	*/
				paste(fip);
				break;

			case 'X':
			case 'x':	/* open	apple cut command */
				cut(fip);
				break;

			case 'I':
			case 'i':	/* open	apple information command */
				show_fileinfo(fip);
				break;


			case 0x8a:	/* open	apple down arrow */
				pagedown(fip);
				break;

			case 0x8b:	/* open	apple up arrow */
				pageup(fip);
				break;

			case 'q':
			case 'Q':	/* open	apple Quit command */
				quit();
				break;
		}

	} while(1);
	
	return 0;
}
