/*
	Name 1: Avery Atchley      
	UTEID 1: aba2543
*/


#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h> /* Standard C Library */
#include <string.h>
#include <ctype.h>

FILE* infile = NULL;
FILE* outfile = NULL;
static int PC;
// Look up table definition
typedef struct {
    char *mnemonic;
    int value;
} LookupTableEntry;
// ***************** LOOKUP TABLE OP CODE INFORMATION *****************
LookupTableEntry opCodes_Table[] = {
    // Accomodate for the byte addressability of each memory location -> Make 8 bit instruction?
    // We could also make it 4 bits and multiply by 16^3...
    {"ADD", 0b0001}, //
    {"AND", 0b0101}, //
    // Check in the branch opcode, which specific is needed
    {"BR",  0b0000}, //
    {"BR",  0b0000},
    {"BRN", 0b0000},
    {"BRP",  0b0000},
    {"BRNP", 0b0000},
    {"BRZ",  0b0000},
    {"BRNZ", 0b0000},
    {"BRZP", 0b0000},
    {"BRNZP",0b0000}, //
    {"JMP", 0b1100}, // aslo same opcode as ret
    {"RET", 0b1100}, //
    {"JSR", 0b0100}, //
    {"JSRR",0b0100}, //
    {"LDB", 0b0010}, //
    {"LDW", 0b0110}, //
    {"LEA", 0b1110}, //
    {"RTI", 0b1000}, //
    {"SHF", 0b1101}, //
    {"LSHF", 0b1101}, //
    {"RSHFL", 0b1101}, //
    {"RSHFA", 0b1101}, //
    {"STB", 0b0011}, //
    {"STW", 0b0111}, //
    {"TRAP",0b1111}, //
    {"XOR", 0b1001}, // same as NOT
    {"NOT", 0b1001}, //
    //Custom Values for non-op code commands
    {".FILL", 20   }, 
    {".ORIG", 21   }, //
    {".END",  22   }, //
    {"HALT",  23   }, //
    {"NOP",  24   },
};
int lookupTableSize = sizeof(opCodes_Table)/sizeof(LookupTableEntry);
// Check if element in assembly file is a valid op code
int getOpcode(const char *mnemonic) {
    if(mnemonic == NULL) return -2; // If string is null, ignore it
    for (int i = 0; i < lookupTableSize; i++) {
        if (strcmp(opCodes_Table[i].mnemonic, mnemonic) == 0) {
            return opCodes_Table[i].value;
        }
    }
    return -1; // Return -1 if mnemonic is not found
}

// ***************** LOOKUP TABLE LABEL TABLE INFORMATION *****************
LookupTableEntry* labels_Table = {NULL}; //Declare as empty to begin
int labels_Size = 0; // Dynamic Size 
const char* invalidLabels [4] = {"IN", "OUT", "GETC", "PUTS"};
void addLabelToTable(char *mnemonic, int mem_Loc){
    //check if label is invalid first:
    for(int i = 0; i < 4; i++){
        if(strcmp(mnemonic, invalidLabels[i])== 0 ){
            printf("Invalid Label: %s\n", mnemonic);
            return;
        }
    }
    for(int temp = 0; mnemonic[temp];temp++){
            mnemonic[temp] = toupper(mnemonic[temp]);
        }
    // Allocate memory for the label table if it is empty
    if (labels_Size == 0) {
        labels_Table = (LookupTableEntry*)malloc(1 * sizeof(LookupTableEntry));
    } else {
        // Reallocate memory space to increase the size of the label table
        labels_Table = realloc(labels_Table, (labels_Size + 1) * sizeof(LookupTableEntry));
    }
    // Allocate memory for the mnemonic and copy the string
    labels_Table[labels_Size].mnemonic = strdup(mnemonic);
    labels_Table[labels_Size].value = mem_Loc;
    labels_Size++;
}
// Custom case-insensitive comparison function
int caseInsensitiveCompare(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (tolower((unsigned char)*str1) != tolower((unsigned char)*str2)) {
            return tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
        }
        str1++;
        str2++;
    }
    return tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
}

// Check if label exists, returns memory location
int getLabel(const char *mnemonic) {
    if(mnemonic == NULL) return -2; // if string is null, ignore
    for (int i = 0; i < labels_Size; i++) {
        if (caseInsensitiveCompare(labels_Table[i].mnemonic, mnemonic) == 0) {
            return labels_Table[i].value;
        }
    }
    return -1; // Return -1 if label is not found
}

// ***************** LOOKUP TABLE REGISTERS *****************
LookupTableEntry register_Table[] = {
    // Accomodate for the byte addressability of each memory location -> Make 8 bit instruction?
    // We could also make it 4 bits and multiply by 16^3...
    {"R0", 0b000}, 
    {"R1", 0b001},
    {"R2", 0b010},
    {"R3", 0b011},
    {"R4", 0b100},
    {"R5", 0b101},
    {"R6", 0b110},
    {"R7", 0b111},
};
// Check if label exists, returns memory location
int getRegister(const char *mnemonic) {
    if(mnemonic == NULL) return -2; // if string is null, ignore
    for (int i = 0; i < 8; i++) {
        if (strcmp(register_Table[i].mnemonic, mnemonic) == 0) {
            return register_Table[i].value;
        }
    }
    return -1; // Return -1 if label is not found
}

// ***************** Instruction Parser and Output Generator *****************
// Used to convert values found in the assembly file (hex, dec, or binary) into it's respective number
int convertToNumber(const char *str) {
    char *endptr;
    int base = 10;
    // Check for binary, decimal, or hexadecimal prefixes
    if (str[0] == '#') {
        str++; // Skip the '#'
        base = 10; // Decimal
    } else if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2; // Skip the 'x'
        base = 16; // Hexadecimal
    } else if ((str[0] == 'x' || str[0] == 'X')) {
        str++; // Skip the 'x'
        base = 16; // Hexadecimal
    } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        str += 2; // Skip the 'b'
        base = 2; // Binary
    }
    else if ((str[0] == 'b' || str[0] == 'B')) {
        str++; // Skip the 'b'
        base = 2; // Binary
    }

    // Convert the string to a number
    long value = strtol(str, &endptr, base);

    // Check for conversion errors
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid number: %s\n", str);
        return -1;
    }

    return (int)value;
}


int part_Instruction(char* elements[], int opCode) {
    // Relevant Variables
    int machineCode = 0;
    int reg1, reg2, reg3, offset, immediate;
    int pc = 0; 
    int flag, nzp, pcoffset, labelAddress, labelBias;
    // Check if there is a label in the line 
    if(getLabel(elements[0]) != -1) labelBias = 1;
    else labelBias = 0;
    // Begin switch-case
    switch (opCode) {
        case 0b0001: // ADD
            flag = 0;
            reg1 = getRegister(elements[1 + labelBias]); // "R1"
            reg2 = getRegister(elements[2 + labelBias]); // "R2"
            if (elements[3  + labelBias][0] == 'R') { // Register mode
                reg3 = getRegister(elements[3  + labelBias]); //  "R3"
                machineCode = (opCode << 12) | (reg1 << 9) | (reg2 << 6) | ((flag << 5) | reg3);
            } else { // Immediate mode
                flag = 1;
                const char *immediateStr = elements[3 + labelBias]; // Skip the first character
                immediate = convertToNumber(immediateStr); // Convert to integer
                // Ensure immediate value is within the expected range
                if (immediate < -16 || immediate > 15) {
                    fprintf(stderr, "Immediate value out of range: %d\n", immediate);
                    // Handle the error appropriately
                }
                machineCode = (opCode << 12) | (reg1 << 9) | (reg2 << 6) | (flag << 5) | (immediate & 0x01F); // Mask to 5 bits

            }
            break;
        case 0b0101: //AND
            flag = 0;
            reg1 = getRegister(elements[1 + labelBias]); // "R1"
            reg2 = getRegister(elements[2 + labelBias]); // "R2"
        if (elements[3 + labelBias][0] == 'R') { // Register mode
            reg3 = getRegister(elements[3 + labelBias]); //  "R3"
            machineCode = (opCode << 12) | (reg1 << 9) | (reg2 << 6) | ((flag << 5) | reg3);
        } else { // Immediate mode
            flag = 1;
            const char *immediateStr = elements[3 + labelBias]; // Skip the first character
            immediate = convertToNumber(immediateStr); // Convert to integer
            machineCode = (opCode << 12) | (reg1 << 9) | (reg2 << 6) | (flag << 5) | (immediate & 0x01F);
        }
            break;
        case 0b0000: // BR (Branch)
            nzp = 0; // Default to 000 (no condition flags set)
            pcoffset = 0;
            // Determine condition flags (NZP)
            if (strcmp(elements[0 + labelBias], "BRN") == 0) nzp = 0b100; // Branch if negative
            else if (strcmp(elements[0 + labelBias], "BRZ") == 0) nzp = 0b010; // Branch if zero
            else if (strcmp(elements[0 + labelBias], "BRP") == 0) nzp = 0b001; // Branch if positive
            else if (strcmp(elements[0 + labelBias], "BRNZ") == 0) nzp = 0b110; // Branch if negative or zero
            else if (strcmp(elements[0 + labelBias], "BRNP") == 0) nzp = 0b101; // Branch if negative or positive
            else if (strcmp(elements[0 + labelBias], "BRZP") == 0) nzp = 0b011; // Branch if zero or positive
            else if (strcmp(elements[0 + labelBias], "BRNZP") == 0) nzp = 0b111; // Branch if negative, zero, or positive
            else if (strcmp(elements[0 + labelBias], "BR") == 0) nzp = 0b111; // Branch if negative, zero, or positive

            // Extract PC offset from the instruction
            if(getLabel(elements[1 + labelBias]) > -1){
                labelAddress = getLabel(elements[1 + labelBias]);
                pcoffset = (labelAddress - PC)/2;
                // printf("Label Address: %i\nCurrent PC: %i\nPCOFFSET IS: %i\n", labelAddress, PC, pcoffset);
            }
            else
                printf("Error: Label %s not found\n", elements[1 + labelBias]);
    
            // Construct machine code
            machineCode = (opCode << 12) | (nzp << 9) | (pcoffset & 0x01FF); // Offset is 9 bits
            break;
        case 0b1100: //JMP
        {
            if(strcmp(elements[0 + labelBias], "JMP") == 0)
            {
                reg1 = getRegister(elements[1 + labelBias]); // Base Register
                // Construct machine code
                machineCode = (opCode << 12) | (0b000 << 9) | (reg1 << 6) | 0b000000; // 5-0 are 0's
            }
            else
            {
                // RET is a special case of JMP with base register R7
                reg1 = getRegister("R7"); // RET uses R7 as the base register
                // Construct machine code
                machineCode = (opCode << 12) | (0b000 << 9) | (0b111 << 6) | 0b000000; // 5-0 are 0's
            }
        }
            break;
            
        case 0b0100: // JSR/JSRR
        {
            // Check if bit 11 is set (1 for JSR, 0 for JSRR)
            if (strcmp(elements[0 + labelBias], "JSR") == 0) {
                // JSR case (bit 11 = 1)
                int labelAddress = getLabel(elements[1  + labelBias]);

                if (labelAddress == -1) {
                    printf("Error: Invalid label %s\n", elements[1 + labelBias]);
                    return -1;  // Handle invalid label error
                }

                // Calculate the PC offset
                pcoffset = (labelAddress - (PC))/2;

                // Construct machine code: opcode (0100) + bit 11 (1) + PC offset (10-0)
                machineCode = (0b0100 << 12) | (1 << 11) | (pcoffset & 0x7FF);
            }
            else if (strcmp(elements[0 + labelBias], "JSRR") == 0) {
                // JSRR case (bit 11 = 0)
                int baseRegister = getRegister(elements[1 + labelBias]);

                if (baseRegister == -1) {
                    printf("Error: Invalid base register %s\n", elements[1 + labelBias]);
                    return -1;  // Handle invalid register error
                }

                // Construct machine code: opcode (0100) + bit 11 (0) + bits 10-9 (00) + base register + bits 5-0 (000000)
                machineCode = (0b0100 << 12) | (0 << 11) | (0b00 << 9) | (baseRegister << 6) | 0b000000;
            }
        }
            break;
        case 0b0010: // LDB
        {
            // Extract destination register (DR)
            int dr = getRegister(elements[1 + labelBias]);

            if (dr == -1) {
                printf("Error: Invalid destination register %s\n", elements[1+ labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract base register (BaseR)
            int baseRegister = getRegister(elements[2 + labelBias]);

            if (baseRegister == -1) {
                printf("Error: Invalid base register %s\n", elements[2 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract the 6-bit offset
            char* tempOff = elements[3 + labelBias];
            int boffset6 = convertToNumber(tempOff);

            // Ensure the offset fits into 6 bits (-32 to 31)
            if (boffset6 < -32 || boffset6 > 31) {
                printf("Error: Offset out of range: %d\n", boffset6);
                return -1;  // Handle out-of-range offset error
            }

            // Construct machine code: opcode (0010) + DR (11-9) + BaseR (8-6) + boffset6 (5-0)
            machineCode = (opCode << 12) | (dr << 9) | (baseRegister << 6) | (boffset6 & 0x03F);
        }
            break;
        case 0b0110: // LDW
        {
            // Extract destination register (DR)
            int dr = getRegister(elements[1 + labelBias]);

            if (dr == -1) {
                printf("Error: Invalid destination register %s\n", elements[1 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract base register (BaseR)
            int baseRegister = getRegister(elements[2 + labelBias]);

            if (baseRegister == -1) {
                printf("Error: Invalid base register %s\n", elements[2 + labelBias]);
                return -1;  // Handle invalid register error
            }
 
            // Extract the 6-bit offset
            char* tempOff = elements[3 + labelBias];
            int boffset6 = convertToNumber(tempOff);
            // Ensure the offset fits into 6 bits (-32 to 31)
            if (boffset6 < -32 || boffset6 > 31) {
                printf("Error: Offset out of range: %d\n", boffset6);
                return -1;  // Handle out-of-range offset error
            }

            // Construct machine code: opcode (0010) + DR (11-9) + BaseR (8-6) + boffset6 (5-0)
            machineCode = (opCode << 12) | (dr << 9) | (baseRegister << 6) | (boffset6 & 0x03F);
        }
            break;
        case 0b1110: // LEA
        {
            // Extract destination register (DR)
            int dr = getRegister(elements[1 + labelBias]);
            if (dr == -1) {
                printf("Error: Invalid destination register %s\n", elements[1 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Find the label in the symbol table and compute the PC-relative offset
            int labelAddress = getLabel(elements[2 + labelBias]);

            if (labelAddress == -1) {
                printf("Error: Label not found: %s\n", elements[2 + labelBias]);
                return -1;  // Handle label not found error
            }

            // Calculate PC-relative offset (current PC is assumed to be provided)
            int pcoffset = (labelAddress - (PC))/2;

            // Ensure the offset fits into 9 bits (-256 to 255)
            if (pcoffset < -256 || pcoffset > 255) {
                printf("Error: Offset out of range: %d\n", pcoffset);
                return -1;  // Handle out-of-range offset error
            }

            // Construct machine code: opcode (1110) + DR (11-9) + pcOffset (8-0)
            machineCode = (opCode << 12) | (dr << 9) | (pcoffset & 0x01FF);
        }
            break;
        case 0b1000: //RTI
        {
            machineCode = 0b1000000000000000;               
        }
            break;
        case 0b1101: // LSHF, RSHFL, RSHFA
        {
            // Extract destination register (DR)
            int dr = getRegister(elements[1 + labelBias]);

            if (dr == -1) {
                printf("Error: Invalid destination register %s\n", elements[1 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract source register (SR)
            int sr = getRegister(elements[2 + labelBias]);

            if (sr == -1) {
                printf("Error: Invalid source register %s\n", elements[2 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract the amount to shift (amount4)
            char* tempOff = elements[3 + labelBias];
            int amount4 = convertToNumber(tempOff);
            if (amount4 < 0 || amount4 > 15) {
                printf("Error: Invalid shift amount %d\n", amount4);
                return -1;  // Handle invalid shift amount
            }

            // Determine the type of shift and set bits 5-4
            int shiftType = 0;
            if (strcmp(elements[0 + labelBias], "LSHF") == 0) {
                shiftType = 0b00; // LSHF: Logical shift left
            } else if (strcmp(elements[0 + labelBias], "RSHFL") == 0) {
                shiftType = 0b01; // RSHFL: Logical shift right
            } else if (strcmp(elements[0+ labelBias], "RSHFA") == 0) {
                shiftType = 0b11; // RSHFA: Arithmetic shift right
            } else {
                printf("Error: Unknown shift instruction %s\n", elements[0]);
                return -1;  // Handle unknown instruction error
            }

            // Construct machine code: opcode (1101) + DR (11-9) + SR (8-6) + shiftType (5-4) + amount4 (3-0)
            machineCode = (opCode << 12) | (dr << 9) | (sr << 6) | (shiftType << 4) | (amount4 & 0x0F);
        }
            break;
        case 0b0011: // STB
        {
            // Extract source register (SR)
            int sr = getRegister(elements[1 + labelBias]);
            if (sr == -1) {
                printf("Error: Invalid source register %s\n", elements[1 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract base register (BaseR)
            int baseR = getRegister(elements[2 + labelBias]);
            if (baseR == -1) {
                printf("Error: Invalid base register %s\n", elements[2 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract the boffset6 (6-bit signed offset)
            char* tempOff = elements[3 + labelBias];
            int boffset6 = convertToNumber(tempOff);
            if (boffset6 < -32 || boffset6 > 31) {
                printf("Error: Invalid offset %d\n", boffset6);
                return -1;  // Handle invalid offset error
            }

            // Construct machine code: opcode (0011) + SR (11-9) + BaseR (8-6) + boffset6 (5-0)
            machineCode = (opCode << 12) | (sr << 9) | (baseR << 6) | (0x03F & boffset6);
        }
            break;
        case 0b0111: // STW
        {
            // Extract source register (SR)
            int sr = getRegister(elements[1 + labelBias]);
            if (sr == -1) {
                printf("Error: Invalid source register %s\n", elements[1 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract base register (BaseR)
            int baseR = getRegister(elements[2 + labelBias]);
            if (baseR == -1) {
                printf("Error: Invalid base register %s\n", elements[2 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract the boffset6 (6-bit signed offset)
            char* tempOff = elements[3 + labelBias];
            int boffset6 = convertToNumber(tempOff);
            if (boffset6 < -32 || boffset6 > 31) {
                printf("Error: Invalid offset %d\n", boffset6);
                return -1;  // Handle invalid offset error
            }

            // Construct machine code: opcode (0011) + SR (11-9) + BaseR (8-6) + boffset6 (5-0)
            machineCode = (opCode << 12) | (sr << 9) | (baseR << 6) | (boffset6 & 0x03F);
        }
            break;
        case 0b1001: // XOR
        {
            // Extract destination register (DR)
            int dr = getRegister(elements[1 + labelBias]);
            if (dr == -1) {
                printf("Error: Invalid destination register %s\n", elements[1 + labelBias]);
                return -1;  // Handle invalid register error
            }

            // Extract source register 1 (SR1)
            int sr1 = getRegister(elements[2 + labelBias]);
            if (sr1 == -1) {
                printf("Error: Invalid source register %s\n", elements[2 + labelBias]);
                return -1;  // Handle invalid register error
            }

            if(strcmp(elements[0 + labelBias], "NOT") == 0)
            {
                // Construct machine code: opcode (1001) + DR (11-9) + SR1 (8-6) + 1 (bit 5) + imm5 (4-0)
                machineCode = ((opCode << 12) | (dr << 9) | (sr1 << 6) | (0b1 << 5) | (0x01F)) & 0xFFFF;
            }
            else{

                // Check if the third operand is a register or an immediate value
                if (getRegister(elements[3 + labelBias]) > -1) {
                    // XOR DR, SR1, SR2 (register format)
                    
                    // Extract source register 2 (SR2)
                    int sr2 = getRegister(elements[3 + labelBias]);
                    if (sr2 == -1) {
                        printf("Error: Invalid source register %s\n", elements[3 + labelBias]);
                        return -1;  // Handle invalid register error
                    }

                    // Construct machine code: opcode (1001) + DR (11-9) + SR1 (8-6) + 0 (bit 5) + SR2 (2-0)
                    machineCode = ((opCode << 12) | (dr << 9) | (sr1 << 6) | (0b000 << 3) | (sr2)) & 0xFFFF;
                } 
                else {
                    // XOR DR, SR1, imm5 (immediate format)

                    // Extract immediate value (imm5)
                    const char *immediateStr = elements[3 + labelBias]; // Skip the first character
                    int imm5 = convertToNumber(immediateStr); // Convert to integer
                    if (imm5 < -16 || imm5 > 15) {
                        printf("Error: Invalid immediate value %d\n", imm5);
                        return -1;  // Handle invalid immediate value error
                    }

                    // Construct machine code: opcode (1001) + DR (11-9) + SR1 (8-6) + 1 (bit 5) + imm5 (4-0)
                    machineCode = ((opCode << 12) | (dr << 9) | (sr1 << 6) | (0b1 << 5) | (imm5 & 0x01F)) & 0xFFFF;
                }
            }
        }
            break;
        case 0b1111: // TRAP
        {
            int trapVect = convertToNumber(elements[1 + labelBias]);
            if (trapVect < -256 || trapVect > 255) {
                printf("Error: trapVector out of range: %d\n", trapVect);
                return -1;  // Handle out-of-range offset error
            }
            machineCode = (opCode << 12) | (0 << 9) | (trapVect & 0x03FF);
        }
            break;
        case 20: // .FILL
        {
            int fillVal = convertToNumber(elements[1 + labelBias]);
            machineCode = (fillVal & 0xFFFF);
        }
            break;
        case 23: // HALT
        {
            machineCode = (0b1111 << 12) | (0b0000 << 9) | 0b00100101;
        }
            break;
        case 24: // NOP
        {
            machineCode = 0; //0x0000
        }
            break;

        // Other cases...
    }

    return machineCode;
}

// ***************** Remove White Spaces *****************
void collapse_spaces(char *str) {
    char *dst = str;
    char *src = str;
    int space_found = 0;

    while (*src != '\0') {
        if (isspace((unsigned char)*src)) {
            if (!space_found) {
                *dst++ = ' ';
                space_found = 1;
            }
        } else {
            *dst++ = *src;
            space_found = 0;
        }
        src++;
    }
    *dst = '\0';
}
//Check if empty Line
int isLineEmpty(const char *line) {
    while (*line != '\0') {
        if (!isspace((unsigned char)*line)) {
            return 0; // Line is not empty
        }
        line++;
    }
    return 1; // Line is empty or contains only whitespace
}

// ***************** File Parser *****************
int parse_files(const char* input, const char* output, int pass){
    // Open the source file for reading
    infile = fopen(input, "r");
    if (!infile) {
        printf("Error: Cannot open file %s\n", input);
        return 4;
    }

    // Open the output file for writing if second pass?
    if(pass == 2){
        outfile = fopen(output, "w");
        // printf("Running");
        if (!outfile) {
            printf("Error: Cannot open file %s\n", output);
            fclose(infile); // Close the input file if the output file cannot be opened
            return 4;
        }
    }
    char line[256]; //Buffer for each line in the assembly file
    char opcodeStr[10]; //Buffer for the converted machine code (in hexadecimal)
    char *line_decoded;
    char *elements[10];
    // Go through the entire file, line by line
    while (fgets(line, sizeof(line), infile)) {
        // Find the first occurrence of ';' and replace it with a null terminator
        char *comment_start = strchr(line, ';');
        if (comment_start != NULL) {
            *comment_start = '\0';
        }
         // Remove extra spaces
        collapse_spaces(line);
        if (!isLineEmpty(line)) {
            printf("Current Line: %s\n", line); // Test
        }
        for(int temp = 0; line[temp];temp++){
            line[temp] = toupper(line[temp]);
        }
        const char s[4] = " ,\n";
        int numElem = 0;
        char *line_decoded;
        char *inner_saveptr = NULL;

        // Separate each instruction into respective parts
        line_decoded = strtok_r(line, s, &inner_saveptr); // Separates based on spaces and commas
        elements[0] = line_decoded;
        numElem++;

        while (line_decoded != NULL) {
            line_decoded = strtok_r(NULL, s, &inner_saveptr); // Separates based on spaces, commas, and new lines
            elements[numElem] = line_decoded;
            numElem++;
        }
        // First pass, add labels with their memory loc, ignore everything else
        if (pass == 1){
            int code = getOpcode(elements[0]);
            if(code != -1){
                // printf("Not a label");
                if(code == 21){ // .ORIG
                    char * temp_PC = elements[1];
                    PC = convertToNumber(temp_PC);
                    // printf("%i\n", PC);
                }
                else if(code == 22){ // code == 22 is .END
                    // Stop reading the input file
                    fclose(infile);
                    // fclose(outfile); // Shouldn't have output file if we are on the first pass
                    return 0;
                }
                else if(code != -2) PC += 2;
            }
            // LABEL!!
            else{
                if(getOpcode(elements[0]) == -1 && getOpcode(elements[1]) >= 0){
                    addLabelToTable(elements[0], PC);
                    PC += 2;
                }
                else if (getOpcode(elements[0]) == -1 && getOpcode(elements[1]) == -2){
                    addLabelToTable(elements[0], PC);
                }
                else{
                    printf("Invalid Command: %s\n", elements[1]);
                    return -1;
                }
            }
        }
        // Second Pass
        else{
            int opCode = getOpcode(elements[0]); //Fetches the opcode of each line -> will expand into fetching and converting the full instruction
            if (opCode == -1 && getLabel(elements[0]) == -1) {
                printf("Error: Invalid mnemonic or label: %s\n", elements[0]);
                return -1;
            }
            else if(opCode == -2){
                // printf("Skipping line\n");
                continue;
            }
            // Opcode or label exists, parse instruction
            else{
                // if label is first element, set opcode to second element in line
                if(getOpcode(elements[0]) == -1) opCode = getOpcode(elements[1]); // We already know its a valid opcode if it makes it to second pass!
                if(opCode == 21) {
                    char * temp_PC = elements[1];
                    PC = convertToNumber(temp_PC);
                    // printf("%i\n", PC);
                    sprintf(opcodeStr, "0x%04X\n", PC);
                    fputs(opcodeStr, outfile); //Writes the OpCode, converted into hexadecimal, to the output file.
                }
                else if(opCode == 22){
                    break;
                }
                else if(opCode == -2){
                    //SKip just label
                    continue;
                }
                else{
                    //Parse through everything!
                    PC += 2;
                    int machineCode = part_Instruction(elements, opCode);
                    printf("Machine Code: 0x%x\n", machineCode);
                    sprintf(opcodeStr, "0x%04X\n", machineCode);
                    fputs(opcodeStr, outfile); //Writes the OpCode, converted into hexadecimal, to the output file.
                    // printf("Current PC: 0x%x\n", PC);
                }
            }
        }
    }
    // Close the files after second pass
    fclose(infile);
    if(pass == 2){
        fclose(outfile);
    }
    // If we reach this on the first pass, there is an error with the input file
    else{
        printf("Missing end statement in assembly file");
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // Default file names
    const char* default_infile = "text.asm";
    const char* default_outfile = "machine_lc3b.obj";

    // Use default file names if not provided in command line arguments
    const char* input_filename = (argc > 1) ? argv[1] : default_infile;
    const char* output_filename = (argc > 2) ? argv[2] : default_outfile;

    // Call Parser Function
    // First Pass
    printf("Starting First Pass:\n");
    if(parse_files(input_filename, output_filename, 1) == 0){
        printf("Successfully added Labels to Table\n\n");
    }
    // Second Pass
    printf("Starting Second Pass:\n");
    if(parse_files(input_filename, output_filename, 2) == 0){
        printf("Successfully parsed assembly file\n");
    }
    printf("Labels Table Freed\n");
    free(labels_Table);
    return 0; 
}
