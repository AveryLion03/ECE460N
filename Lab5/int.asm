.ORIG x1200
        ; Store values of regs we are using
        ADD R6, R6, #-2
        STW R1, R6, #0
        ADD R6, R6, #-2
        STW R2, R6, #0
        ADD R6, R6, #-2
        STW R5, R6, #0
        ADD R6, R6, #-2 ;0x0C
        STW R7, R6, #0
        

        LEA R5, AND_VAL ;0x10
        LDW R5, R5, #0 ;0x12
        LEA R7, LOOP_VAL
        LDW R7, R7, #0
        LEA R1, PTBR
        LDW R1, R1, #0 ;0x1A
        
        ;Loop through a clear REF bit
LOOP    LDW R2, R1, #0 ;0x1C
        AND R2, R2, R5 ;0x1E
        STW R2, R1, #0 ;0x20
        ADD R1, R1, #2 ;0x22
        ADD R7, R7, #-1;0x24
        BRp LOOP        ;0x26

        ; Restore Regs
        LDW R7, R6, #0
        ADD R6, R6, #2 ;0x2A
        LDW R5, R6, #0 ;0x2C
        ADD R6, R6, #2 ;0x2E
        LDW R2, R6, #0 ;x30
        ADD R6, R6, #2 ;x32
        LDW R1, R6, #0 ;x34
        ADD R6, R6, #2 ;x36
        RTI ;x38
        HALT ; 1C


AND_VAL     .FILL xFFFE
LOOP_VAL    .FILL #128
PTBR        .FILL x1000
 .END 
