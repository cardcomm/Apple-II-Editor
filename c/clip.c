
#include <stdlib.h>
#include <string.h>
#include "ed.h"

#define		CLIP_LINES		200

	char	*getmem();
	void bell();
	void message();
	void expand_string();
	void print();
	void inverse();
	void normal();
	void move_dn();
	void move_up();
	void drawscrn();
	int quit();
	int waitkey();
	
	int copy_range();
	int make_room();
	void cut_range();
	void free_clip();

static	char	highlight;	/* if true,	highlight line for copy	*/
static	char	*clipindex[CLIP_LINES];
static	int		toprange;	 /*	line number	of top line	of copy	range */
static	int		botrange;	 /*	line number	of bottom line in range	*/
static	int		distance_dn; /*	save dist_dwn here so we can restore */
							 /*	position after cut or copy */
static	int		lines;		 /*	number of lines	selected so	far	*/

/*
	func. to select	the	line just edited so	it can be cut or copied
	pass: pointer to the current file data struct
*/
void
select(fip)
struct	fileinfo	*fip;
{
	static	char	s[82];

	if(lines > CLIP_LINES-1)	{
		bell();
		message("Selection too large");
		return;
	}
	lines++;
	gotoxy(0,fip->curln_ycor);
	inverse();
	expand_string(s, fip->index[fip->edline]);
	print(s);
	normal();
	move_dn(fip);
	botrange++;
}

/*
	func. to de-select a line while	in cut or copy mode
*/
void
deselect(fip)
struct	fileinfo	*fip;
{
	botrange--;
	lines--;
	move_up(fip);
	if(botrange	< toprange)
		highlight =	0;
}

/*
	func. to handle	the	open apple-C (cut or copy) command
	pass: pointer to current fileinfo structure
*/
void
copy(fip)
struct	fileinfo	*fip;
{
	if(highlight)	{  /* if true make false */
		highlight =	0;
		copy_range(fip);	/* copy	selected range to clip board */
		fip->edline	= toprange;
		fip->dist_dwn =	distance_dn;
		drawscrn();
	}

	else	{		   /* if not, make true	*/
		highlight =	1;
		lines =	0;
		distance_dn	= fip->dist_dwn;
		toprange = fip->edline;	/* get set to start	selecting text */
		botrange = toprange;
		if(clipindex[0]	!= 0)
			free_clip();
	}
}


/*
	Func. to copy selected range of	lines from the a fileinfo
	structure to the clipboard.
	pass: pointer to current fileinfo structure
	returns	-1 on error
*/

int copy_range(fip)
struct	fileinfo	*fip;
{
	register	int	i;
	register	int	c;

	c =	toprange;
	botrange--;
	for(i=0; c<=botrange; i++)	 {
		if(	(clipindex[i] =	getmem(fip->index[c])) == 0)	{
			free_clip();	/* if out of mem, free clip	memory and */
			return(-1);		/*	abort copy */
		}
		strcpy(clipindex[i], fip->index[c++]);
	}
	return(0);
}

/*
	Paste the text that's on the clipboard into	the	file. Insert text
	following the current line.
	Pass: pointer to the current fileinfo structure
*/
void
paste(fip)
struct	fileinfo	*fip;
{
	register	int		d, s;
	register	char	*ptr;

	if(clipindex[0]	== 0)	{
		message("Clipboard empty");
		return;
	}
	d =	fip->edline+1;
	if(	(make_room(fip)) ==	-1 )
		return;
	for(s=0; clipindex[s] != 0 && d	< max_lines-1; s++)	{
		if(	(ptr = getmem(clipindex[s])) ==	0 )	{
			while( (quit())	== -1)	{ /* if	out	of mem,	quit */
				bell();	/* if file error, give another chance */
				message("File error.");
				waitkey();
			}
		}
		fip->index[d] =	ptr;
		strcpy(fip->index[d], clipindex[s]);
		d++;
	}
	fip->update	= 1;	/* notify that we changed file */
	drawscrn();
}

/*
	Move all the addresses in the file index array to make room	for	lines
	to be pasted from clipboard
	pass: pointer to fileinfo struct
	returns: -1	on error, 0	on success
*/
int make_room(fip)
struct	fileinfo	*fip;
{
	register	int		new, old, numlines;

	numlines = botrange	- toprange;
	for(new=0; fip->index[new] != 0; new++)	/* find	first line past	EOF	*/

	if(	(new+numlines) >= (max_lines-1)	) {
		message("File index full");
		bell();
		return(-1);
	}
	old	= new-1;
	new	+= numlines;
	while(old >	fip->edline)	{ /* move the line addresses to	make room */
		fip->index[new]	= fip->index[old];
		new--;	old--;
	}
	return(0);
}

/*
	Cut	text from file and put it on the clipboard intead ofjust
	copying	it.
	pass: pointer to fileinfo structure
*/
void cut(fip)
struct	fileinfo	*fip;
{
	if(highlight==0)   {
		bell();
		return;
	}
	highlight=0;
	if(	(copy_range(fip)) == -1	) /* if	error don't	delete lines of	text */
		return;
	cut_range(fip);
	fip->edline	= toprange;
	fip->dist_dwn =	distance_dn; /*	restore	last edline	and	cursor position	*/
	drawscrn();
	fip->update	= 1;
}


/*
	Func. to "cut" selected	range of lines from	the	fileinfo
	structure to the clipboard.
	pass: pointer to current fileinfoe structure
*/
void cut_range(fip)
struct	fileinfo	*fip;
{
	register	int	d;
	register	int	s;

	for(s=toprange;	s<botrange;	s++)   {
		/* TODO: Aztec C free() returned an error status; cc65 free() returns void. */
		free(fip->index[s]);
	}
	s =	fip->edline;
	d =	toprange;
	while(s	< max_lines-1 )	{
		fip->index[d] =	fip->index[s];
		s++;	d++;
	}
	fip->index[d] =	0;
}

/*
	func. to free all the memory used by the clipboard and clear
	all	elements in	the	index array
*/
void free_clip()
{
	register	int		i;

	for(i=0; clipindex[i]; clipindex[i++]=0) {
		/* TODO: Aztec C free() returned an error status; cc65 free() returns void. */
		free(clipindex[i]);
	}
}


/*
	func. to return	the	value of highlight
*/
int send_highlight()	{
	return(highlight);
}


/*
  function to clear	all	the	elements in	the	clipboard index	array
*/
void
init_clip()
{
	register	int		elem;

	for(elem=0;	elem<CLIP_LINES-1; elem++)
		clipindex[elem]	= 0;
}
