.ORIG x1200
        ; Store values of regs we are using
        ADD R6, R6, #-2
        STW R5, R6, #0
        ADD R6, R6, #-2
        STW R7, R6, #0

        ; Test nested exception
        ; LEA R5, EXCEPT
        ; LDW R5, R5, #0
        ; ADD R5, R5, #1
        ; STW R5, R5, #0

        ; Increment Value at 0x4000
        LEA R5, MEM
        LDW R5, R5, #0 ; x0A
        LDW R7, R5, #0 ; x0C
        ; ADD R5, R5, #1 ; x0E
        ADD R7, R7, #1 ; x10
        STW R7, R5, #0

        ; Restore Regs
        LDW R7, R6, #0
        ADD R6, R6, #2
        LDW R5, R6, #0
        ADD R6, R6, #2
        RTI
        HALT ; 1C

MEM     .FILL x4000
EXCEPT  .FILL 0x2500
 .END 
