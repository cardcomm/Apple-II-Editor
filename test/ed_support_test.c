void __fastcall__ text(void);
void __fastcall__ home(void);
void __fastcall__ gotoxy(unsigned char x, unsigned char y);
void __fastcall__ print(const char *string);
void __fastcall__ solid_line(void);
void __fastcall__ scroll_dn(void);

#define WNDTOP (*(unsigned char *)0x22)
#define WNDBTM (*(unsigned char *)0x23)

int main(void)
{
    text();
    home();

    WNDTOP = 4;
    WNDBTM = 10;

    gotoxy(0, 3);
    solid_line();

    gotoxy(0, 4);
    print("ROW 4");

    gotoxy(0, 5);
    print("ROW 5");

    gotoxy(0, 6);
    print("ROW 6");

    gotoxy(0, 7);
    print("ROW 7");

    gotoxy(0, 8);
    print("ROW 8");

    gotoxy(0, 9);
    print("ROW 9");

    gotoxy(0, 10);
    solid_line();

    scroll_dn();

    return 0;
}