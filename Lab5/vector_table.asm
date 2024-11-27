    .ORIG x0200
    
    .FILL x0000
    .FILL x1200 ; Interrupt
    .FILL x1400 ; Page Fault
    .FILL x1A00 ; Unaligned
    .FILL x1600 ; Protection
    .FILL x1C00 ; Unknown Opcode

    HALT
    .END
