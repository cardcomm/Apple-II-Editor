#include <stdio.h>
#include <apple2.h>

int main(void)
{
    FILE *f;
    static const unsigned char data[] = {
        'O','N','E',0x0D,
        'T','W','O',0x0D,
        'T','H','R','E','E',0x0D
    };

    _filetype = PRODOS_T_TXT;
    _auxtype  = PRODOS_AUX_T_TXT_SEQ;

    f = fopen("FWRTOUT.C", "w");

    if (f == NULL) {
        printf("OPEN FAILED\n");
        return 1;
    }

    fwrite(data, 1, sizeof(data), f);

    fclose(f);

    return 0;
}