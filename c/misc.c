/*	misc. functions	used by	the	ed program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ed.h"

extern struct fileinfo file1, file2;
extern char vport_actv;
extern char div_line;

/*
 * Function declarations required by cc65.
 *
 * The original Aztec C compiler allowed calls before declarations.
 * Preserve the original interfaces.
 */
void init_array();
void init_clip();
void message();
void load_file();
int waitkey();


/* function	to init. the program */

void init_ed(margc, margv)
int	margc;
char*	margv[];
{
	home();
	init_array(file1.index);
	init_array(file2.index);
	init_clip();

	file1.update = 0;	   file2.update	= 0;
	file1.x	= 0;			file2.x	= 0;
	
	/*
	 * cc65 / BASIC.SYSTEM compatibility extension:
	 *
	 * The original Aztec C environment supplied normal command-line
	 * arguments, and the recovered editor assumes margv[1] always names
	 * the first file to load.
	 *
	 * Under ProDOS BASIC.SYSTEM the editor may need to be started with
	 * no filename at all.  In that case create an empty editing buffer
	 * whose first editable line is index[0].
	 *
	 * For a normally loaded file, preserve the original editor behavior:
	 * edline=1 and dist_dwn=1 place the initial current line one row below
	 * the top of the viewport.
	 */
	
	if(margc > 1) {
		strcpy(file1.name, margv[1]);
	
		file1.edline = 1;
		file1.dist_dwn = 1;
	
		message("Loading: ");
		println(file1.name);
	
		load_file(&file1);
	}
	else {
		file1.name[0] = 0;
	
		file1.edline = 0;
		file1.dist_dwn = 0;
	}
	
	/*
	 * Preserve the original two-file startup behavior when two filenames
	 * are supplied.
	 */
	if(margc == 3) {
		strcpy(file2.name, margv[2]);
	
		file2.edline = 1;
		file2.dist_dwn = 1;
	
		message("Loading: ");
		println(file2.name);
	
		load_file(&file2);
	
		div_line = 12;
	}
	else {
		file2.name[0] = 0;
	
		file2.edline = 0;
		file2.dist_dwn = 0;
	}

}


/*
	func. to free all the memory used by a source file
	pass a pointer to the current fileinfo pointer
*/

void free_mem(fip)
struct	fileinfo*	fip;
{
	register	int	i = 0;

	while(fip->index[i])	{
		/*
		 * TODO: Original Aztec C free() returned an error status and this
		 * code checked for -1. cc65 free() returns void, so that check
		 * cannot be preserved directly.
		 */
		free(fip->index[i]);
		fip->index[i++] = 0;
	}
}


/*
	function to	clear all the elements in an array of pointers
	init_array(pointarray);
*/

void init_array(index_array)
char	*index_array[max_lines];
{
	register	int	elem;

	for(elem=0; elem<max_lines; elem++)
		index_array[elem] = NULL;
}


/*
	function to	call malloc	to get memory for a	string
	returns	a pointer to the memory	area.
	mempointer = getmem(string);
*/

char* getmem(string)
char*	string;
{
	register	char	*ptr;

	if(!(ptr = malloc(strlen(string)+1))) {
		message("Out of memory error!");
		waitkey();
		return(0);
	}

	return(ptr);
}


/*
	function to	call realloc to	get	a new mem. block for a string
	returns	a pointer to the memory	area.
	pass the following:
	1. pointer to new string
	2. pointer to mem block	old	string is stored in
	for	example:  newpointer = regetmem(string,	oldpointer);
*/

char* regetmem(string, oldptr)
char*	string;
char*	oldptr;
{
	register	char	*newptr;

	/*
	 * TODO: Original Aztec C free() returned an error status.
	 * cc65 free() returns void.
	 */
	free(oldptr);

	if(!(newptr = malloc(strlen(string)+1))) {
		message("Out of memory error.");
		waitkey();
		return(0);
	}

	return(newptr);
}


/*
	func. to print message to the user at screen location 0,0
	pass: pointer to the string	to be printed
*/

void message(string)
char*	string;
{
	gotoxy(0,0);
	blank_line();
	gotoxy(0,0);
	print(string);
}