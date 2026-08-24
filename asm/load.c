/*
    File load.c

    This file must be linked with the getmem func. in misc.c.
*/

#include <stdio.h>
#include <string.h>
#include "ed.h"

extern struct fileinfo file1, file2;
extern char* vport_top;
extern char* vport_bot;
extern char* cursor_y;

char* getmem();

void bell();
void do_save();
void message();
void free_mem();
void load_file();
void drawscrn();


/*
    func. to prompt user for filename then call the load_file func.
    pass a pointer to the current fileinfo structure.
*/

void do_load(fip)
struct fileinfo* fip;
{
    register int i;
    register struct inpstat* sp;

    if(fip->update == 1) {
        bell();
        bell();
        bell();
        do_save(fip);
    }

    message("Load this file: ");

    sp = editline(
        fip->name,
        (sizeof(fip->name)-2)
    );

    if(sp->esc_stat)
        return;

    /*
     * The editline func. pads input line with spaces so place a 0
     * after last nonspace char.
     */
    for(i=0; fip->name[i] > 0x20; i++);

    fip->name[i] = 0;

    free_mem(fip);

    load_file(fip);

    drawscrn();
}


/*
    function to load source file from the disk

    This function expects to be passed a pointer to the current
    fileinfo structure.
*/

void load_file(fip)
struct fileinfo* fip;
{
    char string[80];
    register int line_num;

    FILE* dfile;

    if((dfile = fopen(fip->name, "r")) == NULL) {
        message("Cannot open file from read !");
        return;
    }

    /*
     * Get strings from the file and allocate memory to put them into.
     */

    line_num = 0;

    while(!feof(dfile) && line_num < max_lines) {

        if(fgets(string, 80, dfile)) {

            if(!(fip->index[line_num] = getmem(string))) {
                message("file to large!");
                break;
            }

            strcpy(
                fip->index[line_num],
                string
            );

            line_num++;
        }
    }

    if(fclose(dfile) != 0) {
        message("Cannot close file from read !");
        return;
    }

    fip->update = 0;
    fip->x = 0;
    fip->edline = 1;
    fip->dist_dwn = 1;
}