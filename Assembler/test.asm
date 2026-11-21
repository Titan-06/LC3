.ORIG x3000

; Clear R1 using AND
AND R1, R1, #0

; Loop 5 times (using R1 as counter)
LOOP ADD R1, R1, R2      ; Increment counter
     ADD R2, R1, #0     ; Copy counter to R2 to check condition
     BRp LOOP           ; If positive, repeat (infinite loop if counter doesn't overflow)

.END   