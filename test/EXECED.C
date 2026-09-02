#include <unistd.h>
#include <stdio.h>

int main(void)
{
    if (exec("ED", "WT.C") == -1) {
        printf("exec failed\n");
    }

    return 0;
}