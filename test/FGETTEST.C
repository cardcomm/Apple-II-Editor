#include <stdio.h>

int main(void)
{
    FILE *f;
    char buf[80];
    unsigned char *p;
    int line = 0;

    f = fopen("SAVETEST.C", "r");

    if (f == NULL) {
        printf("OPEN FAILED\n");
        return 1;
    }

    while (fgets(buf, sizeof(buf), f) != NULL) {
        ++line;

        printf("LINE %d:", line);

        p = (unsigned char *)buf;

        while (*p) {
            printf(" %02X", *p);
            ++p;
        }

        printf("\n");
    }

    fclose(f);
    return 0;
}