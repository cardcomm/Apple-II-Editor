#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <apple2.h>

int main(void)
{
    int fd;

    static const unsigned char data[] = {
        'O','N','E',0x0D,
        'T','W','O',0x0D,
        'T','H','R','E','E',0x0D
    };

    _filetype = PRODOS_T_TXT;
    _auxtype  = PRODOS_AUX_T_TXT_SEQ;

    fd = open(
        "FWROUT2.C",
        O_WRONLY | O_CREAT | O_TRUNC
    );

    if(fd == -1) {
        printf("OPEN FAILED\n");
        return 1;
    }

    if(write(fd, data, sizeof(data)) == -1) {
        printf("WRITE FAILED\n");
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}