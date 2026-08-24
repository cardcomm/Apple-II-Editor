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

	file1.edline = 1;	   file2.edline	= 1;
	file1.dist_dwn = 1;	 file2.dist_dwn	= 1;
	file1.update = 0;	   file2.update	= 0;
	file1.x	= 0;			file2.x	= 0;

	/*
	 * TODO: Original code intentionally preserved.
	 *
	 * The original init_ed() uses margv[1] without first checking
	 * margc. This appears to be a bug if the editor is started
	 * without a filename argument. Do not change during the initial
	 * port; revisit after the original program is running.
	 */
	strcpy(file1.name, margv[1]);

	message("Loading: ");	println(file1.name);
	load_file(file1.name, file1.index);	/* load	file1  */

	if(margc == 3)	{
		strcpy(file2.name, margv[2]);
		message("Loading: "); println(file2.name);
		load_file(file2.name, file2.index);	/* load	file2  */
		div_line = 12;
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