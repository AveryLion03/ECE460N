        .ORIG x3000
        ;Test Instructions
        ;LEA R0, MEM_ADD
        ;LDW R0, R0, #0
        ;ADD R0, R0, #1
        ;AND R2, R2, #0
        ;ADD R2, R2, #2
        ;STB R2, R0, #0
        ;HALT
        ;End Test

        ;Real Program
        LEA R0, LOOP_Val ; Holds how many times we need to loop 19 - 0 A
        LDW R0, R0, #0 ; 16

        ; Read from xC000
        LEA R4, MEM_ADD
        LDW R4, R4, #0 
        AND R3, R3, #0 ; Result Register 10

LOOP    LDB R2, R4, #0  ; 16 0x0A
        ADD R3, R3, R2  ; 10 0x0C
        ADD R4, R4, #1  ; 10 0x0E
        ADD R0, R0, #-1 ; 10 0x10
        BRp LOOP ; 11 0x12

        LEA R0, MEM_STORE ;0x14
        LDW R0, R0, #0  ;0x16
        STW R3, R0, #0 ;0x18
        JMP R3
        HALT    ;0x1A

MEM .FILL   x4000
EXCEPT .FILL 0x0020
LOOP_Val    .FILL x0014
MEM_ADD     .FILL xC000
MEM_TEST    .FILL x0000
MEM_STORE   .FILL xC014
TEST_MEM    .FILL x3500

        .END
