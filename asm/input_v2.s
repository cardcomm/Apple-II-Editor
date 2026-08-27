; input_v2.s
;
; ca65 port of the recovered 03/27/90 INPUT assembly source.
;
; This is the later version used by the Aztec C full-screen editor.
;
; Original interface preserved:
;
;   Y:A = address of zero-terminated text buffer
;   X   = maximum editable length
;
; INPUT returns after:
;   - Return
;   - Escape
;   - an unhandled control character, including Up/Down
;   - an unhandled Open-Apple command
;
; Details are returned through INP_STAT.
;
; Target: 65C02 / enhanced Apple II family

.setcpu "65C02"

.export INPUT
.export INPUT1_1
.export RD_KEY
.export INIT_INP
.export _bell

_bell:
        jmp BELL

.export START_POS
.export INP_STAT

.importzp ptr1


.segment "CODE"


;
; APPLE II / SCREEN EQUATES
;

SCRNWIDTH = $21
YCOOR     = $25
CH80      = $057B
CH40      = $24

KEYBOARD  = $C000
STROBE    = $C010

BASL      = $28


;
; APPLE II SCREEN CHARACTER CODES
;

NORMAL_SPACE  = $A0
INVERSE_SPACE = $20
UNDERLINE     = $DF


;
; cc65 zero-page temporary pointer.
;
; The original used an Aztec C pseudo-register supplied by zpage.h.
; ptr1 is the cc65 equivalent temporary pointer for this port.
;

TMP_PTR = ptr1


;
; SUB TO INPUT A LINE OF CHARACTERS
;
; ENTER WITH MAX LENGTH IN X
; AND ADDRESS OF STRING VARIABLE IN Y:A
;
; EXIT WITH NEW STRING IN VARIABLE
; AND DETAILS ABOUT KEYS PRESSED IN INP_STAT
;

INPUT:
        jsr INIT_INP

INPUT1:
        jsr RD_KEY

INPUT1_1:
        cmp #$8D                    ; RETURN?
        bne I_0

        and #%01111111             ; clear high bit
        sta CONTROL_CR             ; pass back to caller
        jmp INP_EXIT


I_0:
        cmp #$88                    ; LEFT ARROW?
        bne I_1
        jmp CUR_LEFT


I_1:
        cmp #$95                    ; RIGHT ARROW?
        bne I_2
        jmp CUR_RIGHT


I_2:
        cmp #$FF                    ; DELETE?
        bne S_1
        jmp DELETE


S_1:
        cmp #$84                    ; CONTROL-D?
        bne I_3
        jmp DELETE


I_3:
        cmp #$99                    ; CONTROL-Y?
        bne I_4
        jmp APPLE_T                 ; truncate input line


I_4:
        cmp #$9B                    ; ESC?
        bne I_5
        jmp NOSVE_E


I_5:
        cmp #$89                    ; TAB?
        bne I_6
        jmp DO_TAB


I_6:
        cmp #$85                    ; CONTROL-E?
        bne I_7
        jmp APPLE_E                 ; change edit mode


;
; Any other control character is returned to the caller.
;
; This is how UP/DOWN are passed back:
;
;   $8A -> $0A
;   $8B -> $0B
;

I_7:
        cmp #$9F                    ; control char we don't use?
        bcs I_8

        and #$7F                    ; clear high bit
        sta CONTROL_CR
        jmp INP_EXIT


I_8:
        ldy APPLE
        cpy #$80
        bcc I_9
        jmp CHK_APPLE               ; handle Open-Apple commands


I_9:
        ldy CURSOR                  ; check current mode
        cpy #INVERSE_SPACE
        bne INSERT


;
; OVERSTRIKE MODE
;

OVER_STK:
        ldx XCOOR
        cpx MAX_RT
        bne O_1

        jsr BELL
        jmp OV_EXIT


O_1:
        jsr DISP_CHAR
        inc XCOOR

        lda #1                      ; signal string changed
        sta CHANGED


OV_EXIT:
        jmp INPUT1


;
; SUBROUTINE EXIT
;

INP_EXIT:
        jsr SAVE_LINE
        jmp SUB_EXIT


;
; ESCAPE EXIT
;
; Exit without updating the caller's string.
;

NOSVE_E:
        lda #1
        sta ESC_STAT

        lda #0
        sta CHANGED


SUB_EXIT:
        lda XCOOR
        sta LAST_XCOR
        rts


;
; INSERT MODE
;

INSERT:
        pha                         ; save input char

        lda XCOOR
        cmp MAX_RT
        bcs INSERT_ERR

        ldx MAX_RT
        jsr READ_CHAR

        cmp #NORMAL_SPACE           ; room for another character?
        beq INSERT_OK


INSERT_ERR:
        jsr BELL
        pla
        jmp INPUT1


INSERT_OK:
        dec XCOOR
        jsr MOVE_RT                 ; make room for char
        inc XCOOR

        pla
        ldx XCOOR
        jsr DISP_CHAR

        inc XCOOR

        lda #1
        sta CHANGED

        jmp INPUT1


;
; DELETE
;

DELETE:
        lda XCOOR
        cmp MAX_LFT
        beq EXIT_D

        jsr MOVE_LT

        lda #1
        sta CHANGED


EXIT_D:
        jmp INPUT1


;
; CURSOR LEFT
;

CUR_LEFT:
        lda XCOOR
        cmp MAX_LFT
        beq EXIT_L

        dec XCOOR


EXIT_L:
        jmp INPUT1


;
; CURSOR RIGHT
;

CUR_RIGHT:
        lda XCOOR
        cmp MAX_RT
        beq EXIT_R

        inc XCOOR


EXIT_R:
        jmp INPUT1


;
; TAB
;

DO_TAB:
        lda CURSOR
        cmp #INVERSE_SPACE          ; which edit mode?
        bne TAB_INSERT_MODE

        ldy XCOOR
        lda TABTBL,y
        sta XCOOR
        jmp EXIT_TAB


TAB_INSERT_MODE:
        ldy XCOOR
        lda TABTBL,y                ; next tab stop

        sec
        sbc XCOOR                   ; number of spaces to insert
        sta COUNTER


TAB_LOOP:
        lda XCOOR
        cmp MAX_RT
        bcs TAB_NO_ROOM

        ldx MAX_RT
        jsr READ_CHAR

        cmp #NORMAL_SPACE
        beq TAB_YEP


TAB_NO_ROOM:
        jsr BELL
        jmp EXIT_TAB


TAB_YEP:
        dec XCOOR
        jsr MOVE_RT
        inc XCOOR

        lda #NORMAL_SPACE
        ldx XCOOR
        jsr DISP_CHAR

        inc XCOOR

        dec COUNTER
        bne TAB_LOOP

        lda #1
        sta CHANGED


EXIT_TAB:
        jmp INPUT1


;
; OPEN-APPLE COMMANDS
;

CHK_APPLE:
        and #%11011111              ; be sure upper case

        cmp #$C5                    ; E - Open-Apple-E?
        bne CA_1
        jmp APPLE_E


CA_1:
        cmp #$D4                    ; T
        bne CA_2
        jmp APPLE_T


CA_2:
        cmp #$D9                    ; Y
        bne CA_3
        jmp APPLE_T


;
; Unknown Open-Apple command.
;
; Clear high bit and return ASCII code to caller.
;

CA_3:
        and #$7F
        sta APPLECMD
        jmp INP_EXIT


;
; OPEN-APPLE-E / CONTROL-E
;
; Toggle insert / overstrike.
;

APPLE_E:
        lda CURSOR
        cmp #UNDERLINE
        bne SET_INS

        lda #INVERSE_SPACE
        sta CURSOR
        jmp INPUT1


SET_INS:
        lda #UNDERLINE
        sta CURSOR
        jmp INPUT1


;
; OPEN-APPLE-T / OPEN-APPLE-Y / CONTROL-Y
;
; Truncate line at cursor.
;

APPLE_T:
        ldx XCOOR


APT_1:
        lda #NORMAL_SPACE
        jsr DISP_CHAR

        cpx MAX_RT
        beq T_EXIT

        inx
        bne APT_1                   ; always taken


T_EXIT:
        lda #1
        sta CHANGED

        jmp INPUT1


;
; WAIT FOR KEYPRESS WHILE DISPLAYING BLINKING CURSOR
;
; EXIT:
;   A = input character
;   X >= $80 if Open-Apple was held
;

RD_KEY:
        lda #NORMAL_SPACE
        sta APPLECMD                ; clear Open-Apple command status

        ldx XCOOR
        jsr READ_CHAR
        sta CURRENT


GET_LOOP:
        lda CURSOR
        cmp #INVERSE_SPACE
        bne L_1

        lda CURRENT
        cmp #$E1                    ; lower case?
        bcc L_2

        sta $C00F                   ; ALTCHARSETON
        and #$7F
        jmp L_1


L_2:
        and #$3F


L_1:
        ldx XCOOR
        jsr DISP_CHAR               ; display cursor

        ldx #$FF
        ldy #$50


G_LOOP:
        lda KEYBOARD
        bmi GOT_KEY

        dex
        bne G_LOOP

        dey
        bne G_LOOP


        ldx XCOOR
        lda CURRENT
        jsr DISP_CHAR

        ldx #$FF
        ldy #$C0


G_LOOP1:
        lda KEYBOARD
        bmi GOT_KEY

        dex
        bne G_LOOP1

        dey
        bne G_LOOP1

        jmp GET_LOOP


GOT_KEY:
        lda CURRENT
        ldx XCOOR
        jsr DISP_CHAR

        lda $C061                   ; Open-Apple status
        sta APPLE
        tax

        lda KEYBOARD
        sta STROBE

        cpx #$80
        bcc RD_KEY_EXIT

        sta APPLECMD                ; save raw Open-Apple key


RD_KEY_EXIT:
        rts


;
; INITIALIZE INPUT SUBROUTINE
;

INIT_INP:
        sty TMP_PTR+1
        sta TMP_PTR
        stx LINE_LNG

        lda $C01F                   ; 80COL minus if 80-col on
        bpl INIT_40

        lda CH80
        bne INIT_GOT_X              ; always taken


INIT_40:
        lda CH40


INIT_GOT_X:
        sta XCOOR
        sta MAX_LFT

        clc
        adc LINE_LNG

        cmp SCRNWIDTH
        bcc INIT_RIGHT_OK

        lda SCRNWIDTH


INIT_RIGHT_OK:
        sta MAX_RT

        jsr DISP_LINE


        ;
        ; Later editor version supports entering the line at
        ; an existing horizontal position.
        ;

        lda XCOOR
        clc
        adc START_POS

        sta XCOOR
        sta CH80
        sta CH40


        ;
        ; Initialize returned status structure.
        ;

        lda #0

        sta ESC_STAT
        sta CONTROL_CR
        sta CHANGED
        sta LAST_XCOR

        rts


;
; DISPLAY CURRENT STRING ON SCREEN
;

DISP_LINE:
        ldy #0
        sty YTMP

        ldx MAX_LFT


DL_1:
        ldy YTMP
        lda (TMP_PTR),y

        beq DL_EXIT

        ora #%10000000
        jsr DISP_CHAR

        cpx MAX_RT
        beq DL_EXIT

        inx
        inc YTMP

        bne DL_1                    ; always taken


DL_EXIT:
        rts


;
; MOVE STRING FROM SCREEN BACK INTO CALLER'S BUFFER
;

SAVE_LINE:
        ldy #0
        sty Y_TEMP

        ldx MAX_LFT


SV_1:
        jsr READ_CHAR

        ldy Y_TEMP

        and #%01111111              ; clear screen high bit
        sta (TMP_PTR),y

        cpx MAX_RT
        beq SV_EXIT

        inc Y_TEMP
        inx

        jmp SV_1


SV_EXIT:
        ldy Y_TEMP
        iny

        lda #0
        sta (TMP_PTR),y

        rts


;
; DISPLAY CHARACTER
;
; ENTRY:
;   X = screen X coordinate
;   A = screen character
;

DISP_CHAR:
        pha

        ldy $C01F
        cpy #$80
        bcc DISP_40

        txa
        lsr a
        bcs DISP_ODD

        ;
        ; Even 80-column positions use auxiliary video memory.
        ;

        sta $C001                   ; 80STOREON
        sta $C055                   ; PAGE2ON


DISP_ODD:
        tay

        pla
        sta (BASL),y

        sta $C054                   ; PAGE2OFF
        rts


;
; 40-COLUMN MODE
;

DISP_40:
        txa
        tay

        pla
        sta (BASL),y

        rts


;
; READ CHARACTER FROM SCREEN
;
; ENTRY:
;   X = screen X coordinate
;
; EXIT:
;   A = screen character
;

READ_CHAR:
        lda $C01F
        bpl GET_40

        txa
        lsr a
        bcs READ_ODD

        sta $C001                   ; 80STOREON
        sta $C055                   ; PAGE2ON


READ_ODD:
        tay

        lda (BASL),y

        sta $C054                   ; PAGE2OFF
        rts


;
; 40-COLUMN MODE
;

GET_40:
        txa
        tay

        lda (BASL),y
        rts


;
; MOVE CHARACTERS RIGHT ONE POSITION
;
; Used by insert mode.
;

MOVE_RT:
        ldx MAX_RT
        jsr READ_CHAR

        jsr IS_SPACE
        bne BELL_EX

        ldx MAX_RT
        dex
        stx INDEX


MOVE1:
        ldx INDEX
        jsr READ_CHAR

        inc INDEX
        ldx INDEX
        jsr DISP_CHAR

        dec INDEX
        dec INDEX

        lda INDEX
        cmp XCOOR

        bne MOVE1
        beq CLEAN_EX                ; always taken


BELL_EX:
        jsr BELL


CLEAN_EX:
        rts


;
; CHECK FOR SCREEN SPACE
;

IS_SPACE:
        cmp #INVERSE_SPACE
        beq IS_SPACE_EXIT

        cmp #NORMAL_SPACE


IS_SPACE_EXIT:
        rts


;
; MOVE CHARACTERS LEFT ONE POSITION
;
; Used by delete/backspace.
;

MOVE_LT:
        lda XCOOR
        beq BELL_EX

        sta INDEX


MOVE_L:
        ldx INDEX
        jsr READ_CHAR

        dec INDEX
        ldx INDEX
        jsr DISP_CHAR

        inc INDEX
        inc INDEX

        ldy MAX_RT
        iny

        cpy INDEX
        bne MOVE_L

        bra CLEAN_EX1


BELL_EX1:
        jsr BELL
        jmp CLEAN_EX2


CLEAN_EX1:
        ldy XCOOR
        dey
        sty XCOOR

        lda #NORMAL_SPACE
        ldx MAX_RT
        jsr DISP_CHAR               ; blank far-right character


CLEAN_EX2:
        rts


;
; SOFT BELL
;
; Retained from the earlier recovered INPUT source so this module
; remains self-contained under cc65.
;

BELL:
        lda #$15
        ldx #$F0
        jsr TONE
        rts


TONE:
        sta DURATION
        stx PITCH


TONE1:
        lda $C030


TONE_LOOP:
        dey
        bne TONE_CONTINUE

        dec DURATION
        beq TONE_EXIT


TONE_CONTINUE:
        dex
        bne TONE_LOOP

        ldx PITCH
        jmp TONE1


TONE_EXIT:
        rts


;
; TAB STOP TABLE
;
; Original tab size = 4.
;

.segment "RODATA"

TABTBL:
        .byte  4, 4, 4, 4
        .byte  8, 8, 8, 8
        .byte 12,12,12,12
        .byte 16,16,16,16
        .byte 20,20,20,20
        .byte 24,24,24,24
        .byte 28,28,28,28
        .byte 32,32,32,32
        .byte 36,36,36,36
        .byte 40,40,40,40
        .byte 44,44,44,44
        .byte 48,48,48,48
        .byte 52,52,52,52
        .byte 56,56,56,56
        .byte 60,60,60,60
        .byte 64,64,64,64
        .byte 68,68,68,68
        .byte 72,72,72,72
        .byte 76,76,76,76
        .byte 79,79,79,79


;
; WORKING VARIABLES
;

.segment "BSS"

START_POS:
        .res 1

MAX_LFT:
        .res 1

MAX_RT:
        .res 1

APPLE:
        .res 1

LINE_LNG:
        .res 1

XCOOR:
        .res 1


;
; inpstat
;
; IMPORTANT: order must remain exactly synchronized with ed.h.
;

INP_STAT:

APPLECMD:
        .res 1

ESC_STAT:
        .res 1

CONTROL_CR:
        .res 1

CHANGED:
        .res 1

LAST_XCOR:
        .res 1


CURRENT:
        .res 1

YTMP:
        .res 1

Y_TEMP:
        .res 1

INDEX:
        .res 1

COUNTER:
        .res 1

DURATION:
        .res 1

PITCH:
        .res 1
        
        
.segment "DATA"

CURSOR:
        .byte INVERSE_SPACE