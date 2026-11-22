; Prints Hello World , HALT printed after it is done by VM

.ORIG x3000
; Hello World
LEA R0, MSG
PUTS
HALT
MSG .STRINGZ "Hello, World!\n"
.END