
#include	"ed.h"

extern	struct	fileinfo	file1, file2;

extern	char	*vport_top;
extern	char	*vport_bot;
extern	char	div_line;		/* y-cor to	place vport	dividing line */
extern	char	vport_actv;		/* which viewport is active	(1 or 2) */

char	fill_vport();

/*
	Function to	draw the editing screen.
	This function uses the global "fileinfo" structures	and	the
	global symbols div_line	and	vport_actv.
*/

void	drawscrn()		{

	text();
	gotoxy(0,1);	/* draw	the	top	viewport */
	solid_line();
	*vport_top = 2;
	gotoxy(0,div_line);
	solid_line();
	*vport_bot = div_line;

	file1.curln_ycor = fill_vport(&file1);


	*vport_top = div_line+1;	/* draw	the	bottom viewport	*/
	*vport_bot = 23;

	file2.curln_ycor = fill_vport(&file2);

	switch(vport_actv)	{	/* find	out	which viewport is active */
		case 1:
			*vport_top = 2;	/* set active window in	top	viewport */
			*vport_bot = div_line;
			gotoxy(0,file1.curln_ycor);
			break;

		case 2:
			*vport_top = div_line+1; /*	set	active window in bottom	viewport */
			*vport_bot = 23;
			gotoxy(0,file2.curln_ycor);
			break;
	}
}
