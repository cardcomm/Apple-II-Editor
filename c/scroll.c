
#include	"ed.h"

extern	char*	vport_top;
extern	char*	vport_bot;

void scroll_up(void);
void scroll_dn(void);

/*
	func. to handle down arrow keypress while editing a source line
	Pass a pointer to the current fileinfo structure
*/

void move_dn(fip)
struct	fileinfo*	fip;
{
	if (fip->edline == max_lines-1)
		return;

	if(fip->curln_ycor+1 < *vport_bot )	{
		fip->curln_ycor++;
		fip->edline++;
		fip->dist_dwn++;
		gotoxy(0,fip->curln_ycor);
	}
	else	{
		scroll_up();
		fip->edline++;
		gotoxy(0,fip->curln_ycor);
	}
}


/*
	func. to handle up arrow keypress while editing a source line
	Pass a pointer to the current fileinfo structure
*/

void move_up(fip)
struct	fileinfo*	fip;
{
	if(fip->edline == 0)
		return;

	if(fip->curln_ycor > *vport_top )	{
		fip->curln_ycor--;
		fip->edline--;
		fip->dist_dwn--;
		gotoxy(0,fip->curln_ycor);
	}
	else	{
		scroll_dn();
		fip->edline--;
		gotoxy(0,fip->curln_ycor);
	}

}
