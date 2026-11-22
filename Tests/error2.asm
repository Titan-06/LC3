.ORIG x3000
; Line 3 is fine
AND R0, R0, #0
; Line 5 has a bad register (R8 does not exist)
ADD R1, R8, #1
HALT
.END