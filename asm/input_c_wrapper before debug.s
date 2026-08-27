; input_c_wrapper.s
;
; cc65 calling-convention wrapper for the recovered 1990 line editor.
;
; Original public interface:
;
;   editline(string, maxlength)
;   editline2(string, maxlength, string_pos)
;
; Both return a pointer to struct inpstat.
;
; The original Aztec assembly fetched C parameters directly from
; the Aztec C software stack. This wrapper performs only the
; equivalent cc65 calling-convention translation.

.setcpu "65C02"

.export _editline
.export _editline2

.import INPUT
.import START_POS
.import INP_STAT

.import popa
.import popax


.segment "CODE"


;
; struct inpstat* __fastcall__
; editline(char* string, unsigned char maxlength)
;
; On entry:
;   A = maxlength
;   string is on the cc65 C stack
;

_editline:
        sta MAX_LENGTH

        ; Get string pointer from cc65 C stack.
        ;
        ; popax returns:
        ;   A = low byte
        ;   X = high byte

        jsr popax

        ; Original INPUT interface wants:
        ;
        ;   Y:A = string address
        ;   X   = maximum length

        pha
        txa
        tay
        pla

        ldx MAX_LENGTH

        ; Original editline_ always starts at position zero.

        pha
        lda #$00
        sta START_POS
        pla

        jsr INPUT

        ; Original routine returned a pointer to inp_stat.
        ;
        ; cc65 returns a pointer in A/X.

        lda #<INP_STAT
        ldx #>INP_STAT
        rts


;
; struct inpstat* __fastcall__
; editline2(
;     char* string,
;     unsigned char maxlength,
;     unsigned char string_pos
; )
;
; On entry:
;   A = string_pos
;
; Remaining cc65 C-stack arguments, from top:
;   maxlength
;   string low
;   string high
;

_editline2:
        sta START_POS

        ; maxlength is now the top stacked argument.

        jsr popa
        sta MAX_LENGTH

        ; Then get string pointer.

        jsr popax

        ; Convert A/X pointer to original Y:A convention.

        pha
        txa
        tay
        pla

        ldx MAX_LENGTH

        jsr INPUT

        ; Return pointer to inp_stat.

        lda #<INP_STAT
        ldx #>INP_STAT
        rts


.segment "BSS"

MAX_LENGTH:
        .res 1