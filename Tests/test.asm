; Main encoding Test
; Tests Everything but you can't run it

.ORIG x3000
; arithmetic and logic
START AND R0, R0, #0
ADD R0, R0, #5
ADD R1, R0, R0
AND R2, R1, #-1
NOT R3, R2
; memory
LD R0, DATA1
LDI R1, PTR
LEA R2, MSG
ST R0, DATA1
STI R1, PTR
LDR R3, R2, #1
STR R3, R2, #-2
; branches
LOOP ADD R0, R0, #-1
BRp LOOP
BRz SKIP
BRn SKIP
BRnzp SKIP
ADD R0, R0, #1
; subroutines and jumps
SKIP JSR SUB
JSRR R4
JMP R5
RET
RTI
; traps
GETC
OUT
PUTS
IN
PUTSP
TRAP x25
HALT
SUB ADD R6, R6, #1
RET
; data
DATA1 .FILL x1234
PTR .FILL #-1
ARR .BLKW 3
BIN .FILL b101
MSG .STRINGZ "Hi"
.END