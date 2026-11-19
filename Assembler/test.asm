.ORIG x3000

; Test 1: Basic label and arithmetic instruction
START       ADD R1, R1, #1
            ADD R2, R2, R1

; Test 2: Label used in branch (BR)
            BRz SKIP_ADD
            ADD R3, R3, #10
SKIP_ADD    ADD R4, R4, #20

; Test 3: Label used in load/store (LD, ST)
            LD R5, DATA_ADDR
            ST R5, RESULT_LOC

; Test 4: Data directives (.FILL, .BLKW, .STRINGZ)
DATA_ADDR   .FILL x1234
RESULT_LOC  .BLKW 1
MSG         .STRINGZ "Hello"

; Test 5: Indirect load/store and LEA (LDI, STI, LDI, LEA)
            LEA R0, MSG
            LDI R0, INDIRECT_PTR
            STI R0, INDIRECT_STORE

INDIRECT_PTR .FILL x3010
INDIRECT_STORE .BLKW 1

; Test 6: JSR/JSRR (Subroutine calls)
            JSR SUBROUTINE
            JSRR R7

SUBROUTINE  NOT R0, R0
            RET

; Test 7: Trap instructions
            TRAP x25    ; HALT

.END     