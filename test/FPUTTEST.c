#include <stdio.h>
#include <apple2.h>

int main(void)
{
    FILE *f;

    _filetype = PRODOS_T_TXT;
    _auxtype  = PRODOS_AUX_T_TXT_SEQ;

    f = fopen("FPUTOUT.C", "w");

    if (f == NULL) {
        printf("OPEN FAILED\n");
        return 1;
    }

    fputs("ONE\rTWO\rTHREE\r", f);

    fclose(f);

    return 0;
}