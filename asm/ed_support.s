;
; ed_support.s
;
; cc65 replacements for editor support routines that were supplied
; by the original -led library.
;
; The final ED.LIB source has not survived.
;
; Benchmark policy:
;   - preserve known original behavior
;   - use recovered implementation where available
;   - infer only the minimum required behavior from original callers
;
; Initial routines:
;   home()
;   text()
;   crout()
;   gotoxy(x,y)
;   print(string)
;   println(string)
;   blank_line()
;   solid_line()
;
; scroll_up() and scroll_dn() will be added after these routines
; have been tested.
;
; Target: Apple II enhanced / 65C02
;

.setcpu "65C02"

.export _home
.export _text
.export _crout
.export _gotoxy
.export _print
.export _println
.export _blank_line
.export _solid_line
.export _scroll_up
.export _scroll_dn

.import popa


;
; ---------------------------------------------------------------------------
; Apple II screen state
; ---------------------------------------------------------------------------
;

WNDLFT      = $20
WNDWDTH     = $21
WNDTOP      = $22
WNDBTM      = $23

CH40        = $24
CV          = $25

BASL        = $28
INVFLG      = $32

CH80        = $057B


;
; ---------------------------------------------------------------------------
; Apple II soft switches
; ---------------------------------------------------------------------------
;

RD80COL     = $C01F

STORE80ON   = $C001
PAGE2OFF    = $C054
PAGE2ON     = $C055

LC_ROMIN    = $C082
LC_BANK2_RO = $C080


;
; ---------------------------------------------------------------------------
; Apple II Monitor ROM
; ---------------------------------------------------------------------------
;

SETTXT      = $FB39
VTAB        = $FC22
HOME_ROM    = $FC58
CROUT_ROM   = $FD8E


;
; ---------------------------------------------------------------------------
; ROM_CALL
;
; The cc65 Apple II runtime leaves Language Card bank 2 mapped into
; $D000-$FFFF while C code is executing. Therefore, any call into the
; Apple II Monitor ROM must temporarily page the ROM back in.
;
; Restore LC bank 2 read-only immediately after the ROM routine returns.
;
; ---------------------------------------------------------------------------
;

.macro ROM_CALL addr
        bit LC_ROMIN
        jsr addr
        bit LC_BANK2_RO
.endmacro


;
; ---------------------------------------------------------------------------
; Other working storage
; ---------------------------------------------------------------------------
;

.segment "BSS"

RIGHT_MARGIN:
        .res 1

LINE_COUNT:
        .res 1

SAVE_CH40:
        .res 1

SAVE_CH80:
        .res 1
        
SCROLL_ROW:
        .res 1

RIGHT_EDGE:
        .res 1


.segment "CODE"


;
; ---------------------------------------------------------------------------
; home()
;
; Clear the current text window and home the cursor.
; ---------------------------------------------------------------------------
;

_home:
        ROM_CALL HOME_ROM
        rts


;
; ---------------------------------------------------------------------------
; text()
;
; Put the Apple II into text mode.
; ---------------------------------------------------------------------------
;

_text:
        ROM_CALL SETTXT
        rts


;
; ---------------------------------------------------------------------------
; crout()
;
; Output a carriage return through the Monitor ROM.
; ---------------------------------------------------------------------------
;

_crout:
        ROM_CALL CROUT_ROM
        rts


;
; ---------------------------------------------------------------------------
; gotoxy(unsigned char x, unsigned char y)
;
; Declared __fastcall__ in C.
;
; cc65 passes:
;
;       y in A
;       x on the cc65 software stack
;
; Keep both the standard 40-column horizontal cursor and the
; 80-column horizontal cursor synchronized.
; ---------------------------------------------------------------------------
;

_gotoxy:
        sta CV                  ; Y coordinate

        jsr popa                ; X coordinate
        sta CH40
        sta CH80

        ROM_CALL VTAB           ; establish BASL for CV
        rts


;
; ---------------------------------------------------------------------------
; print(const char *string)
;
; Declared __fastcall__.
;
; cc65 passes pointer:
;
;       A = low byte
;       X = high byte
;
; The surviving MY.LIB implementation used self-modifying code for
; its string pointer. Preserve that approach here.
; ---------------------------------------------------------------------------
;

_print:
        sta @get_char+1
        stx @get_char+2

        ldy #0

@get_char:
        lda $FFFF,y             ; operand patched above
        beq @done

        ora #$80
        jsr PUTCHAR_SCREEN

        iny
        bne @get_char

@done:
        rts


;
; ---------------------------------------------------------------------------
; println(const char *string)
;
; Original observed behavior:
;
;       print(string)
;       crout()
; ---------------------------------------------------------------------------
;

_println:
        jsr _print
        ROM_CALL CROUT_ROM
        rts


;
; ---------------------------------------------------------------------------
; PUTCHAR_SCREEN
;
; Internal.
;
; Entry:
;       A = character, high bit already set
;
; Writes at the current horizontal cursor position.
;
; IMPORTANT:
; The editor treats every source line as a separate string. It does not
; logically wrap a source line onto the next editor line.
;
; Therefore, when the right edge of the current text window is reached,
; additional characters are ignored rather than wrapped to another row.
;
; Y must be preserved because print() uses Y as its string index.
; ---------------------------------------------------------------------------
;

PUTCHAR_SCREEN:
        phx
        phy
        pha

;
; Determine the right edge of the current text window.
;
; RIGHT_MARGIN is the first column NOT in the window.
;
;       right = WNDLFT + WNDWDTH
;

        lda WNDLFT
        clc
        adc WNDWDTH
        sta RIGHT_MARGIN

;
; Determine 40- or 80-column mode.
;

        lda RD80COL
        bpl @forty_column


;
; ---------------------------------------------------------------------------
; 80-column output
; ---------------------------------------------------------------------------
;

        ldx CH80
        cpx RIGHT_MARGIN
        bcs @discard

        pla
        jsr DISP_CHAR_80

;
; Advance horizontal position.
;
; It is okay for CH80 to become RIGHT_MARGIN. The next character will
; then be rejected by the test above.
;

        inc CH80

;
; Keep CH40 synchronized with the logical 80-column coordinate. The
; surviving INPUT.ASM similarly maintains both cursor variables.
;

        lda CH80
        sta CH40

        bra @exit


;
; ---------------------------------------------------------------------------
; 40-column output
; ---------------------------------------------------------------------------
;

@forty_column:
        ldx CH40
        cpx RIGHT_MARGIN
        bcs @discard

        pla
        jsr DISP_CHAR_40

        inc CH40

        lda CH40
        sta CH80

        bra @exit


;
; Character fell beyond the right margin.
;

@discard:
        pla


@exit:
        ply
        plx
        rts


;
; ---------------------------------------------------------------------------
; DISP_CHAR_80
;
; Entry:
;
;       X = logical 80-column X coordinate
;       A = Apple II screen character
;
; Uses the same interleaved main/auxiliary screen-memory mechanism as
; the surviving INPUT.ASM:
;
;       even logical columns -> auxiliary screen memory
;       odd  logical columns -> main screen memory
;
; BASL must already point at the current screen row.
;
; Preserves X.
; ---------------------------------------------------------------------------
;

DISP_CHAR_80:
        phx
        pha

        txa
        lsr a                   ; physical byte offset = X / 2
        bcs @main_memory

;
; Even logical column: auxiliary video memory.
;

        sta STORE80ON
        sta PAGE2ON

@main_memory:
        tay

        pla

        jsr NORMALIZE_CHAR

        sta (BASL),y

;
; Always leave main video page selected.
;

        sta PAGE2OFF

        plx
        rts


;
; ---------------------------------------------------------------------------
; DISP_CHAR_40
;
; Entry:
;
;       X = 40-column X coordinate
;       A = Apple II screen character
;
; BASL must already point at the current screen row.
;
; Preserves X.
; ---------------------------------------------------------------------------
;

DISP_CHAR_40:
        phx
        pha

        txa
        tay

        pla

        jsr NORMALIZE_CHAR

        sta (BASL),y

        plx
        rts


;
; ---------------------------------------------------------------------------
; NORMALIZE_CHAR
;
; Reproduces the character handling recovered from the older fTprint
; module in MY.LIB.
;
; Entry:
;       A = Apple II display character
;
; Exit:
;       A = character to place into screen memory
;
; INVFLG is the Apple II Monitor inverse/normal-display flag.
; ---------------------------------------------------------------------------
;

NORMALIZE_CHAR:
        phx

        ldx INVFLG
        cpx #$FF
        beq @normal

        cmp #$E0
        bcs @lower_case

        and #$BF
        bra @done

@lower_case:
        and #$7F

@normal:
@done:
        plx
        rts


;
; ---------------------------------------------------------------------------
; blank_line()
;
; Calling context in the original editor includes:
;
;       gotoxy(0,0);
;       blank_line();
;
; Blank the current screen row across the active text window.
;
; Uses WNDLFT and WNDWDTH rather than assuming a fixed width.
; Supports both 40- and 80-column modes.
;
; Preserve the caller's horizontal cursor position.
; ---------------------------------------------------------------------------
;

_blank_line:
        lda CH40
        sta SAVE_CH40

        lda CH80
        sta SAVE_CH80

        lda WNDWDTH
        sta LINE_COUNT

        beq @blank_done

        lda RD80COL
        bpl @blank40


;
; 80-column mode
;

        ldx WNDLFT

@blank80_loop:
        lda #$A0                ; normal space
        jsr DISP_CHAR_80

        inx
        dec LINE_COUNT
        bne @blank80_loop

        bra @blank_done


;
; 40-column mode
;

@blank40:
        ldx WNDLFT

@blank40_loop:
        lda #$A0
        jsr DISP_CHAR_40

        inx
        dec LINE_COUNT
        bne @blank40_loop


@blank_done:
        lda SAVE_CH40
        sta CH40

        lda SAVE_CH80
        sta CH80

        rts


;
; ---------------------------------------------------------------------------
; solid_line()
;
; Original calling context:
;
;       gotoxy(0,1);
;       solid_line();
;
;       gotoxy(0,div_line);
;       solid_line();
;
; This draws the horizontal viewport divider.
;
; We do not have the final ED.LIB source, so the precise divider glyph
; is not yet known. Use '-' for the initial test build.
;
; TODO:
; If the original divider character can later be established from an
; executable, screenshot, or another surviving source, replace SOLID_CHAR.
;
; Supports both 40- and 80-column modes.
; ---------------------------------------------------------------------------
;

SOLID_CHAR = '_' | $80


_solid_line:
        lda CH40
        sta SAVE_CH40

        lda CH80
        sta SAVE_CH80

        lda WNDWDTH
        sta LINE_COUNT

        beq @solid_done

        lda RD80COL
        bpl @solid40


;
; 80-column mode
;

        ldx WNDLFT

@solid80_loop:
        lda #SOLID_CHAR
        jsr DISP_CHAR_80

        inx
        dec LINE_COUNT
        bne @solid80_loop

        bra @solid_done


;
; 40-column mode
;

@solid40:
        ldx WNDLFT

@solid40_loop:
        lda #SOLID_CHAR
        jsr DISP_CHAR_40

        inx
        dec LINE_COUNT
        bne @solid40_loop


@solid_done:
        lda SAVE_CH40
        sta CH40

        lda SAVE_CH80
        sta CH80

        rts
        
        
        ;
; ---------------------------------------------------------------------------
; scroll_up()
;
; Scroll the active text window upward by one screen row.
;
; WNDTOP = first row in scrolling window
; WNDBTM = first row BELOW scrolling window
;
; Only columns WNDLFT through WNDLFT+WNDWDTH-1 are copied.
; Supports both 40- and 80-column modes.
; ---------------------------------------------------------------------------
;

_scroll_up:
        lda WNDTOP
        sta SCROLL_ROW

@up_loop:
        lda SCROLL_ROW
        clc
        adc #1
        cmp WNDBTM
        bcs @up_blank

        ;
        ; Destination = current row
        ;
        lda SCROLL_ROW
        jsr SET_DEST_ROW

        ;
        ; Source = next row
        ;
        lda SCROLL_ROW
        inc a
        jsr SET_SOURCE_ROW

        jsr COPY_WINDOW_ROW

        inc SCROLL_ROW
        bra @up_loop

@up_blank:
        ;
        ; Blank last visible row: WNDBTM - 1
        ;
        lda WNDBTM
        dec a
        jsr SET_DEST_ROW

        jsr _blank_line
        rts


;
; ---------------------------------------------------------------------------
; scroll_dn()
;
; Scroll the active text window downward by one screen row.
;
; Copies from bottom toward top so source rows are not overwritten
; before they are used.
; ---------------------------------------------------------------------------
;

_scroll_dn:
        lda WNDBTM
        dec a
        sta SCROLL_ROW

@down_loop:
        lda SCROLL_ROW
        cmp WNDTOP
        beq @down_blank

        ;
        ; Destination = current row
        ;
        jsr SET_DEST_ROW

        ;
        ; Source = previous row
        ;
        lda SCROLL_ROW
        dec a
        jsr SET_SOURCE_ROW

        jsr COPY_WINDOW_ROW

        dec SCROLL_ROW
        bra @down_loop

@down_blank:
        ;
        ; Blank first visible row
        ;
        lda WNDTOP
        jsr SET_DEST_ROW

        jsr _blank_line
        rts


;
; ---------------------------------------------------------------------------
; SET_DEST_ROW
;
; A = screen row 0-23
;
; Set BASL/BASH ($28/$29) to the Apple II text address for that row.
; ---------------------------------------------------------------------------
;

SET_DEST_ROW:
        asl a
        tax

        lda TEXT_ROW_ADDRS,x
        sta BASL

        lda TEXT_ROW_ADDRS+1,x
        sta BASL+1

        rts


;
; ---------------------------------------------------------------------------
; SET_SOURCE_ROW
;
; A = screen row 0-23
;
; Patch the source operand used by READ_SOURCE_CHAR.
;
; This avoids consuming additional zero-page storage.
; ---------------------------------------------------------------------------
;

SET_SOURCE_ROW:
        asl a
        tax

        lda TEXT_ROW_ADDRS,x
        sta SOURCE_READ+1

        lda TEXT_ROW_ADDRS+1,x
        sta SOURCE_READ+2

        rts


;
; ---------------------------------------------------------------------------
; COPY_WINDOW_ROW
;
; Copy one screen row within the active horizontal window.
;
; Destination row is in BASL/BASH.
; Source row address has been patched into @source_read.
;
; Uses logical screen X coordinates, so the same loop works in both
; 40- and 80-column modes.
; ---------------------------------------------------------------------------
;

COPY_WINDOW_ROW:
        lda WNDLFT
        clc
        adc WNDWDTH
        sta RIGHT_EDGE

        ldx WNDLFT

@copy_loop:
        cpx RIGHT_EDGE
        bcs @copy_done

        jsr READ_SOURCE_CHAR

        ;
        ; A now contains the actual screen byte.
        ; DISP_CHAR_xx applies NORMALIZE_CHAR, which we do NOT want here:
        ; scrolling must copy the byte exactly as it exists on screen.
        ;
        jsr WRITE_DEST_CHAR_RAW

        inx
        bra @copy_loop

@copy_done:
        rts


;
; ---------------------------------------------------------------------------
; READ_SOURCE_CHAR
;
; X = logical screen column
;
; Returns the exact screen byte in A.
; Supports both 40- and 80-column modes.
;
; X is preserved.
; ---------------------------------------------------------------------------
;

READ_SOURCE_CHAR:
        lda RD80COL
        bpl READ_SOURCE_40

        ;
        ; 80-column mode
        ;
        txa
        lsr a                   ; physical byte offset = X / 2
        bcs READ_SOURCE_80_MAIN

        ;
        ; Even logical column -> auxiliary video memory
        ;
        sta STORE80ON
        sta PAGE2ON

READ_SOURCE_80_MAIN:
        tay

SOURCE_READ:
        lda $FFFF,y             ; patched by SET_SOURCE_ROW

        ;
        ; Always restore main video memory selection.
        ;
        sta PAGE2OFF
        rts


READ_SOURCE_40:
        txa
        tay

        ;
        ; Same patched source address.
        ;
        jmp SOURCE_READ


;
; ---------------------------------------------------------------------------
; WRITE_DEST_CHAR_RAW
;
; X = logical destination screen column
; A = exact screen byte to write
;
; Unlike DISP_CHAR_40 / DISP_CHAR_80, this routine does NOT call
; NORMALIZE_CHAR. Scrolling must preserve inverse/lowercase/etc. exactly.
;
; X is preserved.
; ---------------------------------------------------------------------------
;

WRITE_DEST_CHAR_RAW:
        pha

        lda RD80COL
        bpl @write40

        ;
        ; 80-column mode
        ;
        txa
        lsr a
        bcs @write80_main

        ;
        ; Even logical column -> auxiliary video memory
        ;
        sta STORE80ON
        sta PAGE2ON

@write80_main:
        tay
        pla

        sta (BASL),y

        ;
        ; Always restore main video memory selection.
        ;
        sta PAGE2OFF
        rts


@write40:
        txa
        tay
        pla

        sta (BASL),y
        rts


;
; ---------------------------------------------------------------------------
; Apple II text-page row addresses.
;
; Only the actual 24 displayed rows are represented, so scrolling never
; writes through the screen holes.
; ---------------------------------------------------------------------------
;

TEXT_ROW_ADDRS:
        .word $0400
        .word $0480
        .word $0500
        .word $0580
        .word $0600
        .word $0680
        .word $0700
        .word $0780

        .word $0428
        .word $04A8
        .word $0528
        .word $05A8
        .word $0628
        .word $06A8
        .word $0728
        .word $07A8

        .word $0450
        .word $04D0
        .word $0550
        .word $05D0
        .word $0650
        .word $06D0
        .word $0750
        .word $07D0