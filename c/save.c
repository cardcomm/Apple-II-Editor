
#include	<stdio.h>
#include 	<apple2.h>
#include	"ed.h"

extern	char*	cursor_y;
void message();
int save_file();

/*
	func. to prompt	user for filename then call	the	save_file func.
	pass a pointer to the current fileinfo structure.
	return:	-1 if error	writing	file, 1	if no error
*/
int do_save(fip)
struct	fileinfo*	fip;
{
	register	char	ytmp;
	register	int		i;
	register	struct	inpstat*	sp;	/* pointer to input	return status
											 struct	*/
	ytmp = *cursor_y;
	message("Save this file: ");
	sp = editline(fip->name, (sizeof(fip->name)-2) );
	if(sp->esc_stat)
		return(0);   /* TODO: Original Aztec code used bare return in int function. */;
	for(i=0; fip->name[i] >	0x20; i++);	/* put 0 after last	non-space char */
	fip->name[i] = 0;
	if(	(save_file(fip)) ==	-1)
		return(-1);		/* signal error	writing	file */
	fip->update	= 0;	/* clear file update flag */
	gotoxy(0,ytmp);
	return(1);			/* signal no errors	writing	file */
}

/*
	func. to save a	file to	disk. A	pointer	to the current
	fileinfo structure should be passed	when calling.
	returns	-1 on error, 1 if no error
*/

int save_file(fip)
struct	fileinfo*	fip;
{
	FILE	*dfile;

	register	int		line_num;

	/* get rid of blank	lines at end of	file before	saving */
	for(line_num=0;	fip->index[line_num]!= 0; line_num++);
	line_num--;
	for( ; *fip->index[line_num] ==	'\r'; line_num--);
	line_num++;
	fip->index[line_num] = 0;

	_filetype = PRODOS_T_TXT;
	_auxtype  = PRODOS_AUX_T_TXT_SEQ;

if((dfile = fopen(fip->name, "w")) == NULL) {
		message("Cannot open file for write !");
		return(-1);
		}

	line_num = 0;
	while (	fip->index[line_num] !=	0 )	{
		if(fputs(fip->index[line_num], dfile) == EOF) { 
			message("Cannot write to file !");
			return(-1);
		}
		line_num++;
	}

	if(	(fclose	(dfile)) !=	0)		{
		message("Cannot close file from read !");
		return(-1);
		}
	return(1); /* signal error free	exit */
}
