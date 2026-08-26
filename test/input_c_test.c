#include <stdio.h>
#include <conio.h>
#include <apple2enh.h>

extern void __fastcall__ editline(
    char *buffer,
    unsigned char maxLength
);

int main(void)
{
    char buffer[80] = "THIS IS A TEST";

    videomode(VIDEOMODE_80COL);
    clrscr();

    gotoxy(0, 10);

    editline(buffer, 60);

    gotoxy(0, 12);
    printf("Returned buffer: %s", buffer);

    cgetc();

    return 0;
}