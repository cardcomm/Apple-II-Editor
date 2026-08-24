#include <stdio.h>

#include "ed.h"


int main(void)
{
    static char buffer[81] = "THIS IS A TEST";

    struct inpstat* status;


    printf("Edit the line.\n");
    printf("Try RETURN, UP, DOWN, ESC, etc.\n\n");

    status = editline2(
        buffer,
        sizeof(buffer) - 2,
        0
    );


    printf("\nReturned buffer: %s\n", buffer);

    printf(
        "applecmd=$%02X esc=%u control=$%02X changed=%u x=%u\n",
        (unsigned char)status->applecmd,
        (unsigned char)status->esc_stat,
        (unsigned char)status->control_cr,
        (unsigned char)status->changed,
        (unsigned char)status->last_xcor
    );

    return 0;
}