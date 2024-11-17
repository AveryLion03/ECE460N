.ORIG x3000
LEA R1, THIRD_INT_ADDR, #0
LEA R3, THIRD_INT_ADDR, #0
LEA R4, DOIT, #0
JSRR R4
HALT

DOIT LDB R2, R1, #0    ; Load first integer
STB R2, R3, #2
XOR R5, R2, R2
XOR R6, R2, #8
ADD R5, R5, #-4
ADD R5, R5, #10
RSHFA R5, R5, #1
RET

; Variables
FIRST_INT_ADDR .FILL x0802
SECOND_INT_ADDR .FILL x4050
THIRD_INT_ADDR .FILL x1009
.END
