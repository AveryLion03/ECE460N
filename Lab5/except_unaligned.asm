    .ORIG x1A00
    ;ADD R2, R2, #15

    ;LEA R0, MEM
    ;LDW R0, R0, #0
    ;STW R2, R0, #0 ; Page Fault
    TRAP x25

MEM .FILL x3500

    .END
