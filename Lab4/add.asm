        .ORIG x3000
        
        
        LEA R0, MEM ; 16 Init x4000 to 1
        LDW R1, R0, #0 ; 16
        AND R2, R2, #0 ; 10
        ADD R2, R2, #1 ; 10
        STW R2, R1, #0 ; 16


        LEA R0, LOOP_Val ; Holds how many times we need to loop 19 - 0 16
        LDW R0, R0, #0 ; 16

        ; Read from xC000
        LEA R4, MEM_ADD
        LDW R4, R4, #0 
        AND R3, R3, #0 ; Result Register 10

LOOP    LDB R2, R4, #0  ; 16
        ADD R3, R3, R2  ; 10
        ADD R4, R4, #1  ; 10
        ADD R0, R0, #-1 ; 10
        BRp LOOP ; 11

        LEA R0, MEM_ADD
        LDW R0, R0, #0
        STW R3, R0, #10
        HALT


MEM .FILL   x4000
EXCEPT .FILL 0x0020
LOOP_Val    .FILL x0014
MEM_ADD     .FILL xC000

        .END
