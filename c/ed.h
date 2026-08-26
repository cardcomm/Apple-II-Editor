#define     max_lines   500
#define     tabsize     4

struct fileinfo {
    char    name[65];               /* file name string */
    char*   index[max_lines];       /* file data index array */
    int     edline;                 /* number of current line in file */
    char    dist_dwn;               /* number of lines from top of window to place
                                       the current line */
    char    curln_ycor;             /* y-cor the current source line was printed at */
    int     topline;                /* # of line to display at top of viewport */
    int     botline;                /* # of line to display at bottom of viewport */
    char    topy;                   /* ycor to print top line */
    char    boty;                   /* ycor to print bottom line */
    char    x;                      /* cursor x coordinate */
    char    update;                 /* if true the file was changed and should be
                                       updated if desired by user */
};

/* this structure type is passed back from the input functions */
struct inpstat {
    char    applecmd;               /* if applekey pressed, ascii code here, else space */
    char    esc_stat;               /* true if <esc> was pressed */
    char    control_cr;             /* if control key pressed it's here, else 0 */
    char    changed;                /* true if string was changed */
    char    last_xcor;              /* cursor xcor when exiting editline func. */
};


/*
 * cc65 interface to the original assembly line editor.
 *
 * __fastcall__ is the cc65-specific calling-convention adaptation.
 * The visible function interface otherwise matches the original code.
 */

struct inpstat* __fastcall__ editline(
    char* string,
    unsigned char maxlength
);

struct inpstat* __fastcall__ editline2(
    char* string,
    unsigned char maxlength,
    unsigned char string_pos
);

void __fastcall__ home(void);
void __fastcall__ text(void);
void __fastcall__ crout(void);

void __fastcall__ gotoxy(unsigned char x, unsigned char y);

void __fastcall__ print(const char *string);
void __fastcall__ println(const char *string);

void __fastcall__ blank_line(void);
void __fastcall__ solid_line(void);