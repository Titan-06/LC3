.ORIG x3000
; Line 4: FAR is 301 words after this LEA, but a 9-bit offset only reaches +255
        AND R0, R0, #0
        LEA R0, FAR
        HALT
PAD     .BLKW 300
FAR     .FILL x1234
.END