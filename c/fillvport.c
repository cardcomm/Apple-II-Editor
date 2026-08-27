
#include	"ed.h"

extern	char*	vport_top;
extern	char*	vport_bot;

void expand_string();

/*
	This func. is to print the visible portion of a	source file
	in the viewport. The parm to pass is:
	1. pointer to the current fileinfo structure
	This function returns the y-cor	that the current source	line was
	 printed at.
*/

char	fill_vport(fip)
struct	fileinfo*	fip;
{
	register	int		vport_lines;	/* total number	of lines in	viewport */
	register	int		bot_part;
	register	int		count;
	register	int		y;
	register	char	cur_ycor; /* ycor current source line was printed at */
				char	dest[82];

	vport_lines	= *vport_bot - *vport_top;
	if(fip->dist_dwn > vport_lines)
		fip->dist_dwn =	vport_lines-1;
	if(fip->edline < fip->dist_dwn)
		fip->dist_dwn =	fip->edline;

	cur_ycor = *vport_top +	fip->dist_dwn;

	bot_part = vport_lines - fip->dist_dwn;	/* calc. # of scrn lines below
											   current line	 */
	fip->topline = fip->edline - fip->dist_dwn;
	fip->botline = fip->edline + (bot_part-1);


	count =	fip->topline;
	y =	*vport_top;
	while(count	<= fip->botline	&& count <=	max_lines)	{
		gotoxy(0, y);
		if(fip->index[count] !=	0)	{
			expand_string(dest,	fip->index[count]);
			print(dest);
		}
		else
			blank_line();

		count++;
		y++;
	}
	return (cur_ycor);
}
