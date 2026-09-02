#include <unistd.h>

#define RD80COL (*(volatile unsigned char *)0xC01F)

int main(void)
{
    if(RD80COL & 0x80)
        exec("ED", "WT.C 80");
    else
        exec("ED", "WT.C 40");

    return 0;
}