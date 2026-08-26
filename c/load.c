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
 * cc65 compatibility:
 *
 * The original Aztec C fgets() used by this editor treated the
 * ProDOS CR ($0D) line terminator as the end of an input line.
 *
 * cc65 fgets() does not.  It looks for the C newline character
 * LF ($0A), so a CR-delimited ProDOS TXT file is read in buffer-
 * sized chunks instead of one source line at a time.
 *
 * Read one CR-delimited ProDOS source line here to preserve the
 * behavior expected by the original editor.  The CR is retained
 * because the editor's internal packed-line representation is:
 *
 *     line contents + CR + NUL
 *
 * The editor is designed around a maximum physical line length
 * of 80 columns and does not support wrapped source lines.
 */
static char* get_cr_line(char* string, int size, FILE* dfile)
{
    int c;
    int i;

    i = 0;

    while(i < size - 1) {

        c = fgetc(dfile);

        if(c == EOF) {
            if(i == 0)
                return NULL;

            break;
        }

        string[i++] = (char)c;

        if(c == '\r')
            break;
    }

    string[i] = 0;

    return string;
}

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
	
		/*
		 * cc65 compatibility:
		 * Use the CR-delimited reader above instead of fgets().
		 * See get_cr_line() for the difference from the original
		 * Aztec C runtime behavior.
		 */
		if(get_cr_line(string, 80, dfile)) {
	
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

