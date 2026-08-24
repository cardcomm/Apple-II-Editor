/*
    File cmds.c

    functions to handle the editing commands
*/

#include <stdio.h>
#include <string.h>

#include "ed.h"

extern char div_line;        /* y-cor to place vport dividing line */
extern char vport_actv;
extern struct fileinfo file1, file2;
extern struct fileinfo *fip1;
extern struct fileinfo *fip2;
extern struct fileinfo *fip;

char *regetmem();

/*
 * Function declarations required by cc65.
 *
 * The original Aztec C compiler allowed calls before declarations.
 */
void drawscrn();
void message();
void bell();

int do_save();

int atoi(const char *s);
void free(void *ptr);
void exit(int status);

void itoa(char *buf, int num);


static char replace_str[82];     /* last string Deleted stored here for use
                                    by the Replace command */


/*
    Func. to handle the apple-Replace command

    The Func. really swaps the text of the current line with the contents of
    replace_str buffer. This area contains the string last Deleted.

    pass pointer to the fileinfo structure for the file to be modified
*/

void replace(fip)
struct fileinfo *fip;
{
    static char s[82];

    strcpy(s, fip->index[fip->edline]);

    fip->index[fip->edline] =
        regetmem(replace_str, fip->index[fip->edline]);

    strcpy(fip->index[fip->edline], replace_str);
    strcpy(replace_str, s);

    fip->update = 1;          /* notify that we changed the file */

    drawscrn();
}


/*
    func. to delete the current line

    pass pointer to the fileinfo structure for the file to be modified
*/

void delete(fip)
struct fileinfo *fip;
{
    register int i;

    if(fip->index[fip->edline] != 0) {

        /* save string temporarily for use by Replace */

        strcpy(replace_str, fip->index[fip->edline]);

        /*
         * TODO:
         * Original Aztec C free() returned an error status and this
         * code checked for -1. cc65 free() returns void, so that
         * diagnostic cannot be preserved directly.
         */

        free(fip->index[fip->edline]);
    }

    /*
     * Move all the pointers over to fill the gap in the deleted line.
     */

    for(i = fip->edline; fip->index[i] != 0; ) {
        fip->index[i] = fip->index[++i];
    }

    fip->update = 1;          /* notify that we changed the file */

    drawscrn();
}


/*
    func. to insert a blank line after the current line

    pass pointer to the fileinfo structure for the file to be modified
*/

void insert(fip)
struct fileinfo *fip;
{
    register int new;
    register int old;

    register char *vport_bot = (char *)0x23;


    for(new = 0; fip->index[new] != 0; new++)
        ;                           /* find first line past EOF */

    if(new >= max_lines - 1) {
        message("File index full.");
        return;
    }

    old = new - 1;

    while(old > fip->edline) {

        /* move all line addresses over to make room for new line */

        fip->index[new] = fip->index[old];

        new--;
        old--;
    }

    old++;

    if(fip->curln_ycor + 1 < *vport_bot)
        fip->dist_dwn++;

    fip->index[old] = 0;       /* set address of new line buffer to 0 */

    fip->edline++;             /* move cursor to the blank line */

    fip->x = 0;
    fip->update = 1;           /* notify that we changed the file */

    drawscrn();
}


/*
    func. to change the location of the screen dividing line

    note: this func. changes the global var. div_line
*/

void set_div_ln(void)
{
    register char t;
    register char err;

    static char s[] = "12   ";

    register struct inpstat *sp;


    do {
        err = 0;

        message("Enter new dividing line coordinate: ");

        sp = editline(
            s,
            (sizeof(s) - 2)
        );

        if(sp->esc_stat)
            return;

        t = atoi(s);

        if(t < 2 || t > 23) {

            /* check for out of range */

            bell();
            err = 1;
        }

        if((t < 4 && vport_actv == 1) ||
           (t > 18 && vport_actv == 2)) {

            bell();

            message(
                "Can't make window that small with cursor in it."
            );

            err = 1;
        }

        div_line = t;

    } while(err);

    drawscrn();
}


/*
    func to Jump to the other viewport and make that file the current one
*/

void jump(void)
{
    /*
     * if vport_actv=1 make it a 2 and vice versa
     */

    if(div_line < 4 || div_line > 18) {
        message("Window too small to jump into.");
        return;
    }

    if(vport_actv == 1) {
        vport_actv = 2;
        fip = &file2;
    }
    else {
        vport_actv = 1;
        fip = &file1;
    }

    drawscrn();
}


/*
    open apple Down arrow command.
    Move down in text one screen full.

    pass: pointer to the fileinfo struct for the current file
*/

void pagedown(fip)
struct fileinfo *fip;
{
    register char *vport_top = (char *)0x22;
    register char *vport_bot = (char *)0x23;
    register int lines;

    lines = (*vport_bot - *vport_top) - 2;

    if((fip->botline + lines) >= (max_lines - 1)) {

        fip->edline = max_lines - 1;
        fip->dist_dwn = lines - 1;

        drawscrn();
        bell();

        return;
    }

    fip->edline += lines;

    drawscrn();
}


/*
    open apple Up arrow command.
    Move up in text one screen full.

    pass: pointer to the fileinfo struct for the current file
*/

void pageup(fip)
struct fileinfo *fip;
{
    register char *vport_top = (char *)0x22;
    register char *vport_bot = (char *)0x23;
    register int lines;

    lines = (*vport_bot - *vport_top) - 2;

    if((fip->topline - lines) <= 0) {

        fip->edline = 0;

        bell();
        drawscrn();

        return;
    }

    fip->edline -= lines;

    drawscrn();
}


/*
    func. to move cursor to the line requested by the user

    pass: pointer to the fileinfo struct for the current file
*/

void gotoline(fip)
struct fileinfo *fip;
{
    register struct inpstat *sp;
    register int t;

    static char s[] = "1   ";

    do {
        message("Enter number of line to goto: ");

        sp = editline(
            s,
            (sizeof(s) - 2)
        );

        if(sp->esc_stat)
            return;

        t = atoi(s);

        if(t > -1 && t < max_lines) {
            fip->edline = t;
            break;
        }

        bell();

    } while(0);

    drawscrn();
}


/*
    func. to handle the Quit command

    Original callers test quit() for -1 if a save fails.
*/

int quit(void)
{
    register int errstat;

    if(fip1->update) {

        bell();

        errstat = do_save(fip1);

        if(errstat == -1)
            return -1;             /* error writing file */
    }

    if(fip2->update) {

        bell();

        errstat = do_save(fip2);

        if(errstat == -1)
            return -1;             /* error writing file */
    }

    text();
    println("");

    /*
     * Aztec C allowed exit() with no argument.
     * cc65 uses the standard exit(int) interface.
     */

    exit(0);

    return 0;                       /* unreachable */
}


/*
    display info about the file in the current viewport
*/

void show_fileinfo(fip)
struct fileinfo *fip;
{
    static char buf[10];

    gotoxy(0,0);

    message("The current file is:  ");

    print(fip->name);

    print("                    The current line is  ");

    itoa(buf, fip->edline);

    print(buf);

    drawscrn();
}


/*
    this func. was part of the Manx "ved" source code

    it will convert a positive integer no larger than 9999 to a string

    pass:
        1 pointer to the string to store the results in
        2 number to convert
*/

void itoa(char *buf, int num)
{
    register int flg = 0;
    register int i;
    register int j = 1000;

    while(j) {

        i = num / j;

        num %= j;
        j /= 10;

        if(i == 0 && flg == 0)
            continue;

        *buf++ = i + '0';

        flg = 1;
    }

    if(flg == 0)
        *buf++ = '0';

    *buf = 0;
}