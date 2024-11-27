    .ORIG x1600
    ;AND R2, R2, #0
    ;ADD R2, R2, #2

    ;LEA R0, MEM2
    ;LDW R0, R0, #0
    ;ADD R0, R0, #1
    ;STW R2, R0, #0
    TRAP x25
    
MEM2 .FILL x2000

    .END
