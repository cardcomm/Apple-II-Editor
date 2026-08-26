/*
    func. to wait for a keypress
    returns keypress as an int.
*/

int waitkey(void)
{
    char* keyboard = (char*)0xC000;
    char* strobe   = (char*)0xC010;
    int c;

    while(*keyboard < 128);

    c = *keyboard - 128;
    *strobe = 0;

    return c;
}