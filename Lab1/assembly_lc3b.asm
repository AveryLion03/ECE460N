.ORIG x3000

; Positive test cases
ADD R0, R0, #1      ; 0x3000
AND R1, R1, #0      ; 0x3001
BRz POSITIVE        ; 0x3002
BRn NEGATIVE        ; 0x3003
BRp POSITIVE        ; 0x3004
BRzp POSITIVE       ; 0x3005
BRnp NEGATIVE       ; 0x3006
BRnz NEGATIVE       ; 0x3007
BRnzp POSITIVE      ; 0x3008
HALT                ; 0x3009

POSITIVE
JMP R7              ; 0x300A
JSR SUBROUTINE      ; 0x300B
JSRR R7             ; 0x300C
LDB R2, R3, #0      ; 0x300D
LDW R4, R5, #1      ; 0x300E
LEA R6, LABEL       ; 0x300F
NOP                 ; 0x3010
NOT R7, R7          ; 0x3011
RET                 ; 0x3012
LSHF R0, R0, #1     ; 0x3013
RSHFL R1, R1, #1    ; 0x3014
RSHFA R2, R2, #1    ; 0x3015
RTI                 ; 0x3016
STB R3, R4, #0      ; 0x3017
STW R5, R6, #1      ; 0x3018
TRAP x25            ; 0x3019
XOR R7, R7, R7      ; 0x301A

; Negative test cases
ADD R0, R0, #-1     ; 0x301B
AND R1, R1, #-1     ; 0x301C
BRz NEGATIVE        ; 0x301D
BRn POSITIVE        ; 0x301E
BRp NEGATIVE        ; 0x301F
BRzp NEGATIVE       ; 0x3020
BRnp POSITIVE       ; 0x3021
BRnz POSITIVE       ; 0x3022
BRnzp NEGATIVE      ; 0x3023
HALT                ; 0x3024

NEGATIVE
JMP R7              ; 0x3025
JSR SUBROUTINE      ; 0x3026
JSRR R7             ; 0x3027
LDB R2, R3, #-1     ; 0x3028
LDW R4, R5, #-1     ; 0x3029
LEA R6, LABEL       ; 0x302A
NOP                 ; 0x302B
NOT R7, R7          ; 0x302C
RET                 ; 0x302D
LSHF R0, R0, #1    ; 0x302E
RSHFL R1, R1, #1   ; 0x302F
RSHFA R2, R2, #1   ; 0x3030
RTI                 ; 0x3031
STB R3, R4, #-1     ; 0x3032
STW R5, R6, #-1     ; 0x3033
TRAP x25            ; 0x3034
XOR R7, R7, R7      ; 0x3035
LABEL .FILL x1234   ; 0x3036

; Subroutine
SUBROUTINE
ADD R2, R2, #1      ; 0x3038
RET                 ; 0x3039

.END
