; input_test.s
; Minimal assembly-only harness for the standalone line editor.
; BRUN the linked binary from ProDOS BASIC.SYSTEM.

.setcpu "65C02"

.import INIT_INP, INPUT1_1

HOME    = $FC58
CH      = $24
CV      = $25

.segment "CODE"

start:
        jsr HOME

        ; Start editing at column 0, row 10.
        lda #0
        sta CH
        lda #10
        sta CV
        jsr $FB5B              ; VTAB

        ; Original interface:
        ;   Y:A = buffer address
        ;   X   = maximum line length
        lda #<buffer
        ldy #>buffer
        ldx #60
        jsr INIT_INP
        jsr INPUT1_1

        rts                    ; return to BASIC.SYSTEM

.segment "DATA"

buffer:
        .byte "THIS IS A TEST", 0
        .res 61, 0
