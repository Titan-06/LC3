.ORIG x3000
; Prints the first 12 Fibonacci numbers: 0 1 1 2 3 5 8 13 21 34 55 89
        LD   R1, COUNT          ; R1 = terms left to print
        AND  R2, R2, #0         ; a = 0
        AND  R3, R3, #0
        ADD  R3, R3, #1         ; b = 1

LOOP    ADD  R0, R2, #0         ; R0 = a
        JSR  PRINT_NUM          ; print a
        LD   R0, SPACE
        OUT
        ADD  R4, R2, R3         ; next = a + b
        ADD  R2, R3, #0         ; a = b
        ADD  R3, R4, #0         ; b = next
        ADD  R1, R1, #-1
        BRp  LOOP

        LD   R0, NEWLINE
        OUT
        HALT

; PRINT_NUM: prints R0 (0-99) in decimal. Clobbers R0, R4, R5, R7.
PRINT_NUM
        ST   R7, SAVE_R7        ; OUT overwrites R7, so save it
        AND  R4, R4, #0         ; tens = 0
TENS    ADD  R5, R0, #-10
        BRn  ONES               ; fewer than 10 left
        ADD  R0, R5, #0         ; R0 -= 10
        ADD  R4, R4, #1         ; tens++
        BRnzp TENS
ONES    ADD  R5, R0, #0         ; R5 = ones digit
        ADD  R4, R4, #0
        BRz  SKIP_TENS          ; no leading zero
        LD   R0, ASCII_0
        ADD  R0, R0, R4
        OUT
SKIP_TENS
        LD   R0, ASCII_0
        ADD  R0, R0, R5
        OUT
        LD   R7, SAVE_R7
        RET

COUNT   .FILL #12
SPACE   .FILL x20
NEWLINE .FILL x0A
ASCII_0 .FILL x30
SAVE_R7 .BLKW 1
.END